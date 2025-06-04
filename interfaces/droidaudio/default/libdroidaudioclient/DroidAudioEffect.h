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
#include <system/audio_effect.h>
#include <vector>

using namespace std;

class DroidAudioEffect {
public:
    enum {
        EFFECT_CONFIG_OFF                                                          = 0,
        EFFECT_CONFIG_ON                                                           = 1,
    };
    enum AUDIO_EFFECT_ID {
        /* Basic effects */
        EFFECT_ID_HPEQ                                                             = 0,
        EFFECT_ID_BALANCE                                                          = 1,
        EFFECT_ID_TREBLEBASS                                                       = 2,
        EFFECT_ID_VIRTUALSURROUND                                                  = 3,
        EFFECT_ID_DPE                                                              = 4,
        EFFECT_ID_AMLPEQ                                                           = 5,
        /* advanced effects */
        EFFECT_ID_DAP                                                              = 6,
        EFFECT_ID_VIRTUALX                                                         = 7,
        EFFECT_ID_MIN                                                              = EFFECT_ID_HPEQ,
        EFFECT_ID_MAX                                                              = EFFECT_ID_VIRTUALX,
    };
    static int32_t setAudioEffectEnabled(int32_t effectId, bool enable);
    static bool isAudioEffectEnabled(int32_t effectId);

    static int32_t setParameter(int32_t effectId, int32_t param, int32_t value);
    static int32_t setParameter(int32_t effectId, const vector<uint8_t>& param, const vector<uint8_t>& value);
    static int32_t setParameter(int32_t effectId, int32_t param, const vector<uint8_t>& value);
    static int32_t getParameter(int32_t effectId, const vector<uint8_t>& param, vector<uint8_t>* pValue);
    static int32_t getParameter(int32_t effectId, int32_t param);
    static int32_t getParameter(int32_t effectId, int32_t param, vector<uint8_t>* pValue);
    static int32_t setParameter(int32_t effectId, effect_param_t *param);
    static int32_t getParameter(int32_t effectId, effect_param_t *param);

    static int32_t setBasicEffectEnabled(bool enable);
    static bool isBasicEffectEnabled();
    static int32_t initDualEffectMode();

    enum {
        DUAL_EFFECT_MODE_AUTO                                                      = 0,
        DUAL_EFFECT_MODE_DTS                                                       = 1,
        DUAL_EFFECT_MODE_DOLBY                                                     = 2,
        DUAL_EFFECT_MODE_OFF                                                       = 3,
        DUAL_EFFECT_MODE_MIN                                                       = DUAL_EFFECT_MODE_AUTO,
        DUAL_EFFECT_MODE_MAX                                                       = DUAL_EFFECT_MODE_OFF,
    };
    static int32_t setDualEffectMode(int32_t mode);
    static int32_t getDualEffectMode();

    /*
    * Dolby ms12 audio config
    * -N: Not support MS12
    * -X: support MS12, config = x
    * -Y: support MS12, config = y
    * -X: support MS12, config = z
    */
    enum {
        // for EFFECT_CONFIG_DAP
        EFFECT_CONFIG_DAP_MS12_N                                                   = -1,
        EFFECT_CONFIG_DAP_MS12_X                                                   = 1,
        EFFECT_CONFIG_DAP_MS12_Y                                                   = 0,
        EFFECT_CONFIG_DAP_MS12_Z                                                   = 2,
    };

    enum {
        //karaoke source MIC config
        MIC_SOURCE_CONFIG_USB_IN                                                   = (1 << 0),
        MIC_SOURCE_CONFIG_LINE_IN                                                  = (1 << 1),
    };

    enum {
        EFFECT_CONFIG_HPEQ                                                         = 0,
        EFFECT_CONFIG_BALANCE                                                      = 1,
        EFFECT_CONFIG_TREBLEBASS                                                   = 2,
        EFFECT_CONFIG_VIRTUALSURROUND                                              = 3,
        EFFECT_CONFIG_DPE                                                          = 4,
        EFFECT_CONFIG_DAP                                                          = 5,
        EFFECT_CONFIG_VIRTUALX                                                     = 6,
        EFFECT_CONFIG_AMLPEQ                                                       = 7,
        EFFECT_CONFIG_DOLBY_DRC                                                    = 8,
        EFFECT_CONFIG_DTS_DRC                                                      = 9,
        EFFECT_CONFIG_ENGINEER_MODE                                                = 10,
        EFFECT_CONFIG_AUDIO_LATENCY                                                = 11,
        EFFECT_CONFIG_FORCE_DDP                                                    = 12,
        EFFECT_CONFIG_PASSTHROUGH                                                  = 13,
        EFFECT_CONFIG_AI_DE                                                        = 14,
        EFFECT_CONFIG_AI_AQ                                                        = 15,
        EFFECT_CONFIG_VOLUME_EQ                                                    = 16,
        EFFECT_CONFIG_OTT_MS12                                                     = 17,
        EFFECT_CONFIG_GLOBAL_MIC_DEVICE_TYPE_CONFIG                                = 18,
        EFFECT_CONFIG_MIN                                                          = EFFECT_CONFIG_HPEQ,
        EFFECT_CONFIG_MAX                                                          = EFFECT_CONFIG_GLOBAL_MIC_DEVICE_TYPE_CONFIG,
    };
    static int32_t getEffectFunctionConfig(int32_t id);

    enum {
        EFFECT_CONFIG_STEP_MIN                                                     = 0,
        EFFECT_CONFIG_STEP_MAX                                                     = 100,
    };
    static int32_t setBalance(int32_t step);
    static int32_t getBalance();
    static int32_t setTreble(int32_t step);
    static int32_t getTreble();
    static int32_t setBass(int32_t step);
    static int32_t getBass();

    // for DAP_CMD_2_4_PROFILE
    enum {
        DAP_2_4_PROFILE_MOVIE                                                      = 0,
        DAP_2_4_PROFILE_MUSIC                                                      = 1,
        DAP_2_4_PROFILE_GAME                                                       = 2,
        DAP_2_4_PROFILE_NIGHT                                                      = 3,
        DAP_2_4_PROFILE_VOICE                                                      = 4,
        DAP_2_4_PROFILE_USER_SELECTABLE                                            = 5,
        DAP_2_4_PROFILE_OFF                                                        = 6,
        DAP_2_4_PROFILE_MIN                                                        = DAP_2_4_PROFILE_MOVIE,
        DAP_2_4_PROFILE_MAX                                                        = DAP_2_4_PROFILE_OFF,
    };
    enum {
        // for DAP_CMD_2_4_SURROUND_VIRTUALIZER
        DAP_2_4_SURROUND_VIRTUALIZER_OFF                                           = 0,
        DAP_2_4_SURROUND_VIRTUALIZER_ON                                            = 1,
        DAP_2_4_SURROUND_VIRTUALIZER_AUTO                                          = 2,
    };
    enum {
        // for DAP_CMD_2_4_LEVELER
        DAP_2_4_LEVELER_OFF                                                        = 0,
        DAP_2_4_LEVELER_ON                                                         = 1,
        DAP_2_4_LEVELER_AUTO                                                       = 2,
    };
    enum {
        // for DAP_CMD_EFFECT_MODE
        DAP_EFFECT_MODE_OFF                                                        = 0,
        DAP_EFFECT_MODE_MOVIE                                                      = 1,
        DAP_EFFECT_MODE_MUSIC                                                      = 2,
        DAP_EFFECT_MODE_NIGHT                                                      = 3,
        DAP_EFFECT_MODE_USER                                                       = 4,
    };
    enum {
        // for DAP_SUBCMD_GEQ_BAND[x]
        DAP_GEQ_EFFECT_MODE_OFF                                                    = 0,
        DAP_GEQ_EFFECT_MODE_INIT                                                   = 1,
        DAP_GEQ_EFFECT_MODE_OPEN                                                   = 2,
        DAP_GEQ_EFFECT_MODE_RICH                                                   = 3,
        DAP_GEQ_EFFECT_MODE_FOCUSED                                                = 4,
        DAP_GEQ_EFFECT_MODE_USER                                                   = 5,
    };
    enum {
        /* [DAP 1.3.2] */
        DAP_CMD_1_3_2_BASE_VALUE                                                   = 0,
        DAP_CMD_ENABLE                                                             = 0,
        DAP_CMD_EFFECT_MODE                                                        = 1,
        DAP_CMD_GEQ_GAINS                                                          = 2,
        DAP_CMD_GEQ_ENABLE                                                         = 3,
        DAP_CMD_POST_GAIN                                                          = 4,
        DAP_CMD_VL_ENABLE                                                          = 5,
        DAP_CMD_VL_AMOUNT                                                          = 6,
        DAP_CMD_DE_ENABLE                                                          = 7,
        DAP_CMD_DE_AMOUNT                                                          = 8,
        DAP_CMD_SURROUND_ENABLE                                                    = 9,
        DAP_CMD_SURROUND_BOOST                                                     = 10,
        DAP_CMD_VIRTUALIZER_ENABLE                                                 = 11,
        DAP_CMD_2_4_ENABLE                                                         = 16,
        DAP_SUBCMD_GEQ_BAND1                                                       = 0x100,
        DAP_SUBCMD_GEQ_BAND2                                                       = 0x101,
        DAP_SUBCMD_GEQ_BAND3                                                       = 0x102,
        DAP_SUBCMD_GEQ_BAND4                                                       = 0x103,
        DAP_SUBCMD_GEQ_BAND5                                                       = 0x104,

        /* [DAP 2.4] */
        DAP_CMD_2_4_BASE_VALUE                                                     = 1000,
        DAP_CMD_2_4_PROFILE                                                        = 1000,
        DAP_CMD_2_4_SURROUND_VIRTUALIZER                                           = 1004,
        DAP_CMD_2_4_DIALOGUE_ENHANCER                                              = 1007,
        DAP_CMD_2_4_BASS_ENHANCER                                                  = 1008,
        DAP_CMD_2_4_MI_STEERING                                                    = 1012,
        DAP_CMD_2_4_SURROUND_DECODER_ENABLE                                        = 1013,
        DAP_CMD_2_4_LEVELER                                                        = 1015,

        DAP_SUBCMD_2_4_BASE_VALUE                                                  = 2000,
        DAP_SUBCMD_2_4_SURROUND_VIRTUALIZER_BOOST                                  = 2000,
        DAP_SUBCMD_2_4_DIALOGUE_ENHANCER_AMOUNT                                    = 2001,
        DAP_SUBCMD_2_4_BASS_ENHANCER_BOOST                                         = 2002,
        DAP_SUBCMD_2_4_BASS_ENHANCER_CUTOFF_100                                    = 2003,
        DAP_SUBCMD_2_4_BASS_ENHANCER_CUTOFF_1                                      = 2004,
        DAP_SUBCMD_2_4_BASS_ENHANCER_WIDTH                                         = 2005,
        DAP_SUBCMD_2_4_LEVELER_AMOUNT                                              = 2006,
    };
    static int32_t setDapParam(int32_t id, int32_t value);
    static int32_t getDapParam(int32_t id);

    enum {
        //dpe effect param define
        DPE_CMD_ENABLED                                                            = 48,
        DPE_CMD_INPUTGAIN                                                          = 32,
        DPE_CMD_PRE_EQ                                                             = 64,
        DPE_CMD_PRE_EQ_BAND                                                        = 69,
        DPE_CMD_MBC                                                                = 80,
        DPE_CMD_MBC_BAND                                                           = 85,
        DPE_CMD_POST_EQ                                                            = 96,
        DPE_CMD_POST_EQ_BAND                                                       = 101,
        DPE_CMD_LIMITER                                                            = 112,
        // dpe effect sub pre eq band 0 param define
        DPE_SUBCMD_PRE_EQ_BAND0                                                    = 100000,
        DPE_SUBCMD_PRE_EQ_BAND0_CUTOFFFREQUENCY                                    = 10000,
        DPE_SUBCMD_PRE_EQ_BAND0_GAIN                                               = 10001,
        // dpe effect sub pre eq band 1 param define
        DPE_SUBCMD_PRE_EQ_BAND1                                                    = 100100,
        DPE_SUBCMD_PRE_EQ_BAND1_CUTOFFFREQUENCY                                    = 10010,
        DPE_SUBCMD_PRE_EQ_BAND1_GAIN                                               = 10011,
        // dpe effect sub pre eq band 2 param define
        DPE_SUBCMD_PRE_EQ_BAND2                                                    = 100200,
        DPE_SUBCMD_PRE_EQ_BAND2_CUTOFFFREQUENCY                                    = 10020,
        DPE_SUBCMD_PRE_EQ_BAND2_GAIN                                               = 10021,
        // dpe effect sub mbc param band 0 define
        DPE_SUBCMD_MBC_BAND0                                                       = 110000,
        DPE_SUBCMD_MBC_BAND0_CUTOFFFREQUENCY                                       = 11000,
        DPE_SUBCMD_MBC_BAND0_ATTACKTIME                                            = 11001,
        DPE_SUBCMD_MBC_BAND0_RELEASETIME                                           = 11002,
        DPE_SUBCMD_MBC_BAND0_RATIO                                                 = 11003,
        DPE_SUBCMD_MBC_BAND0_THRESHOLD                                             = 11004,
        DPE_SUBCMD_MBC_BAND0_KNEEWIDTH                                             = 11005,
        DPE_SUBCMD_MBC_BAND0_NOISEGATETHRESHOLD                                    = 11006,
        DPE_SUBCMD_MBC_BAND0_EXPANDERRATIO                                         = 11007,
        DPE_SUBCMD_MBC_BAND0_PREGAIN                                               = 11008,
        DPE_SUBCMD_MBC_BAND0_POSTGAIN                                              = 11009,
        // dpe effect sub mbc param band 1 define
        DPE_SUBCMD_MBC_BAND1                                                       = 110100,
        DPE_SUBCMD_MBC_BAND1_CUTOFFFREQUENCY                                       = 11010,
        DPE_SUBCMD_MBC_BAND1_ATTACKTIME                                            = 11011,
        DPE_SUBCMD_MBC_BAND1_RELEASETIME                                           = 11012,
        DPE_SUBCMD_MBC_BAND1_RATIO                                                 = 11013,
        DPE_SUBCMD_MBC_BAND1_THRESHOLD                                             = 11014,
        DPE_SUBCMD_MBC_BAND1_KNEEWIDTH                                             = 11015,
        DPE_SUBCMD_MBC_BAND1_NOISEGATETHRESHOLD                                    = 11016,
        DPE_SUBCMD_MBC_BAND1_EXPANDERRATIO                                         = 11017,
        DPE_SUBCMD_MBC_BAND1_PREGAIN                                               = 11018,
        DPE_SUBCMD_MBC_BAND1_POSTGAIN                                              = 11019,
        // dpe effect sub mbc param band 2 define
        DPE_SUBCMD_MBC_BAND2                                                       = 110200,
        DPE_SUBCMD_MBC_BAND2_CUTOFFFREQUENCY                                       = 11020,
        DPE_SUBCMD_MBC_BAND2_ATTACKTIME                                            = 11021,
        DPE_SUBCMD_MBC_BAND2_RELEASETIME                                           = 11022,
        DPE_SUBCMD_MBC_BAND2_RATIO                                                 = 11023,
        DPE_SUBCMD_MBC_BAND2_THRESHOLD                                             = 11024,
        DPE_SUBCMD_MBC_BAND2_KNEEWIDTH                                             = 11025,
        DPE_SUBCMD_MBC_BAND2_NOISEGATETHRESHOLD                                    = 11026,
        DPE_SUBCMD_MBC_BAND2_EXPANDERRATIO                                         = 11027,
        DPE_SUBCMD_MBC_BAND2_PREGAIN                                               = 11028,
        DPE_SUBCMD_MBC_BAND2_POSTGAIN                                              = 11029,
        // dpe effect sub post eq band 0 param define
        DPE_SUBCMD_POST_EQ_BAND0                                                   = 120000,
        DPE_SUBCMD_POST_EQ_BAND0_CUTOFFFREQUENCY                                   = 1200 ,
        DPE_SUBCMD_POST_EQ_BAND0_GAIN                                              = 12001,
        // dpe effect sub post eq band 1 param define
        DPE_SUBCMD_POST_EQ_BAND1                                                   = 120100,
        DPE_SUBCMD_POST_EQ_BAND1_CUTOFFFREQUENCY                                   = 12010,
        DPE_SUBCMD_POST_EQ_BAND1_GAIN                                              = 12011,
        // dpe effect sub post eq band 2 param define
        DPE_SUBCMD_POST_EQ_BAND2                                                   = 120200,
        DPE_SUBCMD_POST_EQ_BAND2_CUTOFFFREQUENCY                                   = 12020,
        DPE_SUBCMD_POST_EQ_BAND2_GAIN                                              = 12021,
        //dpe limiter sub  param
        DPE_SUBCMD_LIMITER_ATTACKTIME                                              = 13001,
        DPE_SUBCMD_LIMITER_RELEASETIME                                             = 13002,
        DPE_SUBCMD_LIMITER_RATIO                                                   = 13003,
        DPE_SUBCMD_LIMITER_THRESHOLD                                               = 13004,
        DPE_SUBCMD_LIMITER_POSTGAIN                                                = 13005,
    };
    static int32_t setDpeParam(int32_t id, int32_t value);
    static int32_t getDpeParam(int32_t id);

    enum COMMON_SOUND_MODE_E {
        COMMON_SOUND_MODE_DYNAMIC                                                  = 0,
        COMMON_SOUND_MODE_STANDARD                                                 = 1,
        COMMON_SOUND_MODE_MUSIC                                                    = 2,
        COMMON_SOUND_MODE_NEWS                                                     = 3,
        COMMON_SOUND_MODE_MOVIE                                                    = 4,
        COMMON_SOUND_MODE_GAME                                                     = 5,
        COMMON_SOUND_MODE_NIGHT                                                    = 6,
        COMMON_SOUND_MODE_CUSTOM                                                   = 7,
        COMMON_SOUND_MODE_MIN                                                      = COMMON_SOUND_MODE_DYNAMIC,
        COMMON_SOUND_MODE_MAX                                                      = COMMON_SOUND_MODE_CUSTOM,
    };
    static int32_t setSoundMode(int32_t mode);
    static int32_t getSoundMode();

    enum {
        HPEQ_MODE_EFFECT_BAND1                                                     = 0,
        HPEQ_MODE_EFFECT_BAND2                                                     = 1,
        HPEQ_MODE_EFFECT_BAND3                                                     = 2,
        HPEQ_MODE_EFFECT_BAND4                                                     = 3,
        HPEQ_MODE_EFFECT_BAND5                                                     = 4,
        HPEQ_MODE_EFFECT_BAND6                                                     = 5,
        HPEQ_MODE_EFFECT_BAND7                                                     = 6,
        HPEQ_MODE_EFFECT_BAND8                                                     = 7,
        HPEQ_MODE_EFFECT_BAND9                                                     = 8,
        HPEQ_MODE_EFFECT_BAND_MIN                                                  = HPEQ_MODE_EFFECT_BAND1,
        HPEQ_MODE_EFFECT_BAND_MAX                                                  = HPEQ_MODE_EFFECT_BAND9,
    };
    static int32_t setUserSoundModeParam(int32_t bandNumber, int32_t value, int32_t bandSum);
    static int32_t getUserSoundModeParam(int32_t bandNumber);

    enum {
        HPEQ_BAND_NUM_5                                                            = 5,
        HPEQ_BAND_NUM_7                                                            = 7,
        HPEQ_BAND_NUM_9                                                            = 9,
    };
    static int32_t setHpeqBandNum(int32_t num);
    static int32_t getHpeqBandNum();

    static int32_t setVirtualSurroundEnabled(bool mode);
    static bool isVirtualSurroundEnabled();
    static bool isDtsVXValidEnabled();
    static int32_t setDtsVirtualXEnabled(bool enable);
    static bool isDtsVirtualXEnabled();
    static int32_t setDtsVirtualSurroundEnabled(bool enable);
    static bool isDtsVirtualSurroundEnabled();
    static int32_t setDtsBassEnhancementEnabled(bool enable);
    static bool isDtsBassEnhancementEnabled();

    enum {
        VIRTUALX_DIALOGCLARITY_MODE_OFF                                            = 0,
        VIRTUALX_DIALOGCLARITY_MODE_LOW                                            = 1,
        VIRTUALX_DIALOGCLARITY_MODE_MIDDLE                                         = 2,
        VIRTUALX_DIALOGCLARITY_MODE_HIGH                                           = 3,
        VIRTUALX_DIALOGCLARITY_MODE_MIN                                            = VIRTUALX_DIALOGCLARITY_MODE_OFF,
        VIRTUALX_DIALOGCLARITY_MODE_MAX                                            = VIRTUALX_DIALOGCLARITY_MODE_HIGH,
    };
    static int32_t setDtsDialogClarityMode(int32_t mode);
    static int32_t getDtsDialogClarityMode();

    enum VIRTUALX_MODE_E {
        VIRTUALX_MODE_OFF                                                          = 0,
        VIRTUALX_MODE_BASS                                                         = 1,
        VIRTUALX_MODE_FULL                                                         = 2,
        VIRTUALX_MODE_MIN                                                          = VIRTUALX_MODE_OFF,
        VIRTUALX_MODE_MAX                                                          = VIRTUALX_MODE_FULL,
    };
    static int32_t setDtsVirtualXMode(int32_t mode);
    static int32_t getDtsVirtualXMode();

    static int32_t setDtsTruVolumeHdEnabled(bool enable);
    static bool isDtsTruVolumeHdEnabled();

    static int32_t setAISoundModeEnable(bool enable);
    static bool isAISoundModeEnabled();

    static const char* soundMode2Str(int32_t type);
    static const char* dualEffectMode2Str(int32_t type);
};

vector<uint8_t> valueToByteArray(int32_t value);
vector<uint8_t> valueToByteArray(float value);
int32_t byteArrayToInt(vector<uint8_t> byteArray);
float byteArrayToFloat(const vector<uint8_t>& byteArray);
vector<uint8_t> extractSubvector(const vector<uint8_t>& input, uint32_t start, uint32_t end);
