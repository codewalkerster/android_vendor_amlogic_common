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
#include "DroidAudioCommon.h"
#include "DroidAudioCommonType.h"

namespace android {
using android::media::audio::common::AudioPortExt;
const char * forceUse2Str(audio_policy_forced_cfg_t value) {
    switch (value) {
    case AUDIO_POLICY_FORCE_NONE:
        return "NONE";
    case AUDIO_POLICY_FORCE_SPEAKER:
        return "SPEAKER";
    case AUDIO_POLICY_FORCE_HEADPHONES:
        return "HEADPHONES";
    case AUDIO_POLICY_FORCE_BT_SCO:
        return "BT_SCO";
    case AUDIO_POLICY_FORCE_BT_A2DP:
        return "BT_A2DP";
    case AUDIO_POLICY_FORCE_WIRED_ACCESSORY:
        return "USB";
    case AUDIO_POLICY_FORCE_BT_CAR_DOCK:
        return "BT_CAR_DOCK";
    case AUDIO_POLICY_FORCE_BT_DESK_DOCK:
        return "DESK_DOCK";
    case AUDIO_POLICY_FORCE_ANALOG_DOCK:
        return "SPDIF";
    case AUDIO_POLICY_FORCE_DIGITAL_DOCK:
        return "HDMI";
    case AUDIO_POLICY_FORCE_NO_BT_A2DP:
        return "NO_BT_A2DP";
    default:
        return "UNKNOWN";
    }
};

EngineInstance Aml_loadApmEngineLibraryAndCreateEngine(const std::string& librarySuffix)
{
    int mode = property_get_int32("persist.vendor.media.audio.output.strategy", DROID_AUDIO_OUTPUT_STRATEGY_AUTO);
    std::string suffix = librarySuffix;
    if (mode == DROID_AUDIO_OUTPUT_STRATEGY_SEMI_AUTO) {
        suffix = "_amlogic";
    }

    auto engLib = EngineLibrary::load(suffix);
    if (!engLib) {
        ALOGE("%s: Failed to load the engine library, suffix:%s", __func__, suffix.c_str());
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
EngineInstance Aml_loadApmEngineLibraryAndCreateEngine(const std::string& librarySuffix,
        const media::audio::common::AudioHalEngineConfig& config)
{
    int mode = property_get_int32("persist.vendor.media.audio.output.strategy", DROID_AUDIO_OUTPUT_STRATEGY_AUTO);
    std::string suffix = librarySuffix;
    if (mode == DROID_AUDIO_OUTPUT_STRATEGY_SEMI_AUTO) {
        suffix = "_amlogic";
    }

    auto engLib = EngineLibrary::load(suffix);
    if (!engLib) {
        ALOGE("%s: Failed to load the engine library, suffix:%s", __func__, suffix.c_str());
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

status_t AmlAudioPolicyManager::setDeviceConnectionState(
        audio_policy_dev_state_t state, const android::media::audio::common::AudioPort& port,
        audio_format_t encodedFormat) {
    if (port.ext.getTag() != AudioPortExt::device) {
        return BAD_VALUE;
    }
    audio_devices_t device_type;
    std::string device_address;
    if (status_t status = aidl2legacy_AudioDevice_audio_device(
                port.ext.get<AudioPortExt::device>().device, &device_type, &device_address);
        status != OK) {
        return status;
    };

    if (state == AUDIO_POLICY_DEVICE_STATE_UNAVAILABLE && audio_is_output_device(device_type)) {
        auto deviceToForceUse = [&](audio_devices_t device_type) -> audio_policy_forced_cfg_t {
            switch (device_type) {
                case AUDIO_DEVICE_OUT_HDMI_ARC:
                case AUDIO_DEVICE_OUT_HDMI_EARC:
                    return AUDIO_POLICY_FORCE_DIGITAL_DOCK;
                case AUDIO_DEVICE_OUT_SPEAKER:
                    return AUDIO_POLICY_FORCE_SPEAKER;
                case AUDIO_DEVICE_OUT_SPDIF:
                    return AUDIO_POLICY_FORCE_ANALOG_DOCK;
                case AUDIO_DEVICE_OUT_HDMI:
                    return AUDIO_POLICY_FORCE_BT_CAR_DOCK;
                case AUDIO_DEVICE_OUT_WIRED_HEADPHONE:
                case AUDIO_DEVICE_OUT_WIRED_HEADSET:
                    return AUDIO_POLICY_FORCE_HEADPHONES;
                case AUDIO_DEVICE_OUT_BLUETOOTH_A2DP:
                case AUDIO_DEVICE_OUT_BLUETOOTH_A2DP_HEADPHONES:
                case AUDIO_DEVICE_OUT_BLUETOOTH_A2DP_SPEAKER:
                    return AUDIO_POLICY_FORCE_BT_A2DP;
                case AUDIO_DEVICE_OUT_USB_HEADSET:
                case AUDIO_DEVICE_OUT_USB_DEVICE:
                case AUDIO_DEVICE_OUT_USB_ACCESSORY:
                    return AUDIO_POLICY_FORCE_WIRED_ACCESSORY;
                default:
                    return AUDIO_POLICY_FORCE_NONE;
            }
        };
        // The priority needs to be rescheduled after the currently force use device is disconnected.
        audio_policy_forced_cfg_t curForceUse = mEngine->getForceUse(AUDIO_POLICY_FORCE_FOR_MEDIA);
        audio_policy_forced_cfg_t disconnectDeviceForceUse = deviceToForceUse(device_type);
        AM_LOGI("curForceUse:%s(%d) disconnect_forceuse:%s(%d) ", forceUse2Str(curForceUse),
            curForceUse, forceUse2Str(disconnectDeviceForceUse), disconnectDeviceForceUse);
        if (disconnectDeviceForceUse != AUDIO_POLICY_FORCE_NONE &&
            curForceUse != AUDIO_POLICY_FORCE_NONE &&
            curForceUse == disconnectDeviceForceUse) {
            AM_LOGI("disconnect current forceuse device, set forceUse to none.");
            mEngine->setForceUse(AUDIO_POLICY_FORCE_FOR_MEDIA, AUDIO_POLICY_FORCE_NONE);
        }
    }

    return AudioPolicyManager::setDeviceConnectionState(state, port, encodedFormat);
}

status_t AmlAudioPolicyManager::setDevicesRoleForStrategy(product_strategy_t strategy,
                                                       device_role_t role,
                                                       const AudioDeviceTypeAddrVector &devices) {
    auto streams = mEngine->getStreamTypesForProductStrategy(strategy);
    int count = std::count(streams.begin(), streams.end(), AUDIO_STREAM_VOICE_CALL);
    if (count > 0 && role == DEVICE_ROLE_PREFERRED) {
        for (const auto& device : devices) {
            if (device.mType == AUDIO_DEVICE_OUT_BLUETOOTH_SCO ||
            device.mType == AUDIO_DEVICE_OUT_BLUETOOTH_SCO_HEADSET ||
            device.mType == AUDIO_DEVICE_OUT_BLUETOOTH_SCO_CARKIT) {
                setForceUse(AUDIO_POLICY_FORCE_FOR_COMMUNICATION, AUDIO_POLICY_FORCE_BT_SCO);
                break;
            } else if (device.mType == AUDIO_DEVICE_OUT_SPEAKER) {
                setForceUse(AUDIO_POLICY_FORCE_FOR_COMMUNICATION, AUDIO_POLICY_FORCE_SPEAKER);
                return NO_ERROR;
            }
        }
    }

    return AudioPolicyManager::setDevicesRoleForStrategy(strategy, role, devices);
}

status_t AmlAudioPolicyManager::clearDevicesRoleForStrategy(product_strategy_t strategy,
                                                           device_role_t role) {
    auto streams = mEngine->getStreamTypesForProductStrategy(strategy);
    int count = std::count(streams.begin(), streams.end(), AUDIO_STREAM_VOICE_CALL);
    if (count > 0 && role == DEVICE_ROLE_PREFERRED) {
        setForceUse(AUDIO_POLICY_FORCE_FOR_COMMUNICATION, AUDIO_POLICY_FORCE_NONE);
    }

    return AudioPolicyManager::clearDevicesRoleForStrategy(strategy, role);
}

void AmlAudioPolicyManager::setForceUse(audio_policy_force_use_t usage,
                         audio_policy_forced_cfg_t config) {
    int userForceUse = property_get_int32(PROP_AUDIO_OUTPUT_FORCEUSE, AUDIO_POLICY_FORCE_NONE);
    if (usage == AUDIO_POLICY_FORCE_FOR_MEDIA) {
        AM_LOGI("userDbForceUse:%s(%d) force_device:%s(%d)", forceUse2Str((audio_policy_forced_cfg_t)userForceUse),
            userForceUse, forceUse2Str(config), config);
    }
    // userForceUse specifies the value configured for the user.
    // If forceuse is different from the user value, the forceuse cannot be set. (AudioService.java)
    if (usage == AUDIO_POLICY_FORCE_FOR_MEDIA && userForceUse != config) {
        return;
    }
    AudioPolicyManager::setForceUse(usage, config);
    for (size_t i = 0; i < mAudioPatches.size(); i++) {
        sp<AudioPatch> patch = mAudioPatches.valueAt(i);
        if (patch->mPatch.num_sources > 0 && patch->mPatch.num_sinks > 0) {
            bool source_include_device = false;
            size_t i = 0;
            const struct audio_port_config *source = patch->mPatch.sources;
            for (i = 0; i < patch->mPatch.num_sources; i++) {
                if (source[i].type == AUDIO_PORT_TYPE_DEVICE) {
                   source_include_device = true;
                   break;
                }
            }
            const struct audio_port_config *sink = &patch->mPatch.sinks[0];
            if (source_include_device && sink->type == AUDIO_PORT_TYPE_DEVICE &&
                   (source[i].ext.device.type == AUDIO_DEVICE_IN_AUX_DIGITAL ||
                    source[i].ext.device.type == AUDIO_DEVICE_IN_TV_TUNER ||
                    source[i].ext.device.type == AUDIO_DEVICE_IN_LINE ||
                    source[i].ext.device.type == AUDIO_DEVICE_IN_SPDIF ||
                    source[i].ext.device.type == AUDIO_DEVICE_IN_HDMI_ARC)) {
                auto attributes = mEngine->getAllAttributesForProductStrategy(streamToStrategy(AUDIO_STREAM_MUSIC)).front();
                DeviceVector devices = mEngine->getOutputDevicesForAttributes(attributes, nullptr, false);
                if (!devices.containsDeviceWithType(patch->mPatch.sinks[0].ext.device.type)) {
                    AM_LOGI("tvinput patch output changed. sinks:%s", devices.toString().c_str());
                    mpClientInterface->onAudioPortListUpdate();
                }
                break;
            }
        }
    }
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
    const float BOOT_VIDEO_FIXED_VOLUME_DB = -18.37f;
    bool soundbarMode = property_get_int32("persist.vendor.media.audio.soundbar.mode", 0) == 1;
    bool tvProduct = property_get_bool("ro.vendor.platform.has.tvuimode", false /* default_value */);
    bool bBootVideoRunning = property_get_int32("service.bootvideo.exit", 0) == 1;
    auto setAudioPortConfig = [&](sp<DeviceDescriptor> device) {
        auto &volCurves = getVolumeCurves(AUDIO_STREAM_MUSIC);
        int volumeIndex = volCurves.getVolumeIndex(outputDesc->devices().types());
        device_category devCategory = Volume::getDeviceCategory(outputDesc->devices().types());
        float musicVolumeDb = volCurves.volIndexToDb(devCategory, volumeIndex);
        if (bBootVideoRunning) {
            musicVolumeDb = BOOT_VIDEO_FIXED_VOLUME_DB;
        }
        ALOGV("[checkAndSetVolume:%d] volume:%d volumeDb:%f outputDesc:%s profile:%s",
            __LINE__, volumeIndex, volumeDb, outputDesc->devices().toString().c_str(), outputDesc->getAudioPort()->getName().c_str());
        ALOGV("[checkAndSetVolume:%d] device:%s musicVolumeDb:%f bootVideoRunning:%d ", __LINE__,
            audio_device_to_string(device->type()), musicVolumeDb, bBootVideoRunning);
        struct audio_port_config newConfig;
        device->toAudioPortConfig(&newConfig);
        newConfig.config_mask = AUDIO_PORT_CONFIG_GAIN;
        newConfig.type = AUDIO_PORT_TYPE_DEVICE;
        newConfig.gain.values[0] = musicVolumeDb * 100;
        newConfig.gain.index = 0;
        newConfig.gain.mode = AUDIO_GAIN_MODE_JOINT;
        newConfig.ext.device.type = device->type();
        status_t status = mpClientInterface->setAudioPortConfig(&newConfig, 0 /*delayMs*/);
        if (status != NO_ERROR) {
            ALOGE("[checkAndSetVolume:%d] Error to setAudioPortConfig device:%s, status:%d", __LINE__,
                outputDesc->devices().toString().c_str(), status);
        }
    };

    if (tvProduct || soundbarMode) {
        bool found = false;
        // set the sink gain only for active volumeSource
        for (auto client : outputDesc->clientsList(true /*activeOnly*/)) {
            if (client->volumeSource() == volumeSource) {
                found = true;
            }
        }
        if (found) {
            for (const auto& device : outputDesc->devices()) {
                switch (device->type()) {
                    case AUDIO_DEVICE_OUT_SPEAKER:
                    case AUDIO_DEVICE_OUT_SPDIF:
                    case AUDIO_DEVICE_OUT_WIRED_HEADPHONE:
                    case AUDIO_DEVICE_OUT_WIRED_HEADSET:
                    case AUDIO_DEVICE_OUT_HEARING_AID:
                    case AUDIO_DEVICE_OUT_BLUETOOTH_A2DP:
                    case AUDIO_DEVICE_OUT_BLUETOOTH_A2DP_HEADPHONES:
                    case AUDIO_DEVICE_OUT_BLUETOOTH_A2DP_SPEAKER:
                    case AUDIO_DEVICE_OUT_USB_ACCESSORY:
                    case AUDIO_DEVICE_OUT_USB_DEVICE:
                    case AUDIO_DEVICE_OUT_USB_HEADSET:
                        // set the sink gain to audio hal
                        setAudioPortConfig(device);
                        if (bBootVideoRunning || volumeDb > VOLUME_MIN_DB) {
                            volumeDb = 0.0f;
                        }
                        break;
                    case AUDIO_DEVICE_OUT_HDMI_ARC:
                    case AUDIO_DEVICE_OUT_HDMI_EARC:
                        volumeDb = 0.0f;
                        break;
                    default:
                        break;
                }
            }
        }
    } else {
        VolumeSource musicVolSrc = toVolumeSource(AUDIO_STREAM_MUSIC);
        if (bBootVideoRunning && musicVolSrc == volumeSource) {
            ALOGV("[%s:%d] boot video Running, volume src:%d", __func__, __LINE__, volumeSource);
            volumeDb = -18.37f;
        }
        // set the source gain to audio hal
        for (auto client : outputDesc->clientsList(true /*activeOnly*/)) {
            // mix->dev. mix client: TrackClientDescriptor; dev->dev. dev client: InternalSourceClientDescriptor
            if (client->isInternal() && client->volumeSource() == volumeSource) {
                SourceClientDescriptor* sourceDesc = static_cast<SourceClientDescriptor*>(client.get());
                audio_devices_t sourceDeviceType = sourceDesc->srcDevice()->type();
                if (sourceDeviceType == AUDIO_DEVICE_IN_HDMI || sourceDeviceType == AUDIO_DEVICE_IN_LINE ||
                     sourceDeviceType == AUDIO_DEVICE_IN_HDMI_ARC || sourceDeviceType == AUDIO_DEVICE_IN_HDMI_EARC ||
                     sourceDeviceType == AUDIO_DEVICE_IN_SPDIF || sourceDeviceType == AUDIO_DEVICE_IN_TV_TUNER) {
                    setAudioPortConfig(sourceDesc->srcDevice());
                }
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
    int mode = property_get_int32("persist.vendor.media.audio.output.strategy", DROID_AUDIO_OUTPUT_STRATEGY_AUTO);
    std::string engineSuffix = "";
    if (mode == DROID_AUDIO_OUTPUT_STRATEGY_SEMI_AUTO) {
        engineSuffix = "_amlogic";
    } else {
        engineSuffix = AudioPolicyConfig::loadFromApmXmlConfigWithFallback()->getEngineLibraryNameSuffix();
    }

    audio_policy_forced_cfg_t curForceUse = mEngine->getForceUse(AUDIO_POLICY_FORCE_FOR_MEDIA);
    dprintf(fd, "------ Amlogic_AudioPolicyManager (APM xml Engine: %s) forceuse:%s -------",
        engineSuffix.c_str(), forceUse2Str(curForceUse));
    AudioPolicyManager::dump(fd);
    return NO_ERROR;
}

} // namespace android
