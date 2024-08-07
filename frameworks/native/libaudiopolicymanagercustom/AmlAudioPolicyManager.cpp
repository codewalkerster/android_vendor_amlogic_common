/*
 * Copyright (C) 2024 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#define LOG_TAG "APM_AudioPolicyManager"

// Need to keep the log statements even in production builds
// to enable VERBOSE logging dynamically.
// You can enable VERBOSE logging as follows:
// adb shell setprop log.tag.APM_AudioPolicyManager V
#define LOG_NDEBUG 0

//#define VERY_VERBOSE_LOGGING
#ifdef VERY_VERBOSE_LOGGING
#define ALOGVV ALOGV
#else
#define ALOGVV(a...) do { } while(0)
#endif

#include <algorithm>
#include <inttypes.h>
#include <map>
#include <math.h>
#include <set>
#include <type_traits>
#include <unordered_set>
#include <vector>

#include <Serializer.h>
#include <android/media/audio/common/AudioPort.h>
#include <cutils/bitops.h>
#include <cutils/properties.h>
#include <media/AudioParameter.h>
#include <policy.h>
#include <private/android_filesystem_config.h>
#include <system/audio.h>
#include <system/audio_config.h>
#include <system/audio_effects/effect_hapticgenerator.h>
#include <utils/Log.h>

#include "AmlAudioPolicyManager.h"
#include "TypeConverter.h"

namespace android {
EngineInstance Aml_loadApmEngineLibraryAndCreateEngine(const std::string& librarySuffix __unused)
{
    auto engLib = EngineLibrary::load("_amlogic");
    if (!engLib) {
        ALOGE("%s: Failed to load the engine library, suffix:_amlogic", __func__);
        return nullptr;
    }
    auto engine = engLib->createEngineUsingXmlConfig("");
    if (engine == nullptr) {
        ALOGE("%s: Failed to instantiate the APM engine", __func__);
        return nullptr;
    }
    return engine;
}

#ifdef AML_BOARD_COMPILE_AOSP_TYPE
EngineInstance Aml_loadApmEngineLibraryAndCreateEngine(const std::string& librarySuffix __unused,
        const media::audio::common::AudioHalEngineConfig& config)
{
    auto engLib = EngineLibrary::load("_amlogic");
    if (!engLib) {
        ALOGE("%s: Failed to load the engine library, suffix:_amlogic", __func__);
        return nullptr;
    }
    auto engine = engLib->createEngineUsingHalConfig(config);
    if (engine == nullptr) {
        ALOGE("%s: Failed to instantiate the APM engine", __func__);
        return nullptr;
    }
    return engine;
}
#endif

__attribute__((unused)) extern "C"
AudioPolicyInterface* createAudioPolicyManager(AudioPolicyClientInterface *clientInterface)
{
    AmlAudioPolicyManager *apm = nullptr;
    media::AudioPolicyConfig apmConfig;
    status_t status = NO_ERROR;

#ifdef AML_BOARD_COMPILE_AOSP_TYPE
    if (status = clientInterface->getAudioPolicyConfig(&apmConfig); status == OK) {

        auto config = AudioPolicyConfig::loadFromApmAidlConfigWithFallback(apmConfig);
        LOG_ALWAYS_FATAL_IF(config->getEngineLibraryNameSuffix() !=
                AudioPolicyConfig::kDefaultEngineLibraryNameSuffix,
                "Only default engine is currently supported with the AIDL HAL");
        apm = new AmlAudioPolicyManager(config,
                Aml_loadApmEngineLibraryAndCreateEngine(
                        config->getEngineLibraryNameSuffix(), apmConfig.engineConfig),
                clientInterface);
    } else {
#endif
        auto config = AudioPolicyConfig::loadFromApmXmlConfigWithFallback();  // This can't fail.
        apm = new AmlAudioPolicyManager(config,
                Aml_loadApmEngineLibraryAndCreateEngine(config->getEngineLibraryNameSuffix()),
                clientInterface);
#ifdef AML_BOARD_COMPILE_AOSP_TYPE
    }
#endif
    status = apm->initialize();
    if (status != NO_ERROR) {
        delete apm;
        apm = nullptr;
    }
    return apm;
}

__attribute__((unused)) extern "C"
void destroyAudioPolicyManager(AudioPolicyInterface *interface)
{
    delete interface;
}


AmlAudioPolicyManager::AmlAudioPolicyManager(const sp<const AudioPolicyConfig>& config,
                                       EngineInstance&& engine,
                                       AudioPolicyClientInterface *clientInterface)
    : AudioPolicyManager(config, std::move(engine), clientInterface)
{
}

status_t AmlAudioPolicyManager::checkAndSetVolume(IVolumeCurves &curves,
                                               VolumeSource volumeSource,
                                               int index,
                                               const sp<AudioOutputDescriptor>& outputDesc,
                                               DeviceTypeSet deviceTypes,
                                               int delayMs,
                                               bool force)
{
    // do not change actual attributes volume if the attributes is muted
    if (outputDesc->isMuted(volumeSource)) {
        ALOGVV("%s: volume source %d muted count %d active=%d", __func__, volumeSource,
               outputDesc->getMuteCount(volumeSource), outputDesc->isActive(volumeSource));
        return NO_ERROR;
    }
    VolumeSource callVolSrc = toVolumeSource(AUDIO_STREAM_VOICE_CALL, false);
    VolumeSource btScoVolSrc = toVolumeSource(AUDIO_STREAM_BLUETOOTH_SCO, false);
    bool isVoiceVolSrc = (volumeSource != VOLUME_SOURCE_NONE) && (callVolSrc == volumeSource);
    bool isBtScoVolSrc = (volumeSource != VOLUME_SOURCE_NONE) && (btScoVolSrc == volumeSource);

    bool isScoRequested = isScoRequestedForComm();
    bool isHAUsed = isHearingAidUsedForComm();

    // do not change in call volume if bluetooth is connected and vice versa
    // if sco and call follow same curves, bypass forceUseForComm
    if ((callVolSrc != btScoVolSrc) &&
            ((isVoiceVolSrc && isScoRequested) ||
             (isBtScoVolSrc && !(isScoRequested || isHAUsed))) &&
            !isSingleDeviceType(deviceTypes, AUDIO_DEVICE_OUT_TELEPHONY_TX)) {
        ALOGV("%s cannot set volume group %d volume when is%srequested for comm", __func__,
             volumeSource, isScoRequested ? " " : " not ");
        // Do not return an error here as AudioService will always set both voice call
        // and bluetooth SCO volumes due to stream aliasing.
        return NO_ERROR;
    }
    if (deviceTypes.empty()) {
        deviceTypes = outputDesc->devices().types();
    }

    if (curves.getVolumeIndexMin() < 0 || curves.getVolumeIndexMax() < 0) {
        ALOGE("invalid volume index range");
        return BAD_VALUE;
    }

    float volumeDb = computeVolume(curves, volumeSource, index, deviceTypes);
    if (outputDesc->isFixedVolume(deviceTypes) ||
            // Force VoIP volume to max for bluetooth SCO device except if muted
            (index != 0 && (isVoiceVolSrc || isBtScoVolSrc) &&
                    isSingleDeviceType(deviceTypes, audio_is_bluetooth_out_sco_device))) {
        volumeDb = 0.0f;
    }
    /*[Amlogic start]+++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
    /* Change-Id: I47267f5372b9ea736f2bb902dd6baa0727e5ddce */
    /* Need adjust audio hal volume when television/soundbar platform. */
    bool soundbarMode = property_get_int32("persist.vendor.media.audio.soundbar.mode", 0) == 1;
    bool tvProduct = property_get_bool("ro.vendor.platform.has.tvuimode", false /* default_value */);
#if 0
    bool needApplySinkGain = false;
    if (tvProduct || soundbarMode) {
        needApplySinkGain = true;
    } else {
        bool    bootVideoRunning = property_get_int32("service.bootvideo.exit", 0) == 1;
        if (bootVideoRunning) {
            VolumeSource musicVolSrc = toVolumeSource(AUDIO_STREAM_MUSIC, false);
            ALOGV("[%s:%d] boot video Running, music type:%d", __func__, __LINE__, (musicVolSrc == volumeSource));
            if (bootVideoRunning && musicVolSrc == volumeSource) {
                volumeDb = -18.37f;
            }
        } else {
            for (auto client : outputDesc->clientsList(true /*activeOnly*/)) {
                ALOGV("[checkAndSetVolume:%d] isInternal:%d, profile:%s", __LINE__, client->isInternal(),
                     outputDesc->getAudioPort()->getName().c_str());
                if (client->isInternal()) {
                    SourceClientDescriptor* sourceDesc = static_cast<SourceClientDescriptor*>(client.get());
                    audio_devices_t sourceDevice = sourceDesc->srcDevice()->type();
                    ALOGV("[%s:%d] sourceDevice:%s(%#x)", __func__, __LINE__, audio_device_to_string(sourceDevice), sourceDevice);
                    if (sourceDevice == AUDIO_DEVICE_IN_HDMI || sourceDevice == AUDIO_DEVICE_IN_LINE ||
                         sourceDevice == AUDIO_DEVICE_IN_HDMI_ARC || sourceDevice == AUDIO_DEVICE_IN_HDMI_EARC ||
                         sourceDevice == AUDIO_DEVICE_IN_SPDIF || sourceDevice == AUDIO_DEVICE_IN_TV_TUNER) {
                        needApplySinkGain = true;
                        break;
                    }
                }
            }

        }
    }
#endif
    if (tvProduct || soundbarMode) {
        DeviceTypeSet curSrcDevicesVector = mEngine->getOutputDevicesForStream(AUDIO_STREAM_MUSIC, false).types();
        audio_devices_t curDevice = Volume::getDeviceForVolume(curSrcDevicesVector);
        audio_devices_t outputDescDevices = deviceTypesToBitMask(outputDesc->devices().types());
        if ((curDevice & outputDescDevices) != 0  && outputDesc->getPolicyAudioPort() != nullptr) {
            auto setSinkGainToHal = [this](auto &volumeDb, auto curDevice, auto outputDesc, auto outputDescDevices) {
                DeviceTypeSet   curDeviceVector {curDevice};
                bool            bootVideoRunning = property_get_int32("service.bootvideo.exit", 0) == 1;
                device_category devCategory = Volume::getDeviceCategory(curDeviceVector);
                auto &volCurves = getVolumeCurves(AUDIO_STREAM_MUSIC);
                int volumeIndex = volCurves.getVolumeIndex(curDeviceVector);
                float musicVolumeDb = volCurves.volIndexToDb(devCategory, volumeIndex);
                ALOGV("[checkAndSetVolume:%d] volumeDb:%f volume:%d, curDevice:%#x, devCategory:%d, outputDescDevices:%#x",
                    __LINE__, volumeDb, volumeIndex, curDevice, devCategory, outputDescDevices);
                ALOGV("[checkAndSetVolume:%d] musicVolumeDb:%f, bootVideoRunning:%d profile:%s", __LINE__, musicVolumeDb,
                    bootVideoRunning, outputDesc->getAudioPort()->getName().c_str());
                if (bootVideoRunning) {
                    volumeDb = 0.0f;
                    musicVolumeDb = -18.37f;
                }
                sp<DeviceDescriptor> outputDevice = nullptr;
                for (const auto &device : outputDesc->devices()) {
                    if (device->type() == curDevice) {
                        struct audio_port_config newConfig;
                        struct audio_port_config backupConfig;
                        device->toAudioPortConfig(&newConfig);
                        newConfig.config_mask = AUDIO_PORT_CONFIG_GAIN;
                        newConfig.type = AUDIO_PORT_TYPE_DEVICE;
                        newConfig.gain.values[0] = musicVolumeDb * 100;
                        newConfig.gain.index = 0;
                        newConfig.gain.mode = AUDIO_GAIN_MODE_JOINT;
                        newConfig.ext.device.type = curDevice;
                        status_t status = device->applyAudioPortConfig(&newConfig, &backupConfig);
                        if (status != NO_ERROR) {
                            ALOGE("checkAndSetVolume: Error to apply new config, status:%d", status);
                        }
                        status = mpClientInterface->setAudioPortConfig(&newConfig, 0);
                        if (status != NO_ERROR) {
                            device->applyAudioPortConfig(&backupConfig);
                            ALOGE("[checkAndSetVolume:%d] Error to setAudioPortConfig, status:%d", __LINE__, status);
                        }
                    }
                }
                // for CTS case: testAudioTrackMuteFromStreamVolumeNotification, testMediaPlayerMuteFromStreamVolumeNotification
                if (volumeDb > VOLUME_MIN_DB) {
                    volumeDb = 0.0f;
                }
            };
            if (curDevice == AUDIO_DEVICE_OUT_SPEAKER || curDevice == AUDIO_DEVICE_OUT_SPDIF ||
                curDevice == AUDIO_DEVICE_OUT_WIRED_HEADPHONE || curDevice == AUDIO_DEVICE_OUT_WIRED_HEADSET  ||
                (curDevice & AUDIO_DEVICE_OUT_ALL_A2DP) != 0 || (curDevice & AUDIO_DEVICE_OUT_ALL_USB) != 0) {
                setSinkGainToHal(volumeDb, curDevice, outputDesc, outputDescDevices);
            } else if (curDevice == AUDIO_DEVICE_OUT_HDMI_ARC) {
                volumeDb = 0.0f;
            }
        }
    }
    /*[Amlogic end]-----------------------------------------------------------*/

    const bool muted = (index == 0) && (volumeDb != 0.0f);
#ifdef AML_BOARD_COMPILE_AOSP_TYPE
    outputDesc->setVolume(volumeDb, muted, volumeSource, curves.getStreamTypes(),
            deviceTypes, delayMs, force, isVoiceVolSrc);
#else
    outputDesc->setVolume(
            volumeDb, muted, volumeSource, curves.getStreamTypes(), deviceTypes, delayMs, force);
#endif

    if (outputDesc == mPrimaryOutput && (isVoiceVolSrc || isBtScoVolSrc)) {
        float voiceVolume;
        // Force voice volume to max or mute for Bluetooth SCO as other attenuations are managed by the headset
        if (isVoiceVolSrc) {
            voiceVolume = (float)index/(float)curves.getVolumeIndexMax();
        } else {
            voiceVolume = index == 0 ? 0.0 : 1.0;
        }
        if (voiceVolume != mLastVoiceVolume) {
            mpClientInterface->setVoiceVolume(voiceVolume, delayMs);
            mLastVoiceVolume = voiceVolume;
        }
    }
    return NO_ERROR;
}

bool AmlAudioPolicyManager::isScoRequestedForComm() const {
    AudioDeviceTypeAddrVector devices;
    mEngine->getDevicesForRoleAndStrategy(mCommunnicationStrategy, DEVICE_ROLE_PREFERRED, devices);
    for (const auto &device : devices) {
        if (audio_is_bluetooth_out_sco_device(device.mType)) {
            return true;
        }
    }
    return false;
}

bool AmlAudioPolicyManager::isHearingAidUsedForComm() const {
    DeviceVector devices = mEngine->getOutputDevicesForStream(AUDIO_STREAM_VOICE_CALL,
                                                       true /*fromCache*/);
    for (const auto &device : devices) {
        if (device->type() == AUDIO_DEVICE_OUT_HEARING_AID) {
            return true;
        }
    }
    return false;
}

status_t AmlAudioPolicyManager::dump(int fd)
{
    auto engineSuffix = AudioPolicyConfig::loadFromApmXmlConfigWithFallback()->getEngineLibraryNameSuffix();
    dprintf(fd, "------ Amlogic_AudioPolicyManager (APM xml Engine: %s) -------", engineSuffix.c_str());
    AudioPolicyManager::dump(fd);
    return NO_ERROR;
}

} // namespace android
