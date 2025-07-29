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

#define LOG_TAG "DroidAudioManagerSetting"
//#define LOG_NDEBUG 0

#include <system/audio-base.h>
#include "unistd.h"
#include <log/log.h>

#include <media/AidlConversion.h>
#include <media/AudioSystem.h>


#include "DroidAudioCommon.h"
#include "DroidAudioClientUtils.h"
#include "DroidAudioDb.h"
#include "DroidAudioManagerSetting.h"
#include "DroidAudioPatchManager.h"
#include "DroidAudioManager.h"
#include "SystemControlClient.h"
#include "DroidAudioManagerSetting.h"
#include "DroidAudioEffectSetting.h"
#include "DroidAudioEffect.h"

using namespace std;
using namespace android;


static sp<SystemControlClient> g_SystemControlClient;

static const char* INI_KEY_AM_AUDIO_MANAGER_COEXIST_SPDIF_OTHER                             = "ini_key_am_audio_manager_coexist_spdif_other";

static const char* DB_KEY_AM_AUDIO_CONFIG_AUDIO_SOUNDBAR_MODE_ENABLE                        = "db_key_am_audio_config_audio_soundbar_mode_enable";
static const char* DB_KEY_AM_AUDIO_CONFIG_DOLBY_AUDIO_DRC_MODE                              = "db_key_am_audio_config_dolby_audio_drc_mode";
static const char* DB_KEY_AM_AUDIO_CONFIG_DIGITAL_AUDIO_MODE                                = "db_key_am_audio_config_digital_audio_mode";
static const char* DB_KEY_AM_AUDIO_CONFIG_SOUND_SPDIF_OUTPUT_ENABLE                         = "db_key_am_audio_config_sound_spdif_output_enable";
static const char* DB_KEY_AM_AUDIO_CONFIG_SOUND_SPEAKER_OUTPUT_ENABLE                       = "db_key_am_audio_config_sound_speaker_output_enable";
static const char* DB_KEY_AM_AUDIO_CONFIG_AUDIO_DIALOGUE_ENHANCEMENT_SWITCH                 = "db_key_am_audio_config_audio_dialogue_enhancement_switch";
static const char* DB_KEY_AM_AUDIO_CONFIG_SOUND_DMX_MODE                                    = "db_key_am_audio_config_sound_dmx_mode";
static const char* DB_KEY_AM_AUDIO_CONFIG_FORCE_DDP_SWITCH                                  = "db_key_am_audio_config_force_ddp_switch";
static const char* DB_KEY_AM_AUDIO_CONFIG_DOLBY_AUDIO_DRC_LEVEL                             = "db_key_am_audio_config_dolby_audio_drc_level";
static const char* DB_KEY_AM_AUDIO_CONFIG_AUDIO_OUTPUT_SPEAKER_DELAY_ATV                    = "db_key_am_audio_config_audio_output_speaker_delay_atv";
static const char* DB_KEY_AM_AUDIO_CONFIG_AUDIO_OUTPUT_SPEAKER_DELAY_DTV                    = "db_key_am_audio_config_audio_output_speaker_delay_dtv";
static const char* DB_KEY_AM_AUDIO_CONFIG_AUDIO_OUTPUT_SPEAKER_DELAY_AV                     = "db_key_am_audio_config_audio_output_speaker_delay_av";
static const char* DB_KEY_AM_AUDIO_CONFIG_AUDIO_OUTPUT_SPEAKER_DELAY_HDMI                   = "db_key_am_audio_config_audio_output_speaker_delay_hdmi";
static const char* DB_KEY_AM_AUDIO_CONFIG_AUDIO_OUTPUT_SPEAKER_DELAY_MEDIA                  = "db_key_am_audio_config_audio_output_speaker_delay_media";
static const char* DB_KEY_AM_AUDIO_CONFIG_AUDIO_OUTPUT_SPDIF_DELAY_ATV                      = "db_key_am_audio_config_audio_output_spdif_delay_atv";
static const char* DB_KEY_AM_AUDIO_CONFIG_AUDIO_OUTPUT_SPDIF_DELAY_DTV                      = "db_key_am_audio_config_audio_output_spdif_delay_dtv";
static const char* DB_KEY_AM_AUDIO_CONFIG_AUDIO_OUTPUT_SPDIF_DELAY_AV                       = "db_key_am_audio_config_audio_output_spdif_delay_av";
static const char* DB_KEY_AM_AUDIO_CONFIG_AUDIO_OUTPUT_SPDIF_DELAY_HDMI                     = "db_key_am_audio_config_audio_output_spdif_delay_hdmi";
static const char* DB_KEY_AM_AUDIO_CONFIG_AUDIO_OUTPUT_SPDIF_DELAY_MEDIA                    = "db_key_am_audio_config_audio_output_spdif_delay_media";
static const char* DB_KEY_AM_AUDIO_CONFIG_AUDIO_OUTPUT_HEADPHONE_DELAY_ATV                  = "db_key_am_audio_config_audio_output_headphone_delay_atv";
static const char* DB_KEY_AM_AUDIO_CONFIG_AUDIO_OUTPUT_HEADPHONE_DELAY_DTV                  = "db_key_am_audio_config_audio_output_headphone_delay_dtv";
static const char* DB_KEY_AM_AUDIO_CONFIG_AUDIO_OUTPUT_HEADPHONE_DELAY_AV                   = "db_key_am_audio_config_audio_output_headphone_delay_av";
static const char* DB_KEY_AM_AUDIO_CONFIG_AUDIO_OUTPUT_HEADPHONE_DELAY_HDMI                 = "db_key_am_audio_config_audio_output_headphone_delay_hdmi";
static const char* DB_KEY_AM_AUDIO_CONFIG_AUDIO_OUTPUT_HEADPHONE_DELAY_MEDIA                = "db_key_am_audio_config_audio_output_headphone_delay_media";
static const char* DB_KEY_AM_AUDIO_CONFIG_AUDIO_OUTPUT_ALL_DELAY                            = "db_key_am_audio_config_audio_output_all_delay";
static const char* DB_KEY_AM_AUDIO_CONFIG_DTS_X_AUDIO_DRC_MODE                              = "db_key_am_audio_config_dts_x_audio_drc_mode";
static const char* DB_KEY_AM_AUDIO_CONFIG_AIDE_ENABLE                                       = "db_key_am_audio_config_aide_enable";
static const char* DB_KEY_AM_AUDIO_CONFIG_AIDE_GAIN                                         = "db_key_am_audio_config_aide_gain";
static const char* DB_KEY_AM_AUDIO_CONFIG_SOUND_LEVELER_MODE                                = "db_key_am_audio_config_sound_leveler_mode";
static const char* DB_KEY_AM_AUDIO_CONFIG_SOUND_LEVELER_AMOUNT                              = "db_key_am_audio_config_sound_leveler_amount";
static const char* DB_KEY_AM_AUDIO_CONFIG_SOUND_MIC_SOURCE                                  = "db_key_am_audio_config_sound_mic_source";
static const char* DB_KEY_AM_AUDIO_CONFIG_SOUND_GLOBAL_LINEIN_MIC_ENABLE                    = "db_key_am_audio_config_sound_global_linein_mic_enable";
static const char* DB_KEY_AM_AUDIO_CONFIG_SOUND_LINEIN_MIC_MUTE                             = "db_key_am_audio_config_sound_linein_mic_mute";
static const char* DB_KEY_AM_AUDIO_CONFIG_SOUND_LINEIN_MIC_GAIN                             = "db_key_am_audio_config_sound_linein_mic_gain";
static const char* DB_KEY_AM_AUDIO_CONFIG_SOUND_LINEIN_MIC_REVERB                           = "db_key_am_audio_config_sound_linein_mic_reverb";
static const char* DB_KEY_AM_AUDIO_CONFIG_SOUND_LINEIN_MIC_REVERB_LEVEL                     = "db_key_am_audio_config_sound_linein_mic_reverb_level";
static const char* DB_KEY_AM_AUDIO_CONFIG_GLOBAL_USB_MIC_ENABLE                             = "db_key_am_audio_config_global_usb_mic_enable";
static const char* DB_KEY_AM_AUDIO_CONFIG_USB_MIC_MUTE                                      = "db_key_am_audio_config_usb_mic_mute";
static const char* DB_KEY_AM_AUDIO_CONFIG_USB_MIC_GAIN                                      = "db_key_am_audio_config_usb_mic_gain";
static const char* DB_KEY_AM_AUDIO_CONFIG_USB_MIC_REVERB                                    = "db_key_am_audio_config_usb_mic_reverb";
static const char* DB_KEY_AM_AUDIO_CONFIG_USB_MIC_REVERB_LEVEL                              = "db_key_am_audio_config_usb_mic_reverb_level";

static const vector<const char*> g_VecDbString = {
    DB_KEY_AM_AUDIO_CONFIG_AUDIO_SOUNDBAR_MODE_ENABLE,
    DB_KEY_AM_AUDIO_CONFIG_DOLBY_AUDIO_DRC_MODE,
    DB_KEY_AM_AUDIO_CONFIG_DIGITAL_AUDIO_MODE,
    DB_KEY_AM_AUDIO_CONFIG_SOUND_SPDIF_OUTPUT_ENABLE,
    DB_KEY_AM_AUDIO_CONFIG_SOUND_SPEAKER_OUTPUT_ENABLE,
    DB_KEY_AM_AUDIO_CONFIG_AUDIO_DIALOGUE_ENHANCEMENT_SWITCH,
    DB_KEY_AM_AUDIO_CONFIG_SOUND_DMX_MODE,
    DB_KEY_AM_AUDIO_CONFIG_FORCE_DDP_SWITCH,
    DB_KEY_AM_AUDIO_CONFIG_DOLBY_AUDIO_DRC_LEVEL,
    DB_KEY_AM_AUDIO_CONFIG_AUDIO_OUTPUT_SPEAKER_DELAY_ATV,
    DB_KEY_AM_AUDIO_CONFIG_AUDIO_OUTPUT_SPEAKER_DELAY_DTV,
    DB_KEY_AM_AUDIO_CONFIG_AUDIO_OUTPUT_SPEAKER_DELAY_AV,
    DB_KEY_AM_AUDIO_CONFIG_AUDIO_OUTPUT_SPEAKER_DELAY_HDMI,
    DB_KEY_AM_AUDIO_CONFIG_AUDIO_OUTPUT_SPEAKER_DELAY_MEDIA,
    DB_KEY_AM_AUDIO_CONFIG_AUDIO_OUTPUT_SPDIF_DELAY_ATV,
    DB_KEY_AM_AUDIO_CONFIG_AUDIO_OUTPUT_SPDIF_DELAY_DTV,
    DB_KEY_AM_AUDIO_CONFIG_AUDIO_OUTPUT_SPDIF_DELAY_AV,
    DB_KEY_AM_AUDIO_CONFIG_AUDIO_OUTPUT_SPDIF_DELAY_HDMI,
    DB_KEY_AM_AUDIO_CONFIG_AUDIO_OUTPUT_SPDIF_DELAY_MEDIA,
    DB_KEY_AM_AUDIO_CONFIG_AUDIO_OUTPUT_HEADPHONE_DELAY_ATV,
    DB_KEY_AM_AUDIO_CONFIG_AUDIO_OUTPUT_HEADPHONE_DELAY_DTV,
    DB_KEY_AM_AUDIO_CONFIG_AUDIO_OUTPUT_HEADPHONE_DELAY_AV,
    DB_KEY_AM_AUDIO_CONFIG_AUDIO_OUTPUT_HEADPHONE_DELAY_HDMI,
    DB_KEY_AM_AUDIO_CONFIG_AUDIO_OUTPUT_HEADPHONE_DELAY_MEDIA,
    DB_KEY_AM_AUDIO_CONFIG_AUDIO_OUTPUT_ALL_DELAY,
    DB_KEY_AM_AUDIO_CONFIG_DTS_X_AUDIO_DRC_MODE,
    DB_KEY_AM_AUDIO_CONFIG_AIDE_ENABLE,
    DB_KEY_AM_AUDIO_CONFIG_AIDE_GAIN,
    DB_KEY_AM_AUDIO_CONFIG_SOUND_LEVELER_MODE,
    DB_KEY_AM_AUDIO_CONFIG_SOUND_LEVELER_AMOUNT,
    DB_KEY_AM_AUDIO_CONFIG_SOUND_MIC_SOURCE,
    DB_KEY_AM_AUDIO_CONFIG_SOUND_GLOBAL_LINEIN_MIC_ENABLE,
    DB_KEY_AM_AUDIO_CONFIG_SOUND_LINEIN_MIC_MUTE,
    DB_KEY_AM_AUDIO_CONFIG_SOUND_LINEIN_MIC_GAIN,
    DB_KEY_AM_AUDIO_CONFIG_SOUND_LINEIN_MIC_REVERB,
    DB_KEY_AM_AUDIO_CONFIG_SOUND_LINEIN_MIC_REVERB_LEVEL,
    DB_KEY_AM_AUDIO_CONFIG_GLOBAL_USB_MIC_ENABLE,
    DB_KEY_AM_AUDIO_CONFIG_USB_MIC_MUTE,
    DB_KEY_AM_AUDIO_CONFIG_USB_MIC_GAIN,
    DB_KEY_AM_AUDIO_CONFIG_USB_MIC_REVERB,
    DB_KEY_AM_AUDIO_CONFIG_USB_MIC_REVERB_LEVEL,
};

DroidAudioManagerSetting::DroidAudioManagerSetting():
    DroidAudioDbDescriptor(DroidAudioDbDescriptor::DROIDLOGIC_DB_MODULE_ID_AM_AUDIO_MANAGER, g_VecDbString),
        mCurTvSource(DroidAudioManager::AUDIO_OUTPUT_DELAY_SOURCE_MEDIA) {
    DroidAudioPatchManager::instance();
    AM_LOGI("");
}

vector<uint8_t> DroidAudioManagerSetting::getDefaultValue(const string& key) {
    int32_t value = getIntFromIni(key);
    if (isSoundbarPlatform() && (key == DB_KEY_AM_AUDIO_CONFIG_AUDIO_SOUNDBAR_MODE_ENABLE)) {
        value = 1;
    }
    vector<uint8_t> valueBytes(sizeof(int32_t));
    memcpy(valueBytes.data(), &value, sizeof(int32_t));
    return valueBytes;
}

void DroidAudioManagerSetting::reloadAudio() {
    if (isDriverBaseProject()) {
        AM_LOGI("driver base project, do not reload");
    } else {
        mInitStatus = false;
        init();
    }
    DroidAudioPatchManager::instance().reloadAudio();
}

int32_t DroidAudioManagerSetting::init(bool reset) {
    AM_LOGI("init");
    if (g_SystemControlClient == nullptr) {
        g_SystemControlClient = ::android::SystemControlClient::getInstance();
    }
    if (isDriverBaseProject()) {
        AM_LOGI("driver base project, do not init");
        mInitStatus = true;
        return 0;
    }
    if (mInitStatus && !reset) {
        AM_LOGW("It's already initialized");
        return 0;
    }
    if (!isTvPlatform() && !reset) {
        setSoundBarModeEnabled(isSoundBarModeEnabled());
    }
    bool isSupportDolby = g_SystemControlClient->getPropertyBoolean("ro.vendor.platform.support.dolby", false);
    if (isSupportDolby) {
        setDolbyDrcMode(getDolbyDrcMode());
    }
    setSpeakerEnabled(isSpeakerEnabled());
    setDialogEnhancerLevel(getDialogEnhancerLevel());
    setSoundDmxMode(getSoundDmxMode());
    setForceDDPEnabled(isForceDDPEnabled());
    setDtsXDrcEnabled(isDtsXDrcEnabled());

    setOutputDeviceDelay(mCurTvSource, DroidAudioManager::AUDIO_OUT_DELAY_DEV_HAL_SPEAKER,
        getOutputDeviceDelay(mCurTvSource, DroidAudioManager::AUDIO_OUT_DELAY_DEV_HAL_SPEAKER));
    setOutputDeviceDelay(mCurTvSource, DroidAudioManager::AUDIO_OUT_DELAY_DEV_HAL_SPDIF,
        getOutputDeviceDelay(mCurTvSource, DroidAudioManager::AUDIO_OUT_DELAY_DEV_HAL_SPDIF));
    setOutputDeviceDelay(mCurTvSource, DroidAudioManager::AUDIO_OUT_DELAY_DEV_HAL_HEADPHONE,
        getOutputDeviceDelay(mCurTvSource, DroidAudioManager::AUDIO_OUT_DELAY_DEV_HAL_HEADPHONE));
    setAudioOutputAllDelay(getAudioOutputAllDelay());
    setCoexistSpdifOtherEnabled(isCoexistSpdifOtherEnabled());
    int32_t forceUse = g_SystemControlClient->getPropertyInt(PROP_AUDIO_OUTPUT_FORCEUSE, DroidAudioManager::DROID_AUDIO_FORCE_USE_NONE);
    vector<int32_t> devices;
    devices.push_back(forceUse);
    setOutputDevices(devices);
    resetMicSettings();
    if (isAiDeEnabled()) {
        setAiDeEnabled(true);
        setAiDeGain(getAiDeGain());
    } else {
        setAiDeGain(0);
        setAiDeEnabled(false);
    }
    DroidAudioPatchManager::instance().init();
    mInitStatus = true;
    return 0;
}

int32_t DroidAudioManagerSetting::reset() {
    AM_LOGI("reset");
    if (isDriverBaseProject()) {
        AM_LOGI("driver base project, do not reset");
        return 0;
    }
    resetDefaultValue();
    int32_t defaultVal = getIntFromIni(INI_KEY_AM_AUDIO_MANAGER_COEXIST_SPDIF_OTHER);
    g_SystemControlClient->setProperty(PROP_AUDIO_OUTPUT_SPDIF_COEXIST, defaultVal == 0 ? "0" : "1");
    g_SystemControlClient->setProperty(PROP_AUDIO_OUTPUT_FORCEUSE, to_string(DroidAudioManager::DROID_AUDIO_FORCE_USE_NONE).c_str());
    init(true);
    return 0;
}

void DroidAudioManagerSetting::audioPortOrPatchUpdate() {
    DroidAudioPatchManager::instance().audioPortOrPatchUpdate();
}

int32_t DroidAudioManagerSetting::getAudioFunctionConfig(int id) {
    return DroidAudioEffectSetting::instance()->getEffectFunctionConfig(id);
}

int32_t DroidAudioManagerSetting::dump(int32_t fd, const char **args, uint32_t numArg) {
    dprintf(fd, "------------------------ DroidAudioManagerSetting -----------------------\n");
    int32_t forceUse = g_SystemControlClient->getPropertyInt(PROP_AUDIO_OUTPUT_FORCEUSE, DroidAudioManager::DROID_AUDIO_FORCE_USE_NONE);
    dprintf(fd, "db forceUse:                           %10d\n", forceUse);
    DroidAudioPatchManager::instance().dump(fd, args, numArg);
    return STATUS_OK;
}

int32_t DroidAudioManagerSetting::setAudioCmdParam(int32_t cmd, int32_t param1, int32_t param2, int32_t param3) {
    return DroidAudioPatchManager::instance().setAudioCmdParam(cmd, param1, param2, param3);
}

int32_t DroidAudioManagerSetting::setOutputDevices(const vector<int32_t>& devices) {
    if (devices.size() == 0 || devices.size() > 1) {
        AM_LOGW("devices size is:%zu", devices.size());
        return -1;
    }

    switch (devices[0]) {
        case DroidAudioManager::DROID_AUDIO_FORCE_USE_NONE:
        case DroidAudioManager::DROID_AUDIO_FORCE_USE_SPEAKER:
        case DroidAudioManager::DROID_AUDIO_FORCE_USE_SPDIF:
        case DroidAudioManager::DROID_AUDIO_FORCE_USE_HEADPHONES:
        case DroidAudioManager::DROID_AUDIO_FORCE_USE_HDMI:
        case DroidAudioManager::DROID_AUDIO_FORCE_USE_USB:
        case DroidAudioManager::DROID_AUDIO_FORCE_USE_BT_A2DP:
            break;
        default:
            AM_LOGW("unsupported forceUse:%d", devices[0]);
            return -1;
    }
    AM_LOGI("setForceUse:%d", devices[0]);
    g_SystemControlClient->setProperty(PROP_AUDIO_OUTPUT_FORCEUSE, to_string(devices[0]).c_str());
    AudioSystem::setForceUse(AUDIO_POLICY_FORCE_FOR_MEDIA, (audio_policy_forced_cfg_t)devices[0]);
    return 0;
}

int32_t DroidAudioManagerSetting::getOutputDevices(vector<int32_t>* devices) {
    R_CHECK_POINTER_LEGAL(-1, devices,)
    int32_t audioOutStrategy = g_SystemControlClient->getPropertyInt(PROP_AUDIO_OUTPUT_STRATEGY, DROID_AUDIO_OUTPUT_STRATEGY_AUTO);
    int32_t forceUse = 0;
    if (audioOutStrategy == DROID_AUDIO_OUTPUT_STRATEGY_AUTO || audioOutStrategy == DROID_AUDIO_OUTPUT_STRATEGY_SEMI_AUTO) {
        AudioDeviceTypeAddrVector curDevices{};
        audio_attributes_t attributes = AudioSystem::streamTypeToAttributes(AUDIO_STREAM_MUSIC);
        AudioSystem::getDevicesForAttributes(attributes, &curDevices, false);
        for (auto device : curDevices) {
            if (device.mType == AUDIO_DEVICE_OUT_FM && audioOutStrategy == DROID_AUDIO_OUTPUT_STRATEGY_SEMI_AUTO) {
                int32_t forceUse = g_SystemControlClient->getPropertyInt(PROP_AUDIO_OUTPUT_FORCEUSE, DroidAudioManager::DROID_AUDIO_FORCE_USE_NONE);
                devices->push_back(forceUse);
                return 0;
            }
            switch (device.mType) {
                case AUDIO_DEVICE_OUT_SPEAKER:
                    forceUse = DroidAudioManager::DROID_AUDIO_FORCE_USE_SPEAKER;
                    break;
                case AUDIO_DEVICE_OUT_SPDIF:
                    forceUse = DroidAudioManager::DROID_AUDIO_FORCE_USE_SPDIF;
                    break;
                case AUDIO_DEVICE_OUT_WIRED_HEADSET:
                case AUDIO_DEVICE_OUT_WIRED_HEADPHONE:
                    forceUse = DroidAudioManager::DROID_AUDIO_FORCE_USE_HEADPHONES;
                    break;
                case AUDIO_DEVICE_OUT_HDMI:
                case AUDIO_DEVICE_OUT_HDMI_ARC:
                case AUDIO_DEVICE_OUT_HDMI_EARC:
                    forceUse = DroidAudioManager::DROID_AUDIO_FORCE_USE_HDMI;
                    break;
                case AUDIO_DEVICE_OUT_USB_DEVICE:
                case AUDIO_DEVICE_OUT_USB_ACCESSORY:
                case AUDIO_DEVICE_OUT_USB_HEADSET:
                    forceUse = DroidAudioManager::DROID_AUDIO_FORCE_USE_USB;
                    break;
                case AUDIO_DEVICE_OUT_BLUETOOTH_A2DP:
                case AUDIO_DEVICE_OUT_BLUETOOTH_A2DP_HEADPHONES:
                case AUDIO_DEVICE_OUT_BLUETOOTH_A2DP_SPEAKER:
                    forceUse = DroidAudioManager::DROID_AUDIO_FORCE_USE_BT_A2DP;
                    break;
                default:
                    AM_LOGW("unsupported dev0:%#x", device.mType);
                    return -1;
            }
            devices->push_back(forceUse);
            if (isAudioDebug()) {
                AM_LOGI("find device:%s", audio_device_to_string(device.mType));
            }
        }
        if (devices->size() == 0) {
            AM_LOGW("not find sink device");
        }
    } else {
        forceUse = (int32_t)AudioSystem::getForceUse(AUDIO_POLICY_FORCE_FOR_MEDIA);
        switch (forceUse) {
            case DroidAudioManager::DROID_AUDIO_FORCE_USE_SPEAKER:
            case DroidAudioManager::DROID_AUDIO_FORCE_USE_SPDIF:
            case DroidAudioManager::DROID_AUDIO_FORCE_USE_HEADPHONES:
            case DroidAudioManager::DROID_AUDIO_FORCE_USE_HDMI:
            case DroidAudioManager::DROID_AUDIO_FORCE_USE_USB:
            case DroidAudioManager::DROID_AUDIO_FORCE_USE_BT_A2DP:
                break;
            default:
                AM_LOGW("unsupported forceUse:%d", forceUse);
                return -1;
        }
        devices->push_back(forceUse);
        if (isAudioDebug()) {
            AM_LOGD("return forceUse:%d", forceUse);
        }
    }
    return 0;
}

int32_t DroidAudioManagerSetting::updateCoexistSpdifOther() {
    int32_t coexist = isCoexistSpdifOtherEnabled() ? 1 : 0;
    int32_t curState = AudioSystem::getDeviceConnectionState(AUDIO_DEVICE_OUT_SPDIF, "");
    AM_LOGI("coexist:%d, curState:%d", coexist, curState);

    struct audio_port_v7 audioPort{};
    audioPort.type = AUDIO_PORT_TYPE_DEVICE;
    audioPort.ext.device.type = AUDIO_DEVICE_OUT_SPDIF;
    android::media::audio::common::AudioPort aidlAudioPort = legacy2aidl_audio_port_v7_AudioPort(audioPort, false).value();
    if (coexist == curState) {
        audio_policy_dev_state_t state = (coexist == 1) ? AUDIO_POLICY_DEVICE_STATE_UNAVAILABLE : AUDIO_POLICY_DEVICE_STATE_AVAILABLE;
        AudioSystem::setDeviceConnectionState(state, aidlAudioPort, AUDIO_FORMAT_DEFAULT);
    }
    return 0;
}

int32_t DroidAudioManagerSetting::setCoexistSpdifOtherEnabled(bool enable) {
    if (isAudioDebug()) AM_LOGD("enable: %d", enable);
    g_SystemControlClient->setProperty(PROP_AUDIO_OUTPUT_SPDIF_COEXIST, enable ? "1" : "0");
    updateCoexistSpdifOther();
    int32_t forceUse = AudioSystem::getForceUse(AUDIO_POLICY_FORCE_FOR_MEDIA);
    if (forceUse == DroidAudioManager::DROID_AUDIO_FORCE_USE_SPDIF) {
        g_SystemControlClient->setProperty(PROP_AUDIO_OUTPUT_STRATEGY, to_string(DROID_AUDIO_OUTPUT_STRATEGY_AUTO).c_str());
        AudioSystem::setForceUse(AUDIO_POLICY_FORCE_FOR_MEDIA, (audio_policy_forced_cfg_t)DroidAudioManager::DROID_AUDIO_FORCE_USE_NONE);
        AM_LOGI("delete spdif, setForceUse NONE. enable:%d", enable);
    }
    ::setParameters("hal_param_spdif_coexist_other=", (int32_t)enable);
    return 0;
}
bool DroidAudioManagerSetting::isCoexistSpdifOtherEnabled() {
    int32_t defaultVal = getIntFromIni(INI_KEY_AM_AUDIO_MANAGER_COEXIST_SPDIF_OTHER);
    bool enable = g_SystemControlClient->getPropertyInt(PROP_AUDIO_OUTPUT_SPDIF_COEXIST, defaultVal) != 0;
    if (isAudioDebug()) AM_LOGD("enable: %d", enable);
    return enable;
}

int32_t DroidAudioManagerSetting::setSoundBarModeEnabled(bool enable) {
    const char* SYS_HDMITX_AUDIO_SOUNDBAR_EN = "/sys/class/amhdmitx/amhdmitx0/soundbar_en";
    if (isAudioDebug()) AM_LOGD("setSoundBarModeEnabled soundbar:%d", enable);
    putToDb(DB_KEY_AM_AUDIO_CONFIG_AUDIO_SOUNDBAR_MODE_ENABLE, enable ? 1 : 0);
    AudioSystem::setMasterMute(true);
    g_SystemControlClient->setProperty("persist.vendor.media.audio.soundbar.mode", enable ? "1" : "0");
    g_SystemControlClient->writeSysfs(SYS_HDMITX_AUDIO_SOUNDBAR_EN, enable ? "1" : "0");
    usleep(200 * 1000);
    ::setParameters("hal_param_soundbar_mode=", (enable ? 1 : 0));
    usleep(800 * 1000);
    AudioSystem::setMasterMute(false);
    return 0;
}

bool DroidAudioManagerSetting::isSoundBarModeEnabled() {
    return getIntFromDb(DB_KEY_AM_AUDIO_CONFIG_AUDIO_SOUNDBAR_MODE_ENABLE) != 0;
}

int32_t DroidAudioManagerSetting::setSoundSpdifEnabled(bool enable) {
    const string HAL_PARAM_SPDIF_OUTPUT_ENABLE = "hal_param_spdif_output_enable=";
    if (isAudioDebug()) AM_LOGD("enable: %d", enable);
    putToDb(DB_KEY_AM_AUDIO_CONFIG_SOUND_SPDIF_OUTPUT_ENABLE, enable ? 1 : 0);
    ::setParameters(HAL_PARAM_SPDIF_OUTPUT_ENABLE, (enable ? 1 : 0));
    return 0;
}

bool DroidAudioManagerSetting::isSoundSpdifEnabled() {
    bool enable = getIntFromDb(DB_KEY_AM_AUDIO_CONFIG_SOUND_SPDIF_OUTPUT_ENABLE) != 0;
    if (isAudioDebug()) AM_LOGD("enable: %d", enable);
    return enable;
}

int32_t DroidAudioManagerSetting::setSpeakerEnabled(bool enable) {
    const string HAL_PARAM_SPEAKER_OUTPUT_MUTE = "cmd_aed_lr_ch_volume_mute=";
    if (isAudioDebug()) AM_LOGD("enable: %d", enable);
    ::setParameters(HAL_PARAM_SPEAKER_OUTPUT_MUTE, (enable ? 0 : 1));
    putToDb(DB_KEY_AM_AUDIO_CONFIG_SOUND_SPEAKER_OUTPUT_ENABLE, enable ? 1 : 0);
    return 0;
}

bool DroidAudioManagerSetting::isSpeakerEnabled() {
    bool enable = getIntFromDb(DB_KEY_AM_AUDIO_CONFIG_SOUND_SPEAKER_OUTPUT_ENABLE) != 0;
    if (isAudioDebug()) AM_LOGD("enable: %d", enable);
    return enable;
}

void setAudioOutputDelayToHal(int32_t output, int32_t delayMs) {
    const string PARAM_HAL_AUDIO_OUT_DEV_DELAY = "hal_param_out_dev_delay_time_ms=";
    /* High 16 - bit expression type, low 16 - bit expression delay time. refer to audio hal */
    ::setParameters(PARAM_HAL_AUDIO_OUT_DEV_DELAY, (output << 16 | delayMs));
}

static string getOutputDelayDbId(int32_t source, int32_t device) {
    switch (source) {
        case DroidAudioManager::AUDIO_OUTPUT_DELAY_SOURCE_ATV:
            if (device == DroidAudioManager::AUDIO_OUT_DELAY_DEV_HAL_SPEAKER) {
                return DB_KEY_AM_AUDIO_CONFIG_AUDIO_OUTPUT_SPEAKER_DELAY_ATV;
            } else if (device == DroidAudioManager::AUDIO_OUT_DELAY_DEV_HAL_SPDIF) {
                return DB_KEY_AM_AUDIO_CONFIG_AUDIO_OUTPUT_SPDIF_DELAY_ATV;
            } else if (device == DroidAudioManager::AUDIO_OUT_DELAY_DEV_HAL_HEADPHONE) {
                return DB_KEY_AM_AUDIO_CONFIG_AUDIO_OUTPUT_HEADPHONE_DELAY_ATV;
            }
            break;
        case DroidAudioManager::AUDIO_OUTPUT_DELAY_SOURCE_DTV:
            if (device == DroidAudioManager::AUDIO_OUT_DELAY_DEV_HAL_SPEAKER) {
                return DB_KEY_AM_AUDIO_CONFIG_AUDIO_OUTPUT_SPEAKER_DELAY_DTV;
            } else if (device == DroidAudioManager::AUDIO_OUT_DELAY_DEV_HAL_SPDIF) {
                return DB_KEY_AM_AUDIO_CONFIG_AUDIO_OUTPUT_SPDIF_DELAY_DTV;
            } else if (device == DroidAudioManager::AUDIO_OUT_DELAY_DEV_HAL_HEADPHONE) {
                return DB_KEY_AM_AUDIO_CONFIG_AUDIO_OUTPUT_HEADPHONE_DELAY_DTV;
            }
            break;
        case DroidAudioManager::AUDIO_OUTPUT_DELAY_SOURCE_AV:
            if (device == DroidAudioManager::AUDIO_OUT_DELAY_DEV_HAL_SPEAKER) {
                return DB_KEY_AM_AUDIO_CONFIG_AUDIO_OUTPUT_SPEAKER_DELAY_AV;
            } else if (device == DroidAudioManager::AUDIO_OUT_DELAY_DEV_HAL_SPDIF) {
                return DB_KEY_AM_AUDIO_CONFIG_AUDIO_OUTPUT_SPDIF_DELAY_AV;
            } else if (device == DroidAudioManager::AUDIO_OUT_DELAY_DEV_HAL_HEADPHONE) {
                return DB_KEY_AM_AUDIO_CONFIG_AUDIO_OUTPUT_HEADPHONE_DELAY_AV;
            }
            break;
        case DroidAudioManager::AUDIO_OUTPUT_DELAY_SOURCE_HDMI:
            if (device == DroidAudioManager::AUDIO_OUT_DELAY_DEV_HAL_SPEAKER) {
                return DB_KEY_AM_AUDIO_CONFIG_AUDIO_OUTPUT_SPEAKER_DELAY_HDMI;
            } else if (device == DroidAudioManager::AUDIO_OUT_DELAY_DEV_HAL_SPDIF) {
                return DB_KEY_AM_AUDIO_CONFIG_AUDIO_OUTPUT_SPDIF_DELAY_HDMI;
            } else if (device == DroidAudioManager::AUDIO_OUT_DELAY_DEV_HAL_HEADPHONE) {
                return DB_KEY_AM_AUDIO_CONFIG_AUDIO_OUTPUT_HEADPHONE_DELAY_HDMI;
            }
            break;
        case DroidAudioManager::AUDIO_OUTPUT_DELAY_SOURCE_MEDIA:
            if (device == DroidAudioManager::AUDIO_OUT_DELAY_DEV_HAL_SPEAKER) {
                return DB_KEY_AM_AUDIO_CONFIG_AUDIO_OUTPUT_SPEAKER_DELAY_MEDIA;
            } else if (device == DroidAudioManager::AUDIO_OUT_DELAY_DEV_HAL_SPDIF) {
                return DB_KEY_AM_AUDIO_CONFIG_AUDIO_OUTPUT_SPDIF_DELAY_MEDIA;
            } else if (device == DroidAudioManager::AUDIO_OUT_DELAY_DEV_HAL_HEADPHONE) {
                return DB_KEY_AM_AUDIO_CONFIG_AUDIO_OUTPUT_HEADPHONE_DELAY_MEDIA;
            }
            break;
        default:
            AM_LOGW("invalid source:%d device:%d(%s)", source, device, DroidAudioManager::audioDelayDev2Str(device));
            return "";
    }
    AM_LOGW("invalid device:%d(%s) source:%d", device, DroidAudioManager::audioDelayDev2Str(device), source);
    return "";
}

int32_t DroidAudioManagerSetting::setOutputDeviceDelay(int32_t source, int32_t device, int32_t delayMs) {
    string dbStr = getOutputDelayDbId(source, device);
    if (dbStr == "") {
        return -1;
    }
    if (delayMs < DroidAudioManager::AUDIO_OUT_DELAY_HAL_MIN || delayMs > DroidAudioManager::AUDIO_OUT_DELAY_HAL_MAX) {
        AM_LOGW("unsupport delay time:%d ms, min:%d, max:%d, now use max value",
            delayMs, DroidAudioManager::AUDIO_OUT_DELAY_HAL_MIN, DroidAudioManager::AUDIO_OUT_DELAY_HAL_MAX);
        delayMs = DroidAudioManager::AUDIO_OUT_DELAY_HAL_MAX;
    }
    if (isAudioDebug()) AM_LOGD("%s(%d) source:%s(%d), delayMs:%d", DroidAudioManager::audioDelayDev2Str(device), device,
        DroidAudioManager::tvSource2Str(source), source, delayMs);
    if (mCurTvSource == source) {
        setAudioOutputDelayToHal(device, delayMs);
    } else {
        AM_LOGI("%s(%d) cur source:%s(%d) is not the same as the set source:%s(%d), only save to DB",
            DroidAudioManager::audioDelayDev2Str(device), device,
            DroidAudioManager::tvSource2Str(mCurTvSource), mCurTvSource,
            DroidAudioManager::tvSource2Str(source), source);
    }
    putToDb(dbStr, delayMs);
    return 0;
}

int32_t DroidAudioManagerSetting::getOutputDeviceDelay(int32_t source, int32_t device) {
    string dbStr = getOutputDelayDbId(source, device);
    if (dbStr == "") {
        return -1;
    }
    int32_t delayMs = getIntFromDb(dbStr);
    if (isAudioDebug()) AM_LOGD("device:%s(%d) source:%s(%d), delayMs:%d",
        DroidAudioManager::audioDelayDev2Str(device), device,
        DroidAudioManager::tvSource2Str(source), source, delayMs);
    return delayMs;
}

int32_t DroidAudioManagerSetting::setAudioOutputAllDelay(int32_t delayMs) {
    if (delayMs < DroidAudioManager::AUDIO_OUT_DELAY_HAL_MIN || delayMs > DroidAudioManager::AUDIO_OUT_DELAY_HAL_MAX) {
        AM_LOGW("unsupport delay time:%d ms, min:%d, max:%d, now use max value",
            delayMs, DroidAudioManager::AUDIO_OUT_DELAY_HAL_MIN, DroidAudioManager::AUDIO_OUT_DELAY_HAL_MAX);
        delayMs = DroidAudioManager::AUDIO_OUT_DELAY_HAL_MAX;
    }
    if (isAudioDebug()) AM_LOGD("delayMs:%d", delayMs);
    setAudioOutputDelayToHal(DroidAudioManager::AUDIO_OUT_DELAY_DEV_HAL_ALL, delayMs);
    putToDb(DB_KEY_AM_AUDIO_CONFIG_AUDIO_OUTPUT_ALL_DELAY, delayMs);
    return 0;
}

int32_t DroidAudioManagerSetting::getAudioOutputAllDelay() {
    int32_t delayMs = getIntFromDb(DB_KEY_AM_AUDIO_CONFIG_AUDIO_OUTPUT_ALL_DELAY);
    if (isAudioDebug()) AM_LOGD("delayMs:%d", delayMs);
    return delayMs;
}

int32_t DroidAudioManagerSetting::setTvSourceType(int32_t source) {
    R_CHECK_PARAM_LEGAL(-1, source, DroidAudioManager::AUDIO_OUTPUT_DELAY_SOURCE_MIN, DroidAudioManager::AUDIO_OUTPUT_DELAY_SOURCE_MAX,)
    mCurTvSource = source;
    if (isAudioDebug()) AM_LOGD("source: %s(%d)", DroidAudioManager::tvSource2Str(source), source);
    setOutputDeviceDelay(source, DroidAudioManager::AUDIO_OUT_DELAY_DEV_HAL_SPEAKER,
        getOutputDeviceDelay(source, DroidAudioManager::AUDIO_OUT_DELAY_DEV_HAL_SPEAKER));
    setOutputDeviceDelay(source, DroidAudioManager::AUDIO_OUT_DELAY_DEV_HAL_SPDIF,
        getOutputDeviceDelay(source, DroidAudioManager::AUDIO_OUT_DELAY_DEV_HAL_SPDIF));
    setOutputDeviceDelay(source, DroidAudioManager::AUDIO_OUT_DELAY_DEV_HAL_HEADPHONE,
        getOutputDeviceDelay(source, DroidAudioManager::AUDIO_OUT_DELAY_DEV_HAL_HEADPHONE));
    return 0;
}

int32_t DroidAudioManagerSetting::getTvSourceType() {
    if (isAudioDebug()) AM_LOGD("mCurTvSource: %s(%d)", DroidAudioManager::tvSource2Str(mCurTvSource), mCurTvSource);
    return mCurTvSource;
}

int32_t DroidAudioManagerSetting::setAudioApplyToAll() {
    for (int32_t source = DroidAudioManager::AUDIO_OUTPUT_DELAY_SOURCE_MIN;
            source <= DroidAudioManager::AUDIO_OUTPUT_DELAY_SOURCE_MAX; source++) {
        if (source == mCurTvSource) {
            continue;
        }
        setOutputDeviceDelay(source, DroidAudioManager::AUDIO_OUT_DELAY_DEV_HAL_SPEAKER,
        getOutputDeviceDelay(source, DroidAudioManager::AUDIO_OUT_DELAY_DEV_HAL_SPEAKER));
        setOutputDeviceDelay(source, DroidAudioManager::AUDIO_OUT_DELAY_DEV_HAL_SPDIF,
            getOutputDeviceDelay(source, DroidAudioManager::AUDIO_OUT_DELAY_DEV_HAL_SPDIF));
        setOutputDeviceDelay(source, DroidAudioManager::AUDIO_OUT_DELAY_DEV_HAL_HEADPHONE,
            getOutputDeviceDelay(source, DroidAudioManager::AUDIO_OUT_DELAY_DEV_HAL_HEADPHONE));
    }
    return 0;
}

int32_t DroidAudioManagerSetting::setDigitalAudioMode(int32_t mode, const string& formats) {
    const string HAL_PARAM_DIGITAL_AUDIO_MODE               = "hdmi_format=";
    const int32_t HAL_DIGITAL_AUDIO_MODE_PCM                = 0;
    const int32_t HAL_DIGITAL_AUDIO_MODE_AUTO               = 5;
    const int32_t HAL_DIGITAL_AUDIO_MODE_PASSTHROUGH        = 6;
    if (isAudioDebug()) AM_LOGD("mode:%s(%d), formats:%s", DroidAudioManager::audioDigitalMode2Str(mode), mode, formats.c_str());
    switch (mode) {
        case DroidAudioManager::DIGITAL_AUDIO_MODE_MANUAL:
            if (isTvPlatform()) {
                mode = DroidAudioManager::DIGITAL_AUDIO_MODE_AUTO;
            }
            ::setParameters(HAL_PARAM_DIGITAL_AUDIO_MODE, HAL_DIGITAL_AUDIO_MODE_AUTO);
            break;
        case DroidAudioManager::DIGITAL_AUDIO_MODE_AUTO:
        case DroidAudioManager::DIGITAL_AUDIO_MODE_PASSTHROUGH:
            if (mode == DroidAudioManager::DIGITAL_AUDIO_MODE_AUTO) {
                ::setParameters(HAL_PARAM_DIGITAL_AUDIO_MODE, HAL_DIGITAL_AUDIO_MODE_AUTO);
            } else {
                ::setParameters(HAL_PARAM_DIGITAL_AUDIO_MODE, HAL_DIGITAL_AUDIO_MODE_PASSTHROUGH);
            }
            break;
        case DroidAudioManager::DIGITAL_AUDIO_MODE_PCM:
        default:
            mode = DroidAudioManager::DIGITAL_AUDIO_MODE_PCM;
            ::setParameters(HAL_PARAM_DIGITAL_AUDIO_MODE, HAL_DIGITAL_AUDIO_MODE_PCM);
            break;
    }
    putToDb(DB_KEY_AM_AUDIO_CONFIG_DIGITAL_AUDIO_MODE, mode);
    return 0;
}

int32_t DroidAudioManagerSetting::getDigitalAudioMode() {
    int32_t mode = getIntFromDb(DB_KEY_AM_AUDIO_CONFIG_DIGITAL_AUDIO_MODE);
    if (isAudioDebug()) AM_LOGD("mode: %s(%d)", DroidAudioManager::audioDigitalMode2Str(mode), mode);
    return mode;
}

int32_t DroidAudioManagerSetting::setForceDDPEnabled(bool enable) {
    const string HAL_PARAM_FORCE_DDP_ENABLE = "hal_param_force_ddp=";
    if (isAudioDebug()) AM_LOGD("enable: %d", enable);
    putToDb(DB_KEY_AM_AUDIO_CONFIG_FORCE_DDP_SWITCH, enable ? 1 : 0);
    ::setParameters(HAL_PARAM_FORCE_DDP_ENABLE, (enable ? 1 : 0));
    return 0;
}

bool DroidAudioManagerSetting::isForceDDPEnabled() {
    bool enable = getIntFromDb(DB_KEY_AM_AUDIO_CONFIG_FORCE_DDP_SWITCH) != 0;
    if (isAudioDebug()) AM_LOGD("enable: %d", enable);
    return enable;
}

//const static int32_t CUSTOM_0_DRCMODE                   = 0;
//const static int32_t CUSTOM_1_DRCMODE                   = 1;
const static int32_t LINE_DRCMODE                       = 2;
const static int32_t RF_DRCMODE                         = 3;
const static string AUDIO_DSP_AC3_DRC                   = "/sys/class/audiodsp/ac3_drc_control";
void DroidAudioManagerSetting::setDolbyDrcEnabled(bool enable) {
    if (enable) { //open DRC
        g_SystemControlClient->writeSysfs(AUDIO_DSP_AC3_DRC, "drchighcutscale 0x64");
        g_SystemControlClient->writeSysfs(AUDIO_DSP_AC3_DRC, "drclowboostscale 0x64");
    } else { //close DRC
        g_SystemControlClient->writeSysfs(AUDIO_DSP_AC3_DRC, "drchighcutscale 0");
        g_SystemControlClient->writeSysfs(AUDIO_DSP_AC3_DRC, "drclowboostscale 0");
    }
}

void DroidAudioManagerSetting::setDolbyMode(int32_t mode) {
    //"CUSTOM_0","CUSTOM_1","LINE","RF"; default use "LINE"
    string tempStr = "";
    if (mode >= 0 && mode <= 3) {
        tempStr = string("drcmode") + " " + to_string(mode);
    } else {
        tempStr = string("drcmode") + " " + to_string(LINE_DRCMODE);
    }
    g_SystemControlClient->writeSysfs(AUDIO_DSP_AC3_DRC, tempStr);
}

int32_t DroidAudioManagerSetting::setDolbyDrcMode(int32_t mode) {
    const string HAL_PARAM_ENABLE_DRC_RF_MODE = "hal_param_enable_drc_rf_mode=";
    if (isAudioDebug()) AM_LOGD("mode: %d", mode);
    switch (mode) {
        case DroidAudioManager::DOLBY_DRC_MODE_OFF:
            setDolbyDrcEnabled(false);
            setDolbyMode(LINE_DRCMODE);
            break;
        case DroidAudioManager::DOLBY_DRC_MODE_LINE:
            ::setParameters(HAL_PARAM_ENABLE_DRC_RF_MODE, 0);
            setDolbyMode(LINE_DRCMODE);
            setDolbyDrcLineLevel(getDolbyDrcLineLevel());
            //default value get from INI getIntFromIni(DB_KEY_AM_AUDIO_CONFIG_DOLBY_AUDIO_DRC_LEVEL)
            break;
        case DroidAudioManager::DOLBY_DRC_MODE_RF:
            ::setParameters(HAL_PARAM_ENABLE_DRC_RF_MODE, 1);
            setDolbyDrcEnabled(false);
            setDolbyMode(RF_DRCMODE);
            break;
        default:
            AM_LOGE("not support DRC mode: %d", mode);
            return -1;
        }
    putToDb(DB_KEY_AM_AUDIO_CONFIG_DOLBY_AUDIO_DRC_MODE, mode);
    return 0;
}

int32_t DroidAudioManagerSetting::getDolbyDrcMode() {
    int32_t mode = getIntFromDb(DB_KEY_AM_AUDIO_CONFIG_DOLBY_AUDIO_DRC_MODE);
    if (isAudioDebug()) AM_LOGD("mode: %d", mode);
    return mode;
}

int32_t DroidAudioManagerSetting::setDolbyDrcLineLevel(int32_t level) {
    string tempStr = "";
    stringstream ss;
    ss << hex << level;
    string hexLineLevel = ss.str();
    ::setParameters("hal_param_drc_boost_value=", level);
    ::setParameters("hal_param_drc_cut_value=", level);
    tempStr = string("drchighcutscale") + " " + hexLineLevel;
    g_SystemControlClient->writeSysfs(AUDIO_DSP_AC3_DRC, tempStr);
    tempStr = string("drclowboostscale") + " " + hexLineLevel;
    g_SystemControlClient->writeSysfs(AUDIO_DSP_AC3_DRC, tempStr);
    putToDb(DB_KEY_AM_AUDIO_CONFIG_DOLBY_AUDIO_DRC_LEVEL, level);
    if (isAudioDebug()) AM_LOGD("level: %d, hexLineLevel:%s", level, hexLineLevel.c_str());
    return 0;
}

int32_t DroidAudioManagerSetting::getDolbyDrcLineLevel() {
    int32_t level = getIntFromDb(DB_KEY_AM_AUDIO_CONFIG_DOLBY_AUDIO_DRC_LEVEL);
    if (isAudioDebug()) AM_LOGD("level: %d", level);
    return level;
}

bool DroidAudioManagerSetting::isDtsXEnabled() {
    string value = getParameters("dts_x_enable");
    bool enable = getParameters("dts_x_enable") == "1";
    if (isAudioDebug()) AM_LOGD("enable: %d", enable);
    return enable;
}

int32_t DroidAudioManagerSetting::setDtsXDrcEnabled(bool enable) {
    if (isAudioDebug()) AM_LOGD("enable: %d", enable);
    ::setParameters("dtsx_spk_drc=", (enable != 0 ? 1 : 0));
    putToDb(DB_KEY_AM_AUDIO_CONFIG_DTS_X_AUDIO_DRC_MODE, enable);
    return 0;
}

bool DroidAudioManagerSetting::isDtsXDrcEnabled() {
    bool halEnable = getParameters("dtsx_spk_drc") == "1";
    bool saveResult = getIntFromDb(DB_KEY_AM_AUDIO_CONFIG_DTS_X_AUDIO_DRC_MODE)!= 0;
    if (saveResult != halEnable) {
        AM_LOGW("erro get:%d, saved:%d", halEnable, saveResult);
    }
    if (isAudioDebug()) AM_LOGD("enable: %d", saveResult);
    return saveResult;
}

int32_t DroidAudioManagerSetting::setDialogEnhancerLevel(int32_t level) {
    const string HAL_PARAM_OTT_DIALOGUE_ENHANCEMENT = "hal_param_dap_dialogue_enhancement=";
    const string HAL_PARAM_DIALOGUE_ENHANCEMENT = "hal_param_dialogue_enhancement=";
    int32_t ottMs12Config = getAudioFunctionConfig(DroidAudioEffect::EFFECT_CONFIG_OTT_MS12);
    if (isAudioDebug()) AM_LOGD("ottMs12Config:%d level: %d", ottMs12Config, level);
    switch (level) {
        case DroidAudioManager::DIALOGUE_ENHANCEMENT_LEVEL_OFF:
        case DroidAudioManager::DIALOGUE_ENHANCEMENT_LEVEL_LOW:
        case DroidAudioManager::DIALOGUE_ENHANCEMENT_LEVEL_MEDIUM:
        case DroidAudioManager::DIALOGUE_ENHANCEMENT_LEVEL_HIGH:
            if (ottMs12Config == DroidAudioEffect::EFFECT_CONFIG_DAP_MS12_X) {
                ::setParameters(HAL_PARAM_OTT_DIALOGUE_ENHANCEMENT, level);
                ::setParameters(HAL_PARAM_DIALOGUE_ENHANCEMENT, level);
            } else {
                ::setParameters(HAL_PARAM_DIALOGUE_ENHANCEMENT, level);
            }
            putToDb(DB_KEY_AM_AUDIO_CONFIG_AUDIO_DIALOGUE_ENHANCEMENT_SWITCH, level);
            break;
        default:
            AM_LOGW("invalid level: %d", level);
            break;
    }
    return 0;
}

int32_t DroidAudioManagerSetting::getDialogEnhancerLevel() {
    int32_t level = getIntFromDb(DB_KEY_AM_AUDIO_CONFIG_AUDIO_DIALOGUE_ENHANCEMENT_SWITCH);
    if (isAudioDebug()) AM_LOGD("level: %d", level);
    return level;
}

int32_t DroidAudioManagerSetting::setSoundDmxMode(int32_t mode) {
    const string HAL_PARAM_DXM_MODE = "hal_param_dmx_mode=";
    const string HAL_PARAM_OTT_DXM_MODE = "hal_param_dap_volume_leveler=";
    int32_t ottMs12Config = getAudioFunctionConfig(DroidAudioEffect::EFFECT_CONFIG_OTT_MS12);
    if (isAudioDebug()) AM_LOGD("ottMs12Config:%d mode: %d", ottMs12Config, mode);
    switch (mode) {
        case DroidAudioManager::DOLBY_SOUND_DMX_MODE_SURROUND:
        case DroidAudioManager::DOLBY_SOUND_DMX_MODE_STEREO:
            putToDb(DB_KEY_AM_AUDIO_CONFIG_SOUND_DMX_MODE, mode);
            if (ottMs12Config == DroidAudioEffect::EFFECT_CONFIG_DAP_MS12_X) {
                ::setParameters(HAL_PARAM_OTT_DXM_MODE, mode);
            } else {
                ::setParameters(HAL_PARAM_DXM_MODE, mode);
            }
            break;
        default:
            AM_LOGW("Invalid mode: %d", mode);
            break;
    }
    return 0;
}

int32_t DroidAudioManagerSetting::getSoundDmxMode() {
    int32_t mode = getIntFromDb(DB_KEY_AM_AUDIO_CONFIG_SOUND_DMX_MODE);
    if (isAudioDebug()) AM_LOGD("mode: %d", mode);
    return mode;
}

//OTT config X: leveler control without DAP
int32_t DroidAudioManagerSetting::setSoundLevelerMode(int32_t mode) {
    const string HAL_PARAM_DAP_LEVELER_SETTING = "hal_param_dap_leveler_setting=";
    if (isAudioDebug()) AM_LOGD("mode: %d", mode);

    switch (mode) {
        case DroidAudioManager::DOLBY_SOUND_LEVELER_MODE_OFF:
        case DroidAudioManager::DOLBY_SOUND_LEVELER_MODE_ON:
        case DroidAudioManager::DOLBY_SOUND_LEVELER_MODE_AUTO:
            putToDb(DB_KEY_AM_AUDIO_CONFIG_SOUND_LEVELER_MODE, mode);
            ::setParameters(HAL_PARAM_DAP_LEVELER_SETTING, mode);
            break;
        default:
            AM_LOGW("Invalid mode: %d", mode);
            return -1;
    }
    return 0;
}

int32_t DroidAudioManagerSetting::getSoundLevelerMode() {
    int32_t mode = getIntFromDb(DB_KEY_AM_AUDIO_CONFIG_SOUND_LEVELER_MODE);
    if (isAudioDebug()) AM_LOGD("mode: %d", mode);
    return mode;
}

int32_t DroidAudioManagerSetting::setSoundLevelerAmount(int32_t value) {
    const string HAL_PARAM_DAP_LEVELER_AMOUNT = "hal_param_dap_leveler_amount=";
    const int32_t LEVELER_AMOUNT_MIN = 0;
    const int32_t LEVELER_AMOUNT_MAX = 100;

    if (value < LEVELER_AMOUNT_MIN || value > LEVELER_AMOUNT_MAX) {
        AM_LOGW("Invalid value:%d, min:%d, max:%d", value, LEVELER_AMOUNT_MIN, LEVELER_AMOUNT_MAX);
        return -1;
    }

    if (isAudioDebug()) AM_LOGD("value: %d", value);
    putToDb(DB_KEY_AM_AUDIO_CONFIG_SOUND_LEVELER_AMOUNT, value);
    ::setParameters(HAL_PARAM_DAP_LEVELER_AMOUNT, value);
    return 0;
}

int32_t DroidAudioManagerSetting::getSoundLevelerAmount() {
    int32_t amount = getIntFromDb(DB_KEY_AM_AUDIO_CONFIG_SOUND_LEVELER_AMOUNT);
    if (isAudioDebug()) AM_LOGD("amount: %d", amount);
    return amount;
}

const static string AUDIO_VAD_STRING_VAD_ON                      = "on";
const static string AUDIO_VAD_STRING_VAD_OFF                     = "off";
const static string AUDIO_VAD_UBOOTENV_FFV_WAKE                  = "ubootenv.var.ffv_wake";
const static string AUDIO_VAD_PROPERTY_VADWAKE                   = "persist.vendor.vadwake";
int32_t DroidAudioManagerSetting::setVadEnabled(bool enable) {
    if (isAudioDebug()) AM_LOGD("enable: %d", enable);
    string mode = AUDIO_VAD_STRING_VAD_OFF;
    if (enable) {
        mode = AUDIO_VAD_STRING_VAD_ON;
    }
    g_SystemControlClient->setBootEnv(AUDIO_VAD_UBOOTENV_FFV_WAKE, mode);
    g_SystemControlClient->setProperty(AUDIO_VAD_PROPERTY_VADWAKE, mode);
    return 0;
}

bool DroidAudioManagerSetting::isVadEnabled() {
    string vadUbootEnable = "";
    string defaultStr = AUDIO_VAD_STRING_VAD_OFF;
    g_SystemControlClient->getBootEnv(AUDIO_VAD_UBOOTENV_FFV_WAKE, vadUbootEnable);
    string property = "";
    g_SystemControlClient->getPropertyString(AUDIO_VAD_PROPERTY_VADWAKE, property, defaultStr);
    if (isAudioDebug()) AM_LOGD("enable: %s", vadUbootEnable.c_str());
    return vadUbootEnable == AUDIO_VAD_STRING_VAD_ON;
}

int32_t DroidAudioManagerSetting::openTvAudio(int32_t source) {
    return DroidAudioPatchManager::instance().openTvAudio(source);
}

int32_t DroidAudioManagerSetting::getDroidAudioConfig(int32_t id) {
    switch (id) {
    case DroidAudioManager::DROID_AUDIO_CONFIG_ID_IS_DRIVER_BASE:
        return isDriverBaseProject() ? 1 : 0;
    case DroidAudioManager::DROID_AUDIO_CONFIG_ID_IS_SUPPORT_MS12:
        return isSupportMs12() ? 1 : 0;

    default:
        AM_LOGW("unsupported config id!");
        break;
    }
    return 0;
}

int32_t DroidAudioManagerSetting::setParameters(const string& keyValuePairs) {
    ::setParameters(keyValuePairs);
    return 0;
}

string DroidAudioManagerSetting::getParameters(const string& keys) {
    return ::getParameters(keys);
}

int32_t DroidAudioManagerSetting::createAudioPatch(int32_t sourceDevice, int32_t sinkDevice) {
    return DroidAudioPatchManager::instance().createAudioPatch(sourceDevice, sinkDevice);
}

int32_t DroidAudioManagerSetting::releaseAudioPatch(int32_t handle) {
    return DroidAudioPatchManager::instance().releaseAudioPatch(handle);
}

int32_t DroidAudioManagerSetting::setAiDeEnabled(bool enable) {
    const string HAL_PARAM_AIDE_ENABLE = "audio_enhancement_enable=";
    if (isAudioDebug()) AM_LOGD("enable: %d", enable);
    putToDb(DB_KEY_AM_AUDIO_CONFIG_AIDE_ENABLE, enable ? 1 : 0);
    ::setParameters(HAL_PARAM_AIDE_ENABLE, (enable ? 1 : 0));
    return 0;
}

bool DroidAudioManagerSetting::isAiDeEnabled() {
    bool enable = getIntFromDb(DB_KEY_AM_AUDIO_CONFIG_AIDE_ENABLE) != 0;
    if (isAudioDebug()) AM_LOGD("enable: %d", enable);
    return enable;
}

int32_t DroidAudioManagerSetting::setAiDeGain(int32_t gain) {
    const string HAL_PARAM_AIDE_GAIN = "audio_enhancement_gain=";
    const int32_t AI_DE_GAIN_LEVEL_MIN = 0;
    const int32_t AI_DE_GAIN_LEVEL_MAX = 3;

    if (gain < AI_DE_GAIN_LEVEL_MIN || gain > AI_DE_GAIN_LEVEL_MAX) {
        AM_LOGW("Invalid gain value:%d, min:%d, max:%d", gain, AI_DE_GAIN_LEVEL_MIN, AI_DE_GAIN_LEVEL_MAX);
        return -1;
    }

    if (isAudioDebug()) AM_LOGD("gain: %d", gain);
    putToDb(DB_KEY_AM_AUDIO_CONFIG_AIDE_GAIN, gain);
    ::setParameters(HAL_PARAM_AIDE_GAIN, gain);
    return 0;
}

int32_t DroidAudioManagerSetting::getAiDeGain() {
    int32_t gain = getIntFromDb(DB_KEY_AM_AUDIO_CONFIG_AIDE_GAIN);
    if (isAudioDebug()) AM_LOGD("gain: %d", gain);
    return gain;
}


int32_t DroidAudioManagerSetting::setGlobalMicEnable(int32_t source, bool enable) {
    if (isAudioDebug()) AM_LOGD("source:%d, enable: %d", source, enable);
    switch (source) {
        case DroidAudioManager::MIC_SOURCE_TYPE_BUILT_IN:
        case DroidAudioManager::MIC_SOURCE_TYPE_LINE_IN:
            ::setParameters("hal_param_karaoke_set=linein switch ", (enable ? 1 : 0));
            putToDb(DB_KEY_AM_AUDIO_CONFIG_SOUND_GLOBAL_LINEIN_MIC_ENABLE, enable ? 1 : 0);
            break;
        case DroidAudioManager::MIC_SOURCE_TYPE_USB_IN:
            ::setParameters("hal_param_karaoke_set=usb switch ", (enable ? 1 : 0));
            putToDb(DB_KEY_AM_AUDIO_CONFIG_GLOBAL_USB_MIC_ENABLE, enable ? 1 : 0);
            break;
        default:
            AM_LOGE("Invalid source:%d", source);
            return -1;
    }
    return 0;
}

bool DroidAudioManagerSetting::getGlobalMicStatus(int32_t source) {
    int32_t enable = false;
    switch (source) {
        case DroidAudioManager::MIC_SOURCE_TYPE_BUILT_IN:
        case DroidAudioManager::MIC_SOURCE_TYPE_LINE_IN:
            enable = getIntFromDb(DB_KEY_AM_AUDIO_CONFIG_SOUND_GLOBAL_LINEIN_MIC_ENABLE);
            break;
        case DroidAudioManager::MIC_SOURCE_TYPE_USB_IN:
            enable = getIntFromDb(DB_KEY_AM_AUDIO_CONFIG_GLOBAL_USB_MIC_ENABLE);
            break;
        default:
            AM_LOGE("Invalid source:%d", source);
            break;
    }
    if (isAudioDebug()) AM_LOGD("source:%d, enable: %d", source, enable);
    return enable != 0;
}

int32_t DroidAudioManagerSetting::setMicSource(int32_t source) {
    if (isAudioDebug()) AM_LOGD("source:%d", source);
    switch (source) {
        case DroidAudioManager::MIC_SOURCE_TYPE_BUILT_IN:
        case DroidAudioManager::MIC_SOURCE_TYPE_LINE_IN:
        case DroidAudioManager::MIC_SOURCE_TYPE_USB_IN:
            putToDb(DB_KEY_AM_AUDIO_CONFIG_SOUND_MIC_SOURCE, source);
            break;
        default:
            AM_LOGE("Invalid source:%d", source);
            return -1;
    }
    return 0;
}

int32_t DroidAudioManagerSetting::getMicSource() {
    int32_t source = getIntFromDb(DB_KEY_AM_AUDIO_CONFIG_SOUND_MIC_SOURCE);
    if (isAudioDebug()) AM_LOGD("source:%d", source);
    return source;
}

int32_t DroidAudioManagerSetting::setMicMute(int32_t source, bool mute) {
    if (isAudioDebug()) AM_LOGD("source:%d, mute: %d", source, mute);
    switch (source) {
        case DroidAudioManager::MIC_SOURCE_TYPE_BUILT_IN:
        case DroidAudioManager::MIC_SOURCE_TYPE_LINE_IN:
            ::setParameters("hal_param_karaoke_set=linein mic_mute ", (mute ? 1 : 0));
            putToDb(DB_KEY_AM_AUDIO_CONFIG_SOUND_LINEIN_MIC_MUTE, mute ? 1 : 0);
            break;
        case DroidAudioManager::MIC_SOURCE_TYPE_USB_IN:
            ::setParameters("hal_param_karaoke_set=usb mic_mute ", (mute ? 1 : 0));
            putToDb(DB_KEY_AM_AUDIO_CONFIG_USB_MIC_MUTE, mute ? 1 : 0);
            break;
        default:
            AM_LOGE("Invalid source:%d", source);
            return -1;
    }
    return 0;
}

bool DroidAudioManagerSetting::isMicMute(int32_t source) {
    int32_t mute = false;
    switch (source) {
        case DroidAudioManager::MIC_SOURCE_TYPE_BUILT_IN:
        case DroidAudioManager::MIC_SOURCE_TYPE_LINE_IN:
            mute = getIntFromDb(DB_KEY_AM_AUDIO_CONFIG_SOUND_LINEIN_MIC_MUTE);
            break;
        case DroidAudioManager::MIC_SOURCE_TYPE_USB_IN:
            mute = getIntFromDb(DB_KEY_AM_AUDIO_CONFIG_USB_MIC_MUTE);
            break;
        default:
            AM_LOGE("Invalid source:%d", source);
            break;
    }
    if (isAudioDebug()) AM_LOGD("source:%d, mute: %d", source, mute);
    return mute != 0;
}

/* mic gain rang:[0 100] db
   Value mapping rule: UI index - setting 0  mapping minum volume, such as -999999
    - UI            :    Setting
    - 0                  minum (-100)
    - Val [ 1 100]       Val - 10
*/
int32_t DroidAudioManagerSetting::setMicGain(int32_t source, int32_t gain) {
    if (isAudioDebug()) AM_LOGD("source:%d, gain: %d", source, gain);
    int32_t setValue = (gain == 0 ? -100 : gain - 10);
    switch (source) {
        case DroidAudioManager::MIC_SOURCE_TYPE_BUILT_IN:
        case DroidAudioManager::MIC_SOURCE_TYPE_LINE_IN:
            ::setParameters("hal_param_karaoke_set=linein mic_volume_gain ", setValue);
            putToDb(DB_KEY_AM_AUDIO_CONFIG_SOUND_LINEIN_MIC_GAIN, gain);
            break;
        case DroidAudioManager::MIC_SOURCE_TYPE_USB_IN:
            ::setParameters("hal_param_karaoke_set=usb mic_volume_gain ", setValue);
            putToDb(DB_KEY_AM_AUDIO_CONFIG_USB_MIC_GAIN, gain);
            break;
        default:
            AM_LOGE("Invalid source:%d", source);
            return -1;
    }
    return 0;
}

int32_t DroidAudioManagerSetting::getMicGain(int32_t source) {
    int32_t gain = 0;
    switch (source) {
        case DroidAudioManager::MIC_SOURCE_TYPE_BUILT_IN:
        case DroidAudioManager::MIC_SOURCE_TYPE_LINE_IN:
            gain = getIntFromDb(DB_KEY_AM_AUDIO_CONFIG_SOUND_LINEIN_MIC_GAIN);
            break;
        case DroidAudioManager::MIC_SOURCE_TYPE_USB_IN:
            gain = getIntFromDb(DB_KEY_AM_AUDIO_CONFIG_USB_MIC_GAIN);
            break;
        default:
            AM_LOGE("Invalid source:%d", source);
            break;
    }
    if (isAudioDebug()) AM_LOGD("source:%d, gain: %d", source, gain);
    return 0;
}

int32_t DroidAudioManagerSetting::setMicReverb(int32_t source, bool enable) {
    if (isAudioDebug()) AM_LOGD("source:%d, enable: %d", source, enable);
    int32_t value =  enable? 1 : 0;
    switch (source) {
        case DroidAudioManager::MIC_SOURCE_TYPE_BUILT_IN:
        case DroidAudioManager::MIC_SOURCE_TYPE_LINE_IN:
            ::setParameters("hal_param_karaoke_set=linein reverb_enable ", value);
            putToDb(DB_KEY_AM_AUDIO_CONFIG_SOUND_LINEIN_MIC_REVERB, value);
            break;
        case DroidAudioManager::MIC_SOURCE_TYPE_USB_IN:
            ::setParameters("hal_param_karaoke_set=usb reverb_enable ", value);
            putToDb(DB_KEY_AM_AUDIO_CONFIG_USB_MIC_REVERB, value);
            break;
        default:
            AM_LOGE("Invalid source:%d", source);
            return -1;
    }
    return 0;
}

bool DroidAudioManagerSetting::isEnableMicReverb(int32_t source) {
    int32_t enable = false;
    switch (source) {
        case DroidAudioManager::MIC_SOURCE_TYPE_BUILT_IN:
        case DroidAudioManager::MIC_SOURCE_TYPE_LINE_IN:
            enable = getIntFromDb(DB_KEY_AM_AUDIO_CONFIG_SOUND_LINEIN_MIC_REVERB);
            break;
        case DroidAudioManager::MIC_SOURCE_TYPE_USB_IN:
            enable = getIntFromDb(DB_KEY_AM_AUDIO_CONFIG_USB_MIC_REVERB);
            break;
        default:
            AM_LOGE("Invalid source:%d", source);
            break;
    }
    if (isAudioDebug()) AM_LOGD("source:%d, enable: %d", source, enable);
    return enable != 0;
}

//Reverb level rang int: [0, 5]
int32_t DroidAudioManagerSetting::setMicReverbLevel(int32_t source, int32_t level) {
    if (isAudioDebug()) AM_LOGD("source:%d, level: %d", source, level);
    switch (source) {
        case DroidAudioManager::MIC_SOURCE_TYPE_BUILT_IN:
        case DroidAudioManager::MIC_SOURCE_TYPE_LINE_IN:
            ::setParameters("hal_param_karaoke_set=linein reverb_mode ", level);
            putToDb(DB_KEY_AM_AUDIO_CONFIG_SOUND_LINEIN_MIC_REVERB_LEVEL, level);
            break;
        case DroidAudioManager::MIC_SOURCE_TYPE_USB_IN:
            ::setParameters("hal_param_karaoke_set=usb reverb_mode ", level);
            putToDb(DB_KEY_AM_AUDIO_CONFIG_USB_MIC_REVERB_LEVEL, level);
            break;
        default:
            AM_LOGE("Invalid source:%d", source);
        return -1;
    }
    return 0;
}

int32_t DroidAudioManagerSetting::getMicReverbLevel(int32_t source) {
    int32_t level = 0;
    switch (source) {
        case DroidAudioManager::MIC_SOURCE_TYPE_BUILT_IN:
        case DroidAudioManager::MIC_SOURCE_TYPE_LINE_IN:
            level = getIntFromDb(DB_KEY_AM_AUDIO_CONFIG_SOUND_LINEIN_MIC_REVERB_LEVEL);
            break;
        case DroidAudioManager::MIC_SOURCE_TYPE_USB_IN:
            level = getIntFromDb(DB_KEY_AM_AUDIO_CONFIG_USB_MIC_REVERB_LEVEL);
            break;
        default:
            AM_LOGE("Invalid source:%d", source);
            break;
    }
    if (isAudioDebug()) AM_LOGD("source:%d, level: %d", source, level);
    return level;
}

void DroidAudioManagerSetting::resetMicSettings() {
    int32_t halMicConfig = 0;
    string retStr = getParameters("Global_Mic_Device_Type_Config");
    if (retStr.length() != 0) {
        halMicConfig = stoi(retStr);
    }

    int32_t source = getMicSource();
    if (halMicConfig != 0) {
        setGlobalMicEnable(source, getGlobalMicStatus(source));
        setMicMute(source, isMicMute(source));
        setMicGain(source, getMicGain(source));
        setMicReverb(source, isEnableMicReverb(source));
        setMicReverbLevel(source, getMicReverbLevel(source));
    }
    AM_LOGI("mHalMicConfig: %d, source: %d", halMicConfig, source);
}