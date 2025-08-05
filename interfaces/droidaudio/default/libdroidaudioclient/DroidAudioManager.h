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

#pragma once
#include <vector>

using namespace std;


#define PROP_AUDIO_OUTPUT_FORCEUSE                              "persist.vendor.media.audio.forceuse"
#define PROP_AUDIO_OUTPUT_STRATEGY                              "persist.vendor.media.audio.output.strategy"
#define PROP_AUDIO_OUTPUT_SPDIF_COEXIST                         "persist.vendor.media.audio.spdif.coexist"
/* 0: Auto  1: Semi-Auto  2: Manual (refer to: audio_output_strategy enum in Engine.cpp) */
enum DROID_AUDIO_OUTPUT_STRATEGY_TYPE_E {
    DROID_AUDIO_OUTPUT_STRATEGY_AUTO                            = 0,
    DROID_AUDIO_OUTPUT_STRATEGY_SEMI_AUTO                       = 1,
    DROID_AUDIO_OUTPUT_STRATEGY_MANUAL                          = 2,
};

class DroidAudioManager {
public:
    static int32_t reset();

    enum DROID_AUDIO_CMD_TYPE_E {
        DROID_AUDIO_CMD_START_DECODE                                = 1,
        DROID_AUDIO_CMD_PAUSE_DECODE                                = 2,
        DROID_AUDIO_CMD_RESUME_DECODE                               = 3,
        DROID_AUDIO_CMD_STOP_DECODE                                 = 4,
        DROID_AUDIO_CMD_SET_DECODE_AD                               = 5,
        DROID_AUDIO_CMD_SET_VOLUME                                  = 6,
        DROID_AUDIO_CMD_SET_MUTE                                    = 7,
        DROID_AUDIO_CMD_SET_OUTPUT_MODE                             = 8,
        DROID_AUDIO_CMD_SET_PRE_GAIN                                = 9,
        DROID_AUDIO_CMD_SET_PRE_MUTE                                = 10,
        DROID_AUDIO_CMD_OPEN_DECODER                                = 12,
        DROID_AUDIO_CMD_CLOSE_DECODER                               = 13,
        DROID_AUDIO_CMD_SET_DEMUX_INFO                              = 14,
        DROID_AUDIO_CMD_SET_SECURITY_MEM_LEVEL                      = 15,
        DROID_AUDIO_CMD_SET_HAS_VIDEO                               = 16,
        DROID_AUDIO_CMD_SET_MEDIA_SYCN_ID                           = 17,

        //audio ad
        DROID_AUDIO_CMD_AD_SWITCH_ENABLE                            = 18,
        DROID_AUDIO_CMD_AD_SET_VOLUME                               = 19,
        DROID_AUDIO_CMD_AD_DUAL_SUPPORT                             = 20,
        DROID_AUDIO_CMD_AD_MIX_SUPPORT                              = 21,
        DROID_AUDIO_CMD_AD_MIX_LEVEL                                = 22,
        DROID_AUDIO_CMD_AD_SET_MAIN                                 = 23,
        DROID_AUDIO_CMD_AD_SET_ASSOCIATE                            = 24,

        DROID_AUDIO_CMD_SET_MEDIA_PRESENTATION_ID                   = 25,
        DROID_AUDIO_CMD_SET_AUDIO_PATCH_MANAGE_MODE                 = 26,
        DROID_AUDIO_CMD_SET_SPDIF_PROTECTION_MODE                   = 27,
        DROID_AUDIO_CMD_SET_TSPLAYER_CLIENT_DIED                    = 28,

        DROID_AUDIO_CMD_SET_MEDIA_FIRST_LANG                        = 29,
        DROID_AUDIO_CMD_SET_MEDIA_SECOND_LANG                       = 30,
        DROID_AUDIO_CMD_SET_AUDIO_PICTURE_MODE                      = 31,
        DROID_AUDIO_CMD_SET_AUDIO_PLAYBACK_MODE                     = 32,
        DROID_AUDIO_CMD_SET_AUDIO_PATCH_ADDRESS                     = 33,
    };
    static int32_t setAudioCmdParam(int32_t cmd, int32_t param1, int32_t param2, int32_t param3);

    // audio_policy_forced_cfg_t (system\media\audio\include\system\audio_policy.h)
    enum DROID_AUDIO_FORCE_USE_TYPE_E {
        DROID_AUDIO_FORCE_USE_NONE                                      = 0, // AUDIO_POLICY_FORCE_NONE
        DROID_AUDIO_FORCE_USE_SPEAKER                                   = 1, // AUDIO_POLICY_FORCE_SPEAKER
        DROID_AUDIO_FORCE_USE_HEADPHONES                                = 2, // AUDIO_POLICY_FORCE_HEADPHONES
        DROID_AUDIO_FORCE_USE_BT_A2DP                                   = 4, // AUDIO_POLICY_FORCE_BT_A2DP
        DROID_AUDIO_FORCE_USE_USB                                       = 5, // AUDIO_POLICY_FORCE_WIRED_ACCESSORY
        DROID_AUDIO_FORCE_USE_HDMI                                      = 9, // AUDIO_POLICY_FORCE_DIGITAL_DOCK
        DROID_AUDIO_FORCE_USE_SPDIF                                     = 8, // AUDIO_POLICY_FORCE_ANALOG_DOCK
    };
    static int32_t setOutputDevices(const vector<int32_t>& devices);
    static int32_t getOutputDevices(vector<int32_t>* devices);
    static int32_t setCoexistSpdifOtherEnabled(bool enable);
    static int32_t isCoexistSpdifOtherEnabled();
    static int32_t setSoundBarModeEnabled(bool enable);
    static bool isSoundBarModeEnabled();
    static int32_t setSoundSpdifEnabled(bool enable);
    static bool isSoundSpdifEnabled();
    static int32_t setSpeakerEnabled(bool enable);
    static bool isSpeakerEnabled();

    enum AUDIO_OUTPUT_DELAY_SOURCE_E {
        AUDIO_OUTPUT_DELAY_SOURCE_ATV                                       = 0,
        AUDIO_OUTPUT_DELAY_SOURCE_DTV                                       = 1,
        AUDIO_OUTPUT_DELAY_SOURCE_AV                                        = 2,
        AUDIO_OUTPUT_DELAY_SOURCE_HDMI                                      = 3,
        AUDIO_OUTPUT_DELAY_SOURCE_MEDIA                                     = 4,
        AUDIO_OUTPUT_DELAY_SOURCE_MIN                                       = AUDIO_OUTPUT_DELAY_SOURCE_ATV,
        AUDIO_OUTPUT_DELAY_SOURCE_MAX                                       = AUDIO_OUTPUT_DELAY_SOURCE_MEDIA,
    };
    enum AUDIO_OUTPUT_DELAY_DEV_E {
        AUDIO_OUT_DELAY_DEV_HAL_SPEAKER                                     = 0,
        AUDIO_OUT_DELAY_DEV_HAL_SPDIF                                       = 1,
        AUDIO_OUT_DELAY_DEV_HAL_HEADPHONE                                   = 2,
        AUDIO_OUT_DELAY_DEV_HAL_ALL                                         = 3,
        AUDIO_OUT_DELAY_DEV_HAL_MIN                                         = AUDIO_OUT_DELAY_DEV_HAL_SPEAKER,
        AUDIO_OUT_DELAY_DEV_HAL_MAX                                         = AUDIO_OUT_DELAY_DEV_HAL_HEADPHONE,
    };
    enum {
        AUDIO_OUT_DELAY_HAL_MIN                                             = 0,       // ms
        AUDIO_OUT_DELAY_HAL_MAX                                             = 260,     // ms
    };
    static int32_t setOutputDeviceDelay(int32_t source, int32_t device, int32_t delayMs);
    static int32_t getOutputDeviceDelay(int32_t source, int32_t device);
    static int32_t setAudioOutputAllDelay(int32_t delayMs);
    static int32_t getAudioOutputAllDelay();
    static int32_t setTvSourceType(int32_t source);
    static int32_t getTvSourceType();
    static int32_t setAudioApplyToAll();

    // keep these values in sync with AudioFormat.java
    enum DIGITAL_AUDIO_MODE_ENCODING_E {
        ENCODING_PCM_16BIT                                              = 2,
        ENCODING_PCM_8BIT                                               = 3,
        ENCODING_PCM_FLOAT                                              = 4,
        ENCODING_AC3                                                    = 5,
        ENCODING_E_AC3                                                  = 6,
        ENCODING_DTS                                                    = 7,
        ENCODING_DTS_HD                                                 = 8,
        ENCODING_MP3                                                    = 9,
        ENCODING_AAC_LC                                                 = 10,
        ENCODING_AAC_HE_V1                                              = 11,
        ENCODING_AAC_HE_V2                                              = 12,
        ENCODING_IEC61937                                               = 13,
        ENCODING_DOLBY_TRUEHD                                           = 14,
        ENCODING_AAC_ELD                                                = 15,
        ENCODING_AAC_XHE                                                = 16,
        ENCODING_AC4                                                    = 17,
        ENCODING_E_AC3_JOC                                              = 18,
        ENCODING_DOLBY_MAT                                              = 19,
        ENCODING_OPUS                                                   = 20,
        ENCODING_PCM_24BIT_PACKED                                       = 21,
        ENCODING_PCM_32BIT                                              = 22,
        ENCODING_MPEGH_BL_L3                                            = 23,
        ENCODING_MPEGH_BL_L4                                            = 24,
        ENCODING_MPEGH_LC_L3                                            = 25,
        ENCODING_MPEGH_LC_L4                                            = 26,
        ENCODING_DTS_UHD                                                = 27,
        ENCODING_DRA                                                    = 28,
        ENCODING_DTS_HD_MA                                              = 29,
        ENCODING_DTS_UHD_P2                                             = 30,
        ENCODING_DSD                                                    = 31,
    };
    inline static const std::vector<int32_t> SURROUND_SOUND_ALWAYS_FORMATS = {
        ENCODING_AC3, ENCODING_E_AC3, ENCODING_DOLBY_TRUEHD,
        ENCODING_E_AC3_JOC, ENCODING_DOLBY_MAT,
        ENCODING_DTS, ENCODING_DTS_HD, ENCODING_DTS_UHD_P2, ENCODING_DRA
    };
    enum DIGITAL_AUDIO_MODE_E {
        DIGITAL_AUDIO_MODE_PCM                                          = 0,
        DIGITAL_AUDIO_MODE_AUTO                                         = 1,
        DIGITAL_AUDIO_MODE_MANUAL                                       = 2,
        DIGITAL_AUDIO_MODE_PASSTHROUGH                                  = 3,
        DIGITAL_AUDIO_MODE_ALWAYS                                       = 4,
        DIGITAL_AUDIO_MODE_MIN                                          = DIGITAL_AUDIO_MODE_PCM,
        DIGITAL_AUDIO_MODE_MAX                                          = DIGITAL_AUDIO_MODE_ALWAYS,
    };
    static int32_t setDigitalAudioMode(int32_t mode, const string& formats);
    static int32_t getDigitalAudioMode();
    static int32_t setForceDDPEnabled(bool enable);
    static bool isForceDDPEnabled();

    enum DOLBY_DRC_MODE_E {
        DOLBY_DRC_MODE_OFF                                                            = 0,
        DOLBY_DRC_MODE_LINE                                                           = 1,
        DOLBY_DRC_MODE_RF                                                             = 2,
        DOLBY_DRC_MODE_MIN                                                            = DOLBY_DRC_MODE_OFF,
        DOLBY_DRC_MODE_MAX                                                            = DOLBY_DRC_MODE_RF,
    };
    static int32_t setDolbyDrcMode(int32_t mode);
    static int32_t getDolbyDrcMode();
    static int32_t setDolbyDrcLineLevel(int32_t level);
    static int32_t getDolbyDrcLineLevel();
    static bool isDtsXEnabled();
    static int32_t setDtsXDrcEnabled(bool enable);
    static bool isDtsXDrcEnabled();

    enum DIALOGUE_ENHANCEMENT_LEVEL_E {
        DIALOGUE_ENHANCEMENT_LEVEL_OFF                                                  = 0,
        DIALOGUE_ENHANCEMENT_LEVEL_LOW                                                  = 1,
        DIALOGUE_ENHANCEMENT_LEVEL_MEDIUM                                               = 2,
        DIALOGUE_ENHANCEMENT_LEVEL_HIGH                                                 = 3,
        DIALOGUE_ENHANCEMENT_LEVEL_MIN                                                  = DIALOGUE_ENHANCEMENT_LEVEL_OFF,
        DIALOGUE_ENHANCEMENT_LEVEL_MAX                                                  = DIALOGUE_ENHANCEMENT_LEVEL_HIGH,
    };
    static int32_t setDialogEnhancerLevel(int32_t level);
    static int32_t getDialogEnhancerLevel();

    enum DOLBY_SOUND_DMX_MODE_E {
        DOLBY_SOUND_DMX_MODE_SURROUND                                                   = 0,
        DOLBY_SOUND_DMX_MODE_STEREO                                                     = 1,
        DOLBY_SOUND_DMX_MODE_MIN                                                        = DOLBY_SOUND_DMX_MODE_SURROUND,
        DOLBY_SOUND_DMX_MODE_MAX                                                        = DOLBY_SOUND_DMX_MODE_STEREO,
    };
    static int32_t setSoundDmxMode(int32_t mode);
    static int32_t getSoundDmxMode();
    static int32_t setVadEnabled(bool enable);
    static bool isVadEnabled();

    enum DOLBY_SOUND_LEVELER_MODE_E {
        DOLBY_SOUND_LEVELER_MODE_OFF                                                    = 0,
        DOLBY_SOUND_LEVELER_MODE_ON                                                     = 1,
        DOLBY_SOUND_LEVELER_MODE_AUTO                                                   = 2,
    };
    static int32_t setSoundLevelerMode(int mode);
    static int32_t getSoundLevelerMode();
    static int32_t setSoundLevelerAmount(int value);
    static int32_t getSoundLevelerAmount();

    //  Same as the contents of the DroidLogicTvUtils.java
    enum OPEN_TV_SOURCE_E {
        SOURCE_TYPE_ATV                                                                  = 0,
        SOURCE_TYPE_DTV                                                                  = 1,
    };
    static int32_t openTvAudio(int32_t source);

    enum DROID_AUDIO_CONFIG_ID_E{
        DROID_AUDIO_CONFIG_ID_IS_DRIVER_BASE                                             = 0,
        DROID_AUDIO_CONFIG_ID_IS_SUPPORT_MS12                                            = 1,
    };
    static int32_t getDroidAudioConfig(int32_t id);

    static int32_t setParameters(const string& keyValuePairs);
    static string getParameters(const string& keys);
    /* Create an audio patch between several source and sink ports */
    static int32_t createAudioPatch(int32_t sourceDevice, int32_t sinkDevice);
    /* Release an audio patch */
    static int32_t releaseAudioPatch(int32_t handle);

    //AI DE Control
    static int32_t setAiDeEnabled(bool enable);
    static bool isAiDeEnabled();
    static int32_t setAiDeGain(int32_t value);
    static int32_t getAiDeGain();

    enum MIC_SOURCE_TYPE_E {
        MIC_SOURCE_TYPE_BUILT_IN                                            = 0,  //built in mic
        MIC_SOURCE_TYPE_LINE_IN                                             = 1,  //line in mic
        MIC_SOURCE_TYPE_USB_IN                                              = 2,  //line in mic
    };
    static int32_t setGlobalMicEnable(int32_t source, bool enable);
    static bool getGlobalMicStatus(int32_t source);
    static int32_t setMicSource(int32_t source);
    static int32_t getMicSource();
    static int32_t setMicMute(int32_t source, bool mute);
    static bool isMicMute(int32_t source);

    /* mic gain rang:[0 100] db
       Value mapping rule: UI index - setting 0  mapping minum volume, such as -999999
        - UI            :    Setting
        - 0                  minum (-100)
        - Val [ 1 100]       Val - 10
    */
    static int32_t setMicGain(int32_t source, int32_t gain);
    static int32_t getMicGain(int32_t source);
    static int32_t setMicReverb(int32_t source, bool enable);
    static bool isEnableMicReverb(int32_t source);

    //Reverb level rang int: [0, 5]
    static int32_t setMicReverbLevel(int32_t source, int32_t level);
    static int32_t getMicReverbLevel(int32_t source);

    static const char* audioCmd2Str(int32_t type);
    static const char* audioDigitalMode2Str(int32_t type);
    static const char* audioDelayDev2Str(int32_t type);
    static const char* tvSource2Str(int32_t source);

    static int32_t setMpeghActionEvent(string eventXml);
    static int32_t setMpeghSystemConfig(int id, string value);
    static string getMpeghSystemConfig(int id);
    static string getMpeghSceneXml();
};

