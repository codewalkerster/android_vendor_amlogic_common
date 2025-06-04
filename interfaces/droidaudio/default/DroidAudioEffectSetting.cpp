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

#define LOG_TAG "DroidAudioEffectSetting"
//#define LOG_NDEBUG 0

#include <system/audio-base.h>
#include <cutils/properties.h>
#include "unistd.h"
#include <log/log.h>

#include <media/AidlConversion.h>
#include <media/AudioSystem.h>
#include <media/AudioParameter.h>
#include <media/AudioEffect.h>
#include <media/AudioTrack.h>


#include "DroidAudioEffect.h"
#include "DroidAudioCommon.h"
#include "DroidAudioClientUtils.h"
#include "DroidAudioDb.h"
#include "SystemControlClient.h"
#include "DroidAudioManagerSetting.h"
#include "DroidAudioEffectSetting.h"

using namespace std;
using namespace android;

static const char* DB_KEY_AM_AUDIO_EFFECT_HPEQ_SOUND_MODE                            = "db_key_am_audio_effect_hpeq_sound_mode";
static const char* DB_KEY_AM_AUDIO_EFFECT_HPEQ_BAND1                                 = "db_key_am_audio_effect_hpeq_band1";
static const char* DB_KEY_AM_AUDIO_EFFECT_HPEQ_BAND2                                 = "db_key_am_audio_effect_hpeq_band2";
static const char* DB_KEY_AM_AUDIO_EFFECT_HPEQ_BAND3                                 = "db_key_am_audio_effect_hpeq_band3";
static const char* DB_KEY_AM_AUDIO_EFFECT_HPEQ_BAND4                                 = "db_key_am_audio_effect_hpeq_band4";
static const char* DB_KEY_AM_AUDIO_EFFECT_HPEQ_BAND5                                 = "db_key_am_audio_effect_hpeq_band5";
static const char* DB_KEY_AM_AUDIO_EFFECT_HPEQ_BAND6                                 = "db_key_am_audio_effect_hpeq_band6";
static const char* DB_KEY_AM_AUDIO_EFFECT_HPEQ_BAND7                                 = "db_key_am_audio_effect_hpeq_band7";
static const char* DB_KEY_AM_AUDIO_EFFECT_HPEQ_BAND8                                 = "db_key_am_audio_effect_hpeq_band8";
static const char* DB_KEY_AM_AUDIO_EFFECT_HPEQ_BAND9                                 = "db_key_am_audio_effect_hpeq_band9";

static const char* DB_KEY_AM_AUDIO_EFFECT_BALANCE                                    = "db_key_am_audio_effect_balance";

static const char* DB_KEY_AM_AUDIO_EFFECT_TREBLEBASS_BASS                            = "db_key_am_audio_effect_treblebass_bass";
static const char* DB_KEY_AM_AUDIO_EFFECT_TREBLEBASS_TREBLE                          = "db_key_am_audio_effect_treblebass_treble";

static const char* DB_KEY_AM_AUDIO_EFFECT_VIRTUALSURROUND_ENABLE                     = "db_key_am_audio_effect_virtualsurround_enable";

static const char* DB_KEY_AM_AUDIO_EFFECT_DPE_ENABLED                                = "db_key_am_audio_effect_dpe_enabled";
static const char* DB_KEY_AM_AUDIO_EFFECT_DPE_INPUTGAIN                              = "db_key_am_audio_effect_dpe_inputgain";
static const char* DB_KEY_AM_AUDIO_EFFECT_DPE_PRE_EQ                                 = "db_key_am_audio_effect_dpe_pre_eq";
static const char* DB_KEY_AM_AUDIO_EFFECT_DPE_PRE_EQ_BAND0                           = "db_key_am_audio_effect_dpe_pre_eq_band0";
static const char* DB_KEY_AM_AUDIO_EFFECT_DPE_PRE_EQ_BAND0_CUTOFFFREQUENCY           = "db_key_am_audio_effect_dpe_pre_eq_band0_cutofffrequency";
static const char* DB_KEY_AM_AUDIO_EFFECT_DPE_PRE_EQ_BAND0_GAIN                      = "db_key_am_audio_effect_dpe_pre_eq_band0_gain";
static const char* DB_KEY_AM_AUDIO_EFFECT_DPE_PRE_EQ_BAND1                           = "db_key_am_audio_effect_dpe_pre_eq_band1";
static const char* DB_KEY_AM_AUDIO_EFFECT_DPE_PRE_EQ_BAND1_CUTOFFFREQUENCY           = "db_key_am_audio_effect_dpe_pre_eq_band1_cutofffrequency";
static const char* DB_KEY_AM_AUDIO_EFFECT_DPE_PRE_EQ_BAND1_GAIN                      = "db_key_am_audio_effect_dpe_pre_eq_band1_gain";
static const char* DB_KEY_AM_AUDIO_EFFECT_DPE_PRE_EQ_BAND2                           = "db_key_am_audio_effect_dpe_pre_eq_band2";
static const char* DB_KEY_AM_AUDIO_EFFECT_DPE_PRE_EQ_BAND2_CUTOFFFREQUENCY           = "db_key_am_audio_effect_dpe_pre_eq_band2_cutofffrequency";
static const char* DB_KEY_AM_AUDIO_EFFECT_DPE_PRE_EQ_BAND2_GAIN                      = "db_key_am_audio_effect_dpe_pre_eq_band2_gain";
static const char* DB_KEY_AM_AUDIO_EFFECT_DPE_MBC                                    = "db_key_am_audio_effect_dpe_mbc";
static const char* DB_KEY_AM_AUDIO_EFFECT_DPE_MBC_BAND0                              = "db_key_am_audio_effect_dpe_mbc_band0";
static const char* DB_KEY_AM_AUDIO_EFFECT_DPE_MBC_BAND0_CUTOFFFREQUENCY              = "db_key_am_audio_effect_dpe_mbc_band0_cutoffFrequency";
static const char* DB_KEY_AM_AUDIO_EFFECT_DPE_MBC_BAND0_ATTACKTIME                   = "db_key_am_audio_effect_dpe_mbc_band0_attacktime";
static const char* DB_KEY_AM_AUDIO_EFFECT_DPE_MBC_BAND0_RELEASETIME                  = "db_key_am_audio_effect_dpe_mbc_band0_releasetime";
static const char* DB_KEY_AM_AUDIO_EFFECT_DPE_MBC_BAND0_RATIO                        = "db_key_am_audio_effect_dpe_mbc_band0_ratio";
static const char* DB_KEY_AM_AUDIO_EFFECT_DPE_MBC_BAND0_THRESHOLD                    = "db_key_am_audio_effect_dpe_mbc_band0_threshold";
static const char* DB_KEY_AM_AUDIO_EFFECT_DPE_MBC_BAND0_KNEEWIDTH                    = "db_key_am_audio_effect_dpe_mbc_band0_kneewidth";
static const char* DB_KEY_AM_AUDIO_EFFECT_DPE_MBC_BAND0_NOISEGATE_THRESHOLD          = "db_key_am_audio_effect_dpe_mbc_band0_noisegate_threshold";
static const char* DB_KEY_AM_AUDIO_EFFECT_DPE_MBC_BAND0_EXPANDER_RATIO               = "db_key_am_audio_effect_dpe_mbc_band0_expander_ratio";
static const char* DB_KEY_AM_AUDIO_EFFECT_DPE_MBC_BAND0_PRE_GAIN                     = "db_key_am_audio_effect_dpe_mbc_band0_pre_gain";
static const char* DB_KEY_AM_AUDIO_EFFECT_DPE_MBC_BAND0_POST_GAIN                    = "db_key_am_audio_effect_dpe_mbc_band0_post_gain";
static const char* DB_KEY_AM_AUDIO_EFFECT_DPE_MBC_BAND1                              = "db_key_am_audio_effect_dpe_mbc_band1";
static const char* DB_KEY_AM_AUDIO_EFFECT_DPE_MBC_BAND1_CUTOFFFREQUENCY              = "db_key_am_audio_effect_dpe_mbc_band1_cutoffFrequency";
static const char* DB_KEY_AM_AUDIO_EFFECT_DPE_MBC_BAND1_ATTACKTIME                   = "db_key_am_audio_effect_dpe_mbc_band1_attacktime";
static const char* DB_KEY_AM_AUDIO_EFFECT_DPE_MBC_BAND1_RELEASETIME                  = "db_key_am_audio_effect_dpe_mbc_band1_releasetime";
static const char* DB_KEY_AM_AUDIO_EFFECT_DPE_MBC_BAND1_RATIO                        = "db_key_am_audio_effect_dpe_mbc_band1_ratio";
static const char* DB_KEY_AM_AUDIO_EFFECT_DPE_MBC_BAND1_THRESHOLD                    = "db_key_am_audio_effect_dpe_mbc_band1_threshold";
static const char* DB_KEY_AM_AUDIO_EFFECT_DPE_MBC_BAND1_KNEEWIDTH                    = "db_key_am_audio_effect_dpe_mbc_band1_kneewidth";
static const char* DB_KEY_AM_AUDIO_EFFECT_DPE_MBC_BAND1_NOISEGATE_THRESHOLD          = "db_key_am_audio_effect_dpe_mbc_band1_noisegate_threshold";
static const char* DB_KEY_AM_AUDIO_EFFECT_DPE_MBC_BAND1_EXPANDER_RATIO               = "db_key_am_audio_effect_dpe_mbc_band1_expander_ratio";
static const char* DB_KEY_AM_AUDIO_EFFECT_DPE_MBC_BAND1_PRE_GAIN                     = "db_key_am_audio_effect_dpe_mbc_band1_pre_gain";
static const char* DB_KEY_AM_AUDIO_EFFECT_DPE_MBC_BAND1_POST_GAIN                    = "db_key_am_audio_effect_dpe_mbc_band1_post_gain";
static const char* DB_KEY_AM_AUDIO_EFFECT_DPE_MBC_BAND2                              = "db_key_am_audio_effect_dpe_mbc_band2";
static const char* DB_KEY_AM_AUDIO_EFFECT_DPE_MBC_BAND2_CUTOFFFREQUENCY              = "db_key_am_audio_effect_dpe_mbc_band2_cutoffFrequency";
static const char* DB_KEY_AM_AUDIO_EFFECT_DPE_MBC_BAND2_ATTACKTIME                   = "db_key_am_audio_effect_dpe_mbc_band2_attacktime";
static const char* DB_KEY_AM_AUDIO_EFFECT_DPE_MBC_BAND2_RELEASETIME                  = "db_key_am_audio_effect_dpe_mbc_band2_releasetime";
static const char* DB_KEY_AM_AUDIO_EFFECT_DPE_MBC_BAND2_RATIO                        = "db_key_am_audio_effect_dpe_mbc_band2_ratio";
static const char* DB_KEY_AM_AUDIO_EFFECT_DPE_MBC_BAND2_THRESHOLD                    = "db_key_am_audio_effect_dpe_mbc_band2_threshold";
static const char* DB_KEY_AM_AUDIO_EFFECT_DPE_MBC_BAND2_KNEEWIDTH                    = "db_key_am_audio_effect_dpe_mbc_band2_kneewidth";
static const char* DB_KEY_AM_AUDIO_EFFECT_DPE_MBC_BAND2_NOISEGATE_THRESHOLD          = "db_key_am_audio_effect_dpe_mbc_band2_noisegate_threshold";
static const char* DB_KEY_AM_AUDIO_EFFECT_DPE_MBC_BAND2_EXPANDER_RATIO               = "db_key_am_audio_effect_dpe_mbc_band2_expander_ratio";
static const char* DB_KEY_AM_AUDIO_EFFECT_DPE_MBC_BAND2_PRE_GAIN                     = "db_key_am_audio_effect_dpe_mbc_band2_pre_gain";
static const char* DB_KEY_AM_AUDIO_EFFECT_DPE_MBC_BAND2_POST_GAIN                    = "db_key_am_audio_effect_dpe_mbc_band2_post_gain";
static const char* DB_KEY_AM_AUDIO_EFFECT_DPE_POST_EQ                                = "db_key_am_audio_effect_dpe_post_eq";
static const char* DB_KEY_AM_AUDIO_EFFECT_DPE_POST_EQ_BAND0                          = "db_key_am_audio_effect_dpe_post_eq_band0";
static const char* DB_KEY_AM_AUDIO_EFFECT_DPE_POST_EQ_BAND0_CUTOFFFREQUENCY          = "db_key_am_audio_effect_dpe_post_eq_band0_cutofffrequency";
static const char* DB_KEY_AM_AUDIO_EFFECT_DPE_POST_EQ_BAND0_GAIN                     = "db_key_am_audio_effect_dpe_post_eq_band0_gain";
static const char* DB_KEY_AM_AUDIO_EFFECT_DPE_POST_EQ_BAND1                          = "db_key_am_audio_effect_dpe_post_eq_band1";
static const char* DB_KEY_AM_AUDIO_EFFECT_DPE_POST_EQ_BAND1_CUTOFFFREQUENCY          = "db_key_am_audio_effect_dpe_post_eq_band1_cutofffrequency";
static const char* DB_KEY_AM_AUDIO_EFFECT_DPE_POST_EQ_BAND1_GAIN                     = "db_key_am_audio_effect_dpe_post_eq_band1_gain";
static const char* DB_KEY_AM_AUDIO_EFFECT_DPE_POST_EQ_BAND2                          = "db_key_am_audio_effect_dpe_post_eq_band2";
static const char* DB_KEY_AM_AUDIO_EFFECT_DPE_POST_EQ_BAND2_CUTOFFFREQUENCY          = "db_key_am_audio_effect_dpe_post_eq_band2_cutofffrequency";
static const char* DB_KEY_AM_AUDIO_EFFECT_DPE_POST_EQ_BAND2_GAIN                     = "db_key_am_audio_effect_dpe_post_eq_band2_gain";
static const char* DB_KEY_AM_AUDIO_EFFECT_DPE_LIMITER                                = "db_key_am_audio_effect_dpe_limiter";
static const char* DB_KEY_AM_AUDIO_EFFECT_DPE_LIMITER_ATTACKTIME                     = "db_key_am_audio_effect_dpe_limiter_attacktime";
static const char* DB_KEY_AM_AUDIO_EFFECT_DPE_LIMITER_RELEASETIMR                    = "db_key_am_audio_effect_dpe_limiter_releasetime";
static const char* DB_KEY_AM_AUDIO_EFFECT_DPE_LIMITER_RATIO                          = "db_key_am_audio_effect_dpe_limiter_ratio";
static const char* DB_KEY_AM_AUDIO_EFFECT_DPE_LIMITER_THRESHOLD                      = "db_key_am_audio_effect_dpe_limiter_threshold";
static const char* DB_KEY_AM_AUDIO_EFFECT_DPE_LIMITER_POST_GAIN                      = "db_key_am_audio_effect_dpe_limiter_post_gain";

static const char* DB_KEY_AM_AUDIO_EFFECT_DAP_MODE                                   = "db_key_am_audio_effect_dap_mode";
static const char* DB_KEY_AM_AUDIO_EFFECT_DAP_GEQ_ENABLE                             = "db_key_am_audio_effect_dap_geq";
static const char* DB_KEY_AM_AUDIO_EFFECT_DAP_POST_GAIN                              = "db_key_am_audio_effect_dap_post_gain";
static const char* DB_KEY_AM_AUDIO_EFFECT_DAP_VL_ENABLE                              = "db_key_am_audio_effect_dap_vl";
static const char* DB_KEY_AM_AUDIO_EFFECT_DAP_VL_AMOUNT                              = "db_key_am_audio_effect_dap_vl_amount";
static const char* DB_KEY_AM_AUDIO_EFFECT_DAP_DE_ENABLE                              = "db_key_am_audio_effect_dap_de";
static const char* DB_KEY_AM_AUDIO_EFFECT_DAP_DE_AMOUNT                              = "db_key_am_audio_effect_dap_de_amount";
static const char* DB_KEY_AM_AUDIO_EFFECT_DAP_SURROUND_ENABLE                        = "db_key_am_audio_effect_dap_surround";
static const char* DB_KEY_AM_AUDIO_EFFECT_DAP_SURROUND_BOOST                         = "db_key_am_audio_effect_dap_surround_boost";
static const char* DB_KEY_AM_AUDIO_EFFECT_DAP_GEQ_BAND1                              = "db_key_am_audio_effect_dap_geq_band1";
static const char* DB_KEY_AM_AUDIO_EFFECT_DAP_GEQ_BAND2                              = "db_key_am_audio_effect_dap_geq_band2";
static const char* DB_KEY_AM_AUDIO_EFFECT_DAP_GEQ_BAND3                              = "db_key_am_audio_effect_dap_geq_band3";
static const char* DB_KEY_AM_AUDIO_EFFECT_DAP_GEQ_BAND4                              = "db_key_am_audio_effect_dap_geq_band4";
static const char* DB_KEY_AM_AUDIO_EFFECT_DAP_GEQ_BAND5                              = "db_key_am_audio_effect_dap_geq_band5";
static const char* DB_KEY_AM_AUDIO_EFFECT_DAP_2_4_PROFILE                            = "db_key_am_audio_effect_dap_2_4_profile";
static const char* DB_KEY_AM_AUDIO_EFFECT_DAP_2_4_SURROUND_VIRTUALIZER_MODE          = "db_key_am_audio_effect_dap_2_4_surround_virtualizer_mode";
static const char* DB_KEY_AM_AUDIO_EFFECT_DAP_2_4_SURROUND_VIRTUALIZER_BOOST         = "db_key_am_audio_effect_dap_2_4_surround_virtualizer_boost";
static const char* DB_KEY_AM_AUDIO_EFFECT_DAP_2_4_DIALOGUE_ENHANCER_ENABLE           = "db_key_am_audio_effect_dap_2_4_dialogue_enhancer_enable";
static const char* DB_KEY_AM_AUDIO_EFFECT_DAP_2_4_BASS_ENHANCER_ENABLE               = "db_key_am_audio_effect_dap_2_4_bass_enhancer_enable";
static const char* DB_KEY_AM_AUDIO_EFFECT_DAP_2_4_BASS_ENHANCER_BOOST                = "db_key_am_audio_effect_dap_2_4_bass_enhancer_boost";
static const char* DB_KEY_AM_AUDIO_EFFECT_DAP_2_4_BASS_ENHANCER_CUTOFFX100           = "db_key_am_audio_effect_dap_2_4_bass_enhancer_cutoffX100";
static const char* DB_KEY_AM_AUDIO_EFFECT_DAP_2_4_BASS_ENHANCER_CUTOFFX1             = "db_key_am_audio_effect_dap_2_4_bass_enhancer_cutoffX1";
static const char* DB_KEY_AM_AUDIO_EFFECT_DAP_2_4_BASS_ENHANCER_WIDTH                = "db_key_am_audio_effect_dap_2_4_bass_enhancer_width";
static const char* DB_KEY_AM_AUDIO_EFFECT_DAP_2_4_MI_STEERING                        = "db_key_am_audio_effect_dap_2_4_mi_steering";
static const char* DB_KEY_AM_AUDIO_EFFECT_DAP_2_4_SURROUND_DECODER_ENABLE            = "db_key_am_audio_effect_dap_2_4_surround_decoder_enable";
static const char* DB_KEY_AM_AUDIO_EFFECT_DAP_2_4_LEVELER_MODE                       = "db_key_am_audio_effect_dap_2_4_leveler_mode";
static const char* DB_KEY_AM_AUDIO_EFFECT_DAP_2_4_LEVELER_AMOUNT                     = "db_key_am_audio_effect_dap_2_4_leveler_strength";

static const char* DB_KEY_AM_AUDIO_EFFECT_VIRTUALX_MODE                              = "db_key_am_audio_effect_virtualx_mode";
static const char* DB_KEY_AM_AUDIO_EFFECT_VIRTUALX_TREVOLUME_HD_ENABLE               = "db_key_am_audio_effect_virtualx_truvolume_hd_enabled";
static const char* DB_KEY_AM_AUDIO_EFFECT_VIRTUALX_DIALOG_CLARITY_MODE               = "db_key_am_audio_effect_virtualx_dialog_clarity_mode";
static const char* DB_KEY_AM_AUDIO_EFFECT_VIRTUALX_ENABLED                           = "db_key_am_audio_effect_virtualx_enabled";
static const char* DB_KEY_AM_AUDIO_EFFECT_VIRTUALX_VIRTUAL_SURROUND                  = "db_key_am_audio_effect_virtualx_virtual_surround";
static const char* DB_KEY_AM_AUDIO_EFFECT_VIRTUALX_TRUBASS_DISCARD                   = "db_key_am_audio_effect_virtualx_trubass_discard";

static const char* DB_KEY_AM_AUDIO_EFFECT_DUAL_SOUND_EFFECT_MODE                     = "db_key_am_audio_effect_dual_sound_effect_mode";
static const char* DB_KEY_AM_AUDIO_EFFECT_BASIC_EFFECT_MODE                          = "db_key_am_audio_effect_basic_effect_mode";
static const char* DB_KEY_AM_AUDIO_EFFECT_AI_SOUND_ENABLE                            = "db_key_am_audio_effect_ai_sound_enable";

static const vector<const char *> g_VecDbString = {
    DB_KEY_AM_AUDIO_EFFECT_HPEQ_SOUND_MODE,
    DB_KEY_AM_AUDIO_EFFECT_HPEQ_BAND1,
    DB_KEY_AM_AUDIO_EFFECT_HPEQ_BAND2,
    DB_KEY_AM_AUDIO_EFFECT_HPEQ_BAND3,
    DB_KEY_AM_AUDIO_EFFECT_HPEQ_BAND4,
    DB_KEY_AM_AUDIO_EFFECT_HPEQ_BAND5,
    DB_KEY_AM_AUDIO_EFFECT_HPEQ_BAND6,
    DB_KEY_AM_AUDIO_EFFECT_HPEQ_BAND7,
    DB_KEY_AM_AUDIO_EFFECT_HPEQ_BAND8,
    DB_KEY_AM_AUDIO_EFFECT_HPEQ_BAND9,

    DB_KEY_AM_AUDIO_EFFECT_BALANCE,

    DB_KEY_AM_AUDIO_EFFECT_TREBLEBASS_BASS,
    DB_KEY_AM_AUDIO_EFFECT_TREBLEBASS_TREBLE,

    DB_KEY_AM_AUDIO_EFFECT_VIRTUALSURROUND_ENABLE,

    DB_KEY_AM_AUDIO_EFFECT_DPE_ENABLED,
    DB_KEY_AM_AUDIO_EFFECT_DPE_INPUTGAIN,
    DB_KEY_AM_AUDIO_EFFECT_DPE_PRE_EQ,
    DB_KEY_AM_AUDIO_EFFECT_DPE_PRE_EQ_BAND0,
    DB_KEY_AM_AUDIO_EFFECT_DPE_PRE_EQ_BAND0_CUTOFFFREQUENCY,
    DB_KEY_AM_AUDIO_EFFECT_DPE_PRE_EQ_BAND0_GAIN,
    DB_KEY_AM_AUDIO_EFFECT_DPE_PRE_EQ_BAND1,
    DB_KEY_AM_AUDIO_EFFECT_DPE_PRE_EQ_BAND1_CUTOFFFREQUENCY,
    DB_KEY_AM_AUDIO_EFFECT_DPE_PRE_EQ_BAND1_GAIN,
    DB_KEY_AM_AUDIO_EFFECT_DPE_PRE_EQ_BAND2,
    DB_KEY_AM_AUDIO_EFFECT_DPE_PRE_EQ_BAND2_CUTOFFFREQUENCY,
    DB_KEY_AM_AUDIO_EFFECT_DPE_PRE_EQ_BAND2_GAIN,
    DB_KEY_AM_AUDIO_EFFECT_DPE_MBC,
    DB_KEY_AM_AUDIO_EFFECT_DPE_MBC_BAND0,
    DB_KEY_AM_AUDIO_EFFECT_DPE_MBC_BAND0_CUTOFFFREQUENCY,
    DB_KEY_AM_AUDIO_EFFECT_DPE_MBC_BAND0_ATTACKTIME,
    DB_KEY_AM_AUDIO_EFFECT_DPE_MBC_BAND0_RELEASETIME,
    DB_KEY_AM_AUDIO_EFFECT_DPE_MBC_BAND0_RATIO,
    DB_KEY_AM_AUDIO_EFFECT_DPE_MBC_BAND0_THRESHOLD,
    DB_KEY_AM_AUDIO_EFFECT_DPE_MBC_BAND0_KNEEWIDTH,
    DB_KEY_AM_AUDIO_EFFECT_DPE_MBC_BAND0_NOISEGATE_THRESHOLD,
    DB_KEY_AM_AUDIO_EFFECT_DPE_MBC_BAND0_EXPANDER_RATIO,
    DB_KEY_AM_AUDIO_EFFECT_DPE_MBC_BAND0_PRE_GAIN,
    DB_KEY_AM_AUDIO_EFFECT_DPE_MBC_BAND0_POST_GAIN,
    DB_KEY_AM_AUDIO_EFFECT_DPE_MBC_BAND1,
    DB_KEY_AM_AUDIO_EFFECT_DPE_MBC_BAND1_CUTOFFFREQUENCY,
    DB_KEY_AM_AUDIO_EFFECT_DPE_MBC_BAND1_ATTACKTIME,
    DB_KEY_AM_AUDIO_EFFECT_DPE_MBC_BAND1_RELEASETIME,
    DB_KEY_AM_AUDIO_EFFECT_DPE_MBC_BAND1_RATIO,
    DB_KEY_AM_AUDIO_EFFECT_DPE_MBC_BAND1_THRESHOLD,
    DB_KEY_AM_AUDIO_EFFECT_DPE_MBC_BAND1_KNEEWIDTH,
    DB_KEY_AM_AUDIO_EFFECT_DPE_MBC_BAND1_NOISEGATE_THRESHOLD,
    DB_KEY_AM_AUDIO_EFFECT_DPE_MBC_BAND1_EXPANDER_RATIO,
    DB_KEY_AM_AUDIO_EFFECT_DPE_MBC_BAND1_PRE_GAIN,
    DB_KEY_AM_AUDIO_EFFECT_DPE_MBC_BAND1_POST_GAIN,
    DB_KEY_AM_AUDIO_EFFECT_DPE_MBC_BAND2,
    DB_KEY_AM_AUDIO_EFFECT_DPE_MBC_BAND2_CUTOFFFREQUENCY,
    DB_KEY_AM_AUDIO_EFFECT_DPE_MBC_BAND2_ATTACKTIME,
    DB_KEY_AM_AUDIO_EFFECT_DPE_MBC_BAND2_RELEASETIME,
    DB_KEY_AM_AUDIO_EFFECT_DPE_MBC_BAND2_RATIO,
    DB_KEY_AM_AUDIO_EFFECT_DPE_MBC_BAND2_THRESHOLD,
    DB_KEY_AM_AUDIO_EFFECT_DPE_MBC_BAND2_KNEEWIDTH,
    DB_KEY_AM_AUDIO_EFFECT_DPE_MBC_BAND2_NOISEGATE_THRESHOLD,
    DB_KEY_AM_AUDIO_EFFECT_DPE_MBC_BAND2_EXPANDER_RATIO,
    DB_KEY_AM_AUDIO_EFFECT_DPE_MBC_BAND2_PRE_GAIN,
    DB_KEY_AM_AUDIO_EFFECT_DPE_MBC_BAND2_POST_GAIN,
    DB_KEY_AM_AUDIO_EFFECT_DPE_POST_EQ,
    DB_KEY_AM_AUDIO_EFFECT_DPE_POST_EQ_BAND0,
    DB_KEY_AM_AUDIO_EFFECT_DPE_POST_EQ_BAND0_CUTOFFFREQUENCY,
    DB_KEY_AM_AUDIO_EFFECT_DPE_POST_EQ_BAND0_GAIN,
    DB_KEY_AM_AUDIO_EFFECT_DPE_POST_EQ_BAND1,
    DB_KEY_AM_AUDIO_EFFECT_DPE_POST_EQ_BAND1_CUTOFFFREQUENCY,
    DB_KEY_AM_AUDIO_EFFECT_DPE_POST_EQ_BAND1_GAIN,
    DB_KEY_AM_AUDIO_EFFECT_DPE_POST_EQ_BAND2,
    DB_KEY_AM_AUDIO_EFFECT_DPE_POST_EQ_BAND2_CUTOFFFREQUENCY,
    DB_KEY_AM_AUDIO_EFFECT_DPE_POST_EQ_BAND2_GAIN,
    DB_KEY_AM_AUDIO_EFFECT_DPE_LIMITER,
    DB_KEY_AM_AUDIO_EFFECT_DPE_LIMITER_ATTACKTIME,
    DB_KEY_AM_AUDIO_EFFECT_DPE_LIMITER_RELEASETIMR,
    DB_KEY_AM_AUDIO_EFFECT_DPE_LIMITER_RATIO,
    DB_KEY_AM_AUDIO_EFFECT_DPE_LIMITER_THRESHOLD,
    DB_KEY_AM_AUDIO_EFFECT_DPE_LIMITER_POST_GAIN,

    DB_KEY_AM_AUDIO_EFFECT_DAP_MODE,
    DB_KEY_AM_AUDIO_EFFECT_DAP_GEQ_ENABLE,
    DB_KEY_AM_AUDIO_EFFECT_DAP_POST_GAIN,
    DB_KEY_AM_AUDIO_EFFECT_DAP_VL_ENABLE,
    DB_KEY_AM_AUDIO_EFFECT_DAP_VL_AMOUNT,
    DB_KEY_AM_AUDIO_EFFECT_DAP_DE_ENABLE,
    DB_KEY_AM_AUDIO_EFFECT_DAP_DE_AMOUNT,
    DB_KEY_AM_AUDIO_EFFECT_DAP_SURROUND_ENABLE,
    DB_KEY_AM_AUDIO_EFFECT_DAP_SURROUND_BOOST,
    DB_KEY_AM_AUDIO_EFFECT_DAP_GEQ_BAND1,
    DB_KEY_AM_AUDIO_EFFECT_DAP_GEQ_BAND2,
    DB_KEY_AM_AUDIO_EFFECT_DAP_GEQ_BAND3,
    DB_KEY_AM_AUDIO_EFFECT_DAP_GEQ_BAND4,
    DB_KEY_AM_AUDIO_EFFECT_DAP_GEQ_BAND5,
    DB_KEY_AM_AUDIO_EFFECT_DAP_2_4_PROFILE,
    DB_KEY_AM_AUDIO_EFFECT_DAP_2_4_SURROUND_VIRTUALIZER_MODE,
    DB_KEY_AM_AUDIO_EFFECT_DAP_2_4_SURROUND_VIRTUALIZER_BOOST,
    DB_KEY_AM_AUDIO_EFFECT_DAP_2_4_DIALOGUE_ENHANCER_ENABLE,
    DB_KEY_AM_AUDIO_EFFECT_DAP_2_4_BASS_ENHANCER_ENABLE,
    DB_KEY_AM_AUDIO_EFFECT_DAP_2_4_BASS_ENHANCER_BOOST,
    DB_KEY_AM_AUDIO_EFFECT_DAP_2_4_BASS_ENHANCER_CUTOFFX100,
    DB_KEY_AM_AUDIO_EFFECT_DAP_2_4_BASS_ENHANCER_CUTOFFX1,
    DB_KEY_AM_AUDIO_EFFECT_DAP_2_4_BASS_ENHANCER_WIDTH,
    DB_KEY_AM_AUDIO_EFFECT_DAP_2_4_MI_STEERING,
    DB_KEY_AM_AUDIO_EFFECT_DAP_2_4_SURROUND_DECODER_ENABLE,
    DB_KEY_AM_AUDIO_EFFECT_DAP_2_4_LEVELER_MODE,
    DB_KEY_AM_AUDIO_EFFECT_DAP_2_4_LEVELER_AMOUNT,

    DB_KEY_AM_AUDIO_EFFECT_VIRTUALX_MODE,
    DB_KEY_AM_AUDIO_EFFECT_VIRTUALX_TREVOLUME_HD_ENABLE,
    DB_KEY_AM_AUDIO_EFFECT_VIRTUALX_DIALOG_CLARITY_MODE,
    DB_KEY_AM_AUDIO_EFFECT_VIRTUALX_VIRTUAL_SURROUND,
    DB_KEY_AM_AUDIO_EFFECT_VIRTUALX_TRUBASS_DISCARD,

    DB_KEY_AM_AUDIO_EFFECT_DUAL_SOUND_EFFECT_MODE,
    DB_KEY_AM_AUDIO_EFFECT_BASIC_EFFECT_MODE,
    DB_KEY_AM_AUDIO_EFFECT_AI_SOUND_ENABLE,
};

AttributionSourceState createAttributionSource() {
    AttributionSourceState attributionSource;
    attributionSource.packageName = "audioserver";
    attributionSource.uid = VALUE_OR_FATAL(legacy2aidl_uid_t_int32_t(getuid()));
    attributionSource.pid = getpid();
    attributionSource.token = sp<BBinder>::make();
    return attributionSource;
}

status_t applyAudioEffectByPlayEmptyTrack() {
    AM_LOGI("start");
    status_t status;
    audio_attributes_t attributes;
    sp<AudioTrack> pAudioTrack = new AudioTrack();
    memset(&attributes, 0, sizeof(attributes));
    attributes.content_type = AUDIO_CONTENT_TYPE_MUSIC;
    attributes.usage = AUDIO_USAGE_MEDIA;
    status = pAudioTrack->set(AUDIO_STREAM_MUSIC, 48000, AUDIO_FORMAT_PCM_16_BIT, AUDIO_CHANNEL_OUT_STEREO,
                              0, AUDIO_OUTPUT_FLAG_NONE, nullptr, 0, 0, false, AUDIO_SESSION_OUTPUT_MIX,
                              AudioTrack::TRANSFER_DEFAULT, nullptr, createAttributionSource(), &attributes,
                              false, 1.0f, AUDIO_PORT_HANDLE_NONE);
    R_CHECK_RET(status, "set failed.")
    status = pAudioTrack->initCheck();
    R_CHECK_RET(status, "initCheck failed.")
    status = pAudioTrack->start();
    R_CHECK_RET(status, "start failed.")
    AM_LOGI("end");
    return 0;
}

AmlAudioEffect::AmlAudioEffect(DroidAudioEffectSetting *pSetting,int32_t id,
    const string& name, const string& type, const string& uuid) :
    AudioEffect(createAttributionSource()), mpSetting(pSetting), mId(id),
        mEffectName(name), mEffectType(type), mEffectUuid(uuid) {
}

int32_t AmlAudioEffect::putToDb(const string& key, int32_t value) {
    return mpSetting->putToDb(key, value);
}

int32_t AmlAudioEffect::getIntFromDb(const string& key) {
    return mpSetting->getIntFromDb(key);
}

int32_t AmlAudioEffect::getIntFromIni(const string& key) {
    return mpSetting->getIntFromIni(key);
}

int32_t AmlAudioEffect::setParameter(const vector<uint8_t>& param, const vector<uint8_t>& value) {
    if (param.size() == 0 || value.size() == 0) {
        AM_LOGE("invalid param, param.size:%zu, value.size:%zu", param.size(), value.size());
        return -1;
    }
    size_t totalSize = sizeof(effect_param_t) + param.size() + value.size();
    vector<uint8_t> buffer(totalSize);
    effect_param_t* pEffectParam = reinterpret_cast<effect_param_t*>(buffer.data());
    pEffectParam->psize = param.size();
    pEffectParam->vsize = value.size();
    memcpy(pEffectParam->data, param.data(), param.size());
    memcpy(pEffectParam->data + param.size(), value.data(), value.size());
    status_t ret = AudioEffect::setParameter(pEffectParam);
    R_CHECK_RET(ret,)
    return 0;
}

int32_t AmlAudioEffect::setParameter(int32_t param, int32_t value) {
    return setParameter(valueToByteArray(param), valueToByteArray(value));
}

int32_t AmlAudioEffect::setParameter(int32_t param, const vector<uint8_t>& value) {
    return setParameter(valueToByteArray(param), value);
}

int32_t AmlAudioEffect::getParameter(const vector<uint8_t>& param, vector<uint8_t>& value) {
    size_t totalSize = sizeof(effect_param_t) + param.size() + value.size();
    vector<uint8_t> buffer(totalSize);
    effect_param_t* pEffectParam = reinterpret_cast<effect_param_t*>(buffer.data());
    pEffectParam->psize = param.size();
    pEffectParam->vsize = value.size();
    memcpy(pEffectParam->data, param.data(), param.size());
    status_t ret = AudioEffect::getParameter(pEffectParam);
    memcpy(value.data(), pEffectParam->data + param.size(), value.size());
    R_CHECK_RET(ret,)
    return 0;
}

int32_t AmlAudioEffect::getParameter(int32_t param) {
    vector<uint8_t> valueBuffer(4);
    getParameter(valueToByteArray(param), valueBuffer);
    int32_t value = byteArrayToInt(valueBuffer);
    return value;
}

int32_t AmlAudioEffect::getParameter(int32_t param, vector<uint8_t>& value) {
    return getParameter(valueToByteArray(param), value);
}

AmlAudioEffectHpeq::AmlAudioEffectHpeq(DroidAudioEffectSetting* pSetting) :
    AmlAudioEffect(pSetting, DroidAudioEffect::EFFECT_ID_HPEQ, "Hpeq", "ce2c14af-84df-4c36-acf5-87e428ed05fc", "") {
}

int32_t AmlAudioEffectHpeq::init() {
    seParamEnable(DroidAudioEffect::EFFECT_CONFIG_ON);
    setHpeqBandNum(mpSetting->getHpeqBandNum());
    AM_LOGI("effect: %s bandSum:%d fistBoot:%d", mEffectName.c_str(), mpSetting->getHpeqBandNum(), mpSetting->firstBoot());
    if (!mpSetting->firstBoot()) {
        return 0;
    }
    vector<uint8_t> eqBandNum(DroidAudioEffect::HPEQ_MODE_EFFECT_BAND_MAX + 1);
    getParameter(PARAM_CMD_CUSTOM, eqBandNum);
    for (int32_t i = DroidAudioEffect::HPEQ_MODE_EFFECT_BAND_MIN; i <= DroidAudioEffect::HPEQ_MODE_EFFECT_BAND_MAX; i++) {
        string dbId = eqBandNumberToDbId(i);
        if (dbId.empty()) {
            AM_LOGE("the EQ band number:%d is invalid!", i);
            return -1;
        }
        putToDb(dbId, eqBandNum[i - DroidAudioEffect::HPEQ_MODE_EFFECT_BAND_MIN]);
    }
    int savedSoundMode = getIntFromDb(DB_KEY_AM_AUDIO_EFFECT_HPEQ_SOUND_MODE);
    setSoundMode(savedSoundMode);
    if (savedSoundMode == DroidAudioEffect::COMMON_SOUND_MODE_CUSTOM) {
        //set one band, at the same time the others will be set
        setDifferentBandEffects(DroidAudioEffect::HPEQ_MODE_EFFECT_BAND1, getIntFromDb(DB_KEY_AM_AUDIO_EFFECT_HPEQ_BAND1), false);
    }

    return 0;
}

int32_t AmlAudioEffectHpeq::seParamEnable(int32_t enable) {
    return setParameter(PARAM_CMD_ENABLE, enable);
}

//set sound mode except customed one
int32_t AmlAudioEffectHpeq::setSoundMode(int32_t mode) {
    if (isAudioDebug()) AM_LOGD("mode: %d(%s)", mode, DroidAudioEffect::soundMode2Str(mode));
    int32_t ret = setParameter(PARAM_CMD_SOUND_MODE, mode);
    R_CHECK_RET(ret,);
    return ret;
}

int32_t AmlAudioEffectHpeq::getSoundMode() {
    int32_t value = getParameter(PARAM_CMD_SOUND_MODE);
    int32_t saveResult = getIntFromDb(DB_KEY_AM_AUDIO_EFFECT_HPEQ_SOUND_MODE);
    if (saveResult != value) {
        AM_LOGW("erro get: %d(%s), saved: %d(%s)", value, DroidAudioEffect::soundMode2Str(value),
            saveResult, DroidAudioEffect::soundMode2Str(saveResult));
    }
    if (isAudioDebug()) AM_LOGD("mode: %d(%s)", saveResult, DroidAudioEffect::soundMode2Str(saveResult));
    return saveResult;
}

string AmlAudioEffectHpeq::eqBandNumberToDbId(int32_t bandNumber) {
    string dbId = "";
    switch (bandNumber) {
        case DroidAudioEffect::HPEQ_MODE_EFFECT_BAND1:
            dbId = DB_KEY_AM_AUDIO_EFFECT_HPEQ_BAND1;
            break;
        case DroidAudioEffect::HPEQ_MODE_EFFECT_BAND2:
            dbId = DB_KEY_AM_AUDIO_EFFECT_HPEQ_BAND2;
            break;
        case DroidAudioEffect::HPEQ_MODE_EFFECT_BAND3:
            dbId = DB_KEY_AM_AUDIO_EFFECT_HPEQ_BAND3;
            break;
        case DroidAudioEffect::HPEQ_MODE_EFFECT_BAND4:
            dbId = DB_KEY_AM_AUDIO_EFFECT_HPEQ_BAND4;
            break;
        case DroidAudioEffect::HPEQ_MODE_EFFECT_BAND5:
            dbId = DB_KEY_AM_AUDIO_EFFECT_HPEQ_BAND5;
            break;
        case DroidAudioEffect::HPEQ_MODE_EFFECT_BAND6:
            dbId = DB_KEY_AM_AUDIO_EFFECT_HPEQ_BAND6;
            break;
        case DroidAudioEffect::HPEQ_MODE_EFFECT_BAND7:
            dbId = DB_KEY_AM_AUDIO_EFFECT_HPEQ_BAND7;
            break;
        case DroidAudioEffect::HPEQ_MODE_EFFECT_BAND8:
            dbId = DB_KEY_AM_AUDIO_EFFECT_HPEQ_BAND8;
            break;
        case DroidAudioEffect::HPEQ_MODE_EFFECT_BAND9:
            dbId = DB_KEY_AM_AUDIO_EFFECT_HPEQ_BAND9;
            break;
        default:
            AM_LOGE("the EQ band number:%d is invalid!", bandNumber);
            return "";
        }
    return dbId;
}

int32_t AmlAudioEffectHpeq::setUserSoundModeParam(int32_t bandNumber, int32_t value, int32_t bandSum) {
    R_CHECK_PARAM_LEGAL(-1, bandNumber, DroidAudioEffect::HPEQ_MODE_EFFECT_BAND_MIN, DroidAudioEffect::HPEQ_MODE_EFFECT_BAND_MAX,)
    if (isAudioDebug()) {
        AM_LOGD("setUserSoundModeParam bandNumber:%d, value:%d", bandNumber, value);
    }
    if (bandSum != DroidAudioEffect::HPEQ_BAND_NUM_5 &&
        bandSum != DroidAudioEffect::HPEQ_BAND_NUM_7 &&
        bandSum != DroidAudioEffect::HPEQ_BAND_NUM_9) {
        AM_LOGE("the EQ band number:%d is invalid!", bandNumber);
        return -1;
    }
    setDifferentBandEffects(bandNumber, value, true);
    return 0;
}

int32_t AmlAudioEffectHpeq::getUserSoundModeParam(int32_t bandNumber) {
    string dbId = eqBandNumberToDbId(bandNumber);
    if (dbId.empty()) {
        AM_LOGE("the EQ band number:%d is invalid!", bandNumber);
        return 0;
    }

    int32_t value = getIntFromDb(dbId);
    if (isAudioDebug()) {
        AM_LOGD("band number:%d, value:%d", bandNumber, value);
    }
    return value;
}

int32_t AmlAudioEffectHpeq::setDifferentBandEffects(int32_t bandnum, int32_t value, bool needsave) {
    if (isAudioDebug()) AM_LOGD("NO.%d: %d", bandnum, value);
    vector<uint8_t> needband;
    for (int32_t i = DroidAudioEffect::HPEQ_MODE_EFFECT_BAND1; i <= DroidAudioEffect::HPEQ_MODE_EFFECT_BAND9; i++) {
        if (bandnum == i) {
            needband.push_back(value);
        } else {
            string dbId = eqBandNumberToDbId(i);
            if (dbId.empty()) {
                AM_LOGE("the EQ band number:%d is invalid!", i);
                return -1;
            }
            needband.push_back(getIntFromDb(dbId));
        }
    }
    int32_t ret = setParameter(PARAM_CMD_CUSTOM, needband);
    if (needsave) {
        string dbId = eqBandNumberToDbId(bandnum);
        if (dbId.empty()) {
            AM_LOGE("the EQ band number:%d is invalid!", bandnum);
            return -1;
        }
        putToDb(dbId, value);
    }
    return ret;
}

int32_t AmlAudioEffectHpeq::setHpeqBandNum(int32_t num) {
    if (isAudioDebug()) AM_LOGD("num:%d", num);
    setParameter(PARAM_CMD_BAND_NUM, num);
    return 0;
}

int32_t AmlAudioEffectHpeq::toHpeqSoundMode(int32_t mode) {
    switch (mode) {
        case DroidAudioEffect::COMMON_SOUND_MODE_STANDARD:
            return HPEQ_SOUND_MODE_STANDARD;
        case DroidAudioEffect::COMMON_SOUND_MODE_MUSIC:
            return HPEQ_SOUND_MODE_MUSIC;
        case DroidAudioEffect::COMMON_SOUND_MODE_GAME:
            return HPEQ_SOUND_MODE_GAME;
        case DroidAudioEffect::COMMON_SOUND_MODE_MOVIE:
            return HPEQ_SOUND_MODE_MOVIE;
        case DroidAudioEffect::COMMON_SOUND_MODE_CUSTOM:
            return HPEQ_SOUND_MODE_CUSTOM;
        case DroidAudioEffect::COMMON_SOUND_MODE_NEWS:
            return HPEQ_SOUND_MODE_NEWS;
        default: {
            AM_LOGW("Not support SoundMode:%d", mode);
            return HPEQ_SOUND_MODE_STANDARD;
        }
    }
}

AmlAudioEffectBalance::AmlAudioEffectBalance(DroidAudioEffectSetting* pSetting) :
    AmlAudioEffect(pSetting, DroidAudioEffect::EFFECT_ID_BALANCE, "Balance", "7cb34dc0-242e-11e6-bb63-0002a5d5c51b", "") {
}

int32_t AmlAudioEffectBalance::init() {
    setBalance(getIntFromDb(DB_KEY_AM_AUDIO_EFFECT_BALANCE));
    return 0;
}

int32_t AmlAudioEffectBalance::seParamEnable(int32_t enable) {
    return setParameter(PARAM_CMD_BALANCE_ENABLE, enable);
}

int32_t AmlAudioEffectBalance::setBalance(int32_t step) {
    R_CHECK_PARAM_LEGAL(-1, step, DroidAudioEffect::EFFECT_CONFIG_STEP_MIN, DroidAudioEffect::EFFECT_CONFIG_STEP_MAX,)
    if (isAudioDebug()) AM_LOGD("step:%d", step);
    putToDb(DB_KEY_AM_AUDIO_EFFECT_BALANCE, step);
    int32_t ret = setParameter(PARAM_CMD_BALANCE_LEVEL, step);
    R_CHECK_RET(ret,)
    return 0;
}

int32_t AmlAudioEffectBalance::getBalance() {
    int32_t value = getParameter(PARAM_CMD_BALANCE_LEVEL);
    int32_t saveResult = getIntFromDb(DB_KEY_AM_AUDIO_EFFECT_BALANCE);
    if (saveResult != value) {
        AM_LOGW("erro get:%d, saved:%d", value, saveResult);
    }
    if (isAudioDebug()) AM_LOGD("value: %d", saveResult);
    return saveResult;
}

AmlAudioEffectTrebleBass::AmlAudioEffectTrebleBass(DroidAudioEffectSetting* pSetting) :
    AmlAudioEffect(pSetting, DroidAudioEffect::EFFECT_ID_TREBLEBASS, "TrebleBass", "7e282240-242e-11e6-bb63-0002a5d5c51b", "") {
}

int32_t AmlAudioEffectTrebleBass::init() {
    setBass(getIntFromDb(DB_KEY_AM_AUDIO_EFFECT_TREBLEBASS_BASS));
    setTreble(getIntFromDb(DB_KEY_AM_AUDIO_EFFECT_TREBLEBASS_TREBLE));
    return 0;
}

int32_t AmlAudioEffectTrebleBass::seParamEnable(int32_t enable) {
    return setParameter(PARAM_CMD_TREBLEBASS_ENABLE, enable);
}

int32_t AmlAudioEffectTrebleBass::setTreble(int32_t step) {
    R_CHECK_PARAM_LEGAL(-1, step, DroidAudioEffect::EFFECT_CONFIG_STEP_MIN, DroidAudioEffect::EFFECT_CONFIG_STEP_MAX,)
    if (isAudioDebug()) AM_LOGD("step:%d", step);
    putToDb(DB_KEY_AM_AUDIO_EFFECT_TREBLEBASS_TREBLE, step);
    int32_t ret = setParameter(PARAM_CMD_TREBLEBASS_TREBLE_LEVEL, step);
    R_CHECK_RET(ret,)
    return 0;
}

int32_t AmlAudioEffectTrebleBass::getTreble() {
    int32_t value = getParameter(PARAM_CMD_TREBLEBASS_TREBLE_LEVEL);
    int32_t saveResult = getIntFromDb(DB_KEY_AM_AUDIO_EFFECT_TREBLEBASS_TREBLE);
    if (saveResult != value) {
        AM_LOGW("erro get:%d, saved:%d", value, saveResult);
    }
    if (isAudioDebug()) AM_LOGD("value: %d", saveResult);
    return saveResult;
}

int32_t AmlAudioEffectTrebleBass::setBass(int32_t step) {
    R_CHECK_PARAM_LEGAL(-1, step, DroidAudioEffect::EFFECT_CONFIG_STEP_MIN, DroidAudioEffect::EFFECT_CONFIG_STEP_MAX,)
    if (isAudioDebug()) AM_LOGD("step:%d", step);
    putToDb(DB_KEY_AM_AUDIO_EFFECT_TREBLEBASS_BASS, step);
    int32_t ret = setParameter(PARAM_CMD_TREBLEBASS_BASS_LEVEL, step);
    R_CHECK_RET(ret,)
    return 0;
}

int32_t AmlAudioEffectTrebleBass::getBass() {
    int32_t value = getParameter(PARAM_CMD_TREBLEBASS_BASS_LEVEL);
    int32_t saveResult = getIntFromDb(DB_KEY_AM_AUDIO_EFFECT_TREBLEBASS_BASS);
    if (saveResult != value) {
        AM_LOGW("erro get:%d, saved:%d", value, saveResult);
    }
    if (isAudioDebug()) AM_LOGD("value: %d", saveResult);
    return saveResult;
}

AmlAudioEffectVirtualSurround::AmlAudioEffectVirtualSurround(DroidAudioEffectSetting* pSetting) :
    AmlAudioEffect(pSetting, DroidAudioEffect::EFFECT_ID_VIRTUALSURROUND, "VirtualSurround", "c656ec6f-d6be-4e7f-854b-1218077f3915", "") {
}

int32_t AmlAudioEffectVirtualSurround::init() {
    int32_t ret = setVirtualSurroundEnabled(getIntFromDb(DB_KEY_AM_AUDIO_EFFECT_VIRTUALSURROUND_ENABLE));
    R_CHECK_RET(ret,);
    return ret;
}

int32_t AmlAudioEffectVirtualSurround::setVirtualSurroundEnabled(bool enable) {
    if (isAudioDebug()) AM_LOGD("enable:%d", enable);
    int32_t ret = setParameter(PARAM_CMD_ENABLE, enable);
    putToDb(DB_KEY_AM_AUDIO_EFFECT_VIRTUALSURROUND_ENABLE, enable);
    R_CHECK_RET(ret,);
    return ret;
}

bool AmlAudioEffectVirtualSurround::isVirtualSurroundEnabled() {
    int32_t value = getParameter(PARAM_CMD_ENABLE);
    int32_t saveResult = getIntFromDb(DB_KEY_AM_AUDIO_EFFECT_VIRTUALSURROUND_ENABLE);
    if (saveResult != value) {
        AM_LOGW("erro get:%d, saved:%d", value, saveResult);
    }
    bool enable = (saveResult != 0);
    if (isAudioDebug()) AM_LOGD("enable: %d", enable);
    return saveResult;
}

AmlAudioEffectDpe::AmlAudioEffectDpe(DroidAudioEffectSetting* pSetting) :
    AmlAudioEffect(pSetting, DroidAudioEffect::EFFECT_ID_DPE, "DPE", "f70bcbf4-7457-11ec-b4f0-020017000b7b", "") {
}
int32_t AmlAudioEffectDpe::init() {
    int32_t value = 0;
    value = getDpeParam(DroidAudioEffect::DPE_CMD_ENABLED);
    setDpeParam(DroidAudioEffect::DPE_CMD_ENABLED, getDpeParam(DroidAudioEffect::DPE_CMD_ENABLED));
    //AM_LOGD("setDpeParam,DroidAudioEffect::DPE_CMD_ENABLED:", value);
    if (value == 0) {
        AM_LOGI("DPE is off.");
        return -1;
    }
    value = getDpeParam(DroidAudioEffect::DPE_CMD_INPUTGAIN);
    setDpeParam(DroidAudioEffect::DPE_CMD_INPUTGAIN, getDpeParam(DroidAudioEffect::DPE_CMD_INPUTGAIN));
    //AM_LOGD("setDpeParam,DroidAudioEffect::DPE_CMD_INPUTGAIN:", value);

    value = getDpeParam(DroidAudioEffect::DPE_CMD_PRE_EQ);
    setDpeParam(DroidAudioEffect::DPE_CMD_PRE_EQ, getDpeParam(DroidAudioEffect::DPE_CMD_PRE_EQ));
    //AM_LOGD("setDpeParam,DroidAudioEffect::DPE_CMD_PRE_EQ:", value);

    value = getDpeParam(DroidAudioEffect::DPE_CMD_MBC);
    setDpeParam(DroidAudioEffect::DPE_CMD_MBC, getDpeParam(DroidAudioEffect::DPE_CMD_MBC));
    //AM_LOGD("setDpeParam,DroidAudioEffect::DPE_CMD_MBC:", value);

    value = getDpeParam(DroidAudioEffect::DPE_CMD_POST_EQ);
    setDpeParam(DroidAudioEffect::DPE_CMD_POST_EQ, getDpeParam(DroidAudioEffect::DPE_CMD_POST_EQ));
    //AM_LOGD("setDpeParam,DroidAudioEffect::DPE_CMD_POST_EQ:", value);

    value = getDpeParam(DroidAudioEffect::DPE_CMD_LIMITER);
    setDpeParam(DroidAudioEffect::DPE_CMD_LIMITER, getDpeParam(DroidAudioEffect::DPE_CMD_LIMITER));
    //AM_LOGD("setDpeParam,DroidAudioEffect::DPE_CMD_LIMITER:", value);

    //pre eq band
    value = getDpeParam(DroidAudioEffect::DPE_SUBCMD_PRE_EQ_BAND0_CUTOFFFREQUENCY);
    setDpeParam(DroidAudioEffect::DPE_SUBCMD_PRE_EQ_BAND0_CUTOFFFREQUENCY, getDpeParam(DroidAudioEffect::DPE_SUBCMD_PRE_EQ_BAND0_CUTOFFFREQUENCY));
    //AM_LOGD("setDpeParam,DroidAudioEffect::DPE_SUBCMD_PRE_EQ_BAND0_CUTOFFFREQUENCY:", value);

    setDpeParam(DroidAudioEffect::DPE_SUBCMD_PRE_EQ_BAND0_GAIN, getDpeParam(DroidAudioEffect::DPE_SUBCMD_PRE_EQ_BAND0_GAIN));
    setDpeParam(DroidAudioEffect::DPE_SUBCMD_PRE_EQ_BAND1_CUTOFFFREQUENCY, getDpeParam(DroidAudioEffect::DPE_SUBCMD_PRE_EQ_BAND1_CUTOFFFREQUENCY));
    setDpeParam(DroidAudioEffect::DPE_SUBCMD_PRE_EQ_BAND1_GAIN, getDpeParam(DroidAudioEffect::DPE_SUBCMD_PRE_EQ_BAND1_GAIN));
    setDpeParam(DroidAudioEffect::DPE_SUBCMD_PRE_EQ_BAND2_CUTOFFFREQUENCY, getDpeParam(DroidAudioEffect::DPE_SUBCMD_PRE_EQ_BAND2_CUTOFFFREQUENCY));
    setDpeParam(DroidAudioEffect::DPE_SUBCMD_PRE_EQ_BAND2_GAIN, getDpeParam(DroidAudioEffect::DPE_SUBCMD_PRE_EQ_BAND2_GAIN));

    //post eq band
    setDpeParam(DroidAudioEffect::DPE_SUBCMD_POST_EQ_BAND0_CUTOFFFREQUENCY, getDpeParam(DroidAudioEffect::DPE_SUBCMD_POST_EQ_BAND0_CUTOFFFREQUENCY));
    setDpeParam(DroidAudioEffect::DPE_SUBCMD_POST_EQ_BAND0_GAIN, getDpeParam(DroidAudioEffect::DPE_SUBCMD_POST_EQ_BAND0_GAIN));
    setDpeParam(DroidAudioEffect::DPE_SUBCMD_POST_EQ_BAND1_CUTOFFFREQUENCY, getDpeParam(DroidAudioEffect::DPE_SUBCMD_POST_EQ_BAND1_CUTOFFFREQUENCY));
    setDpeParam(DroidAudioEffect::DPE_SUBCMD_POST_EQ_BAND1_GAIN, getDpeParam(DroidAudioEffect::DPE_SUBCMD_POST_EQ_BAND1_GAIN));
    setDpeParam(DroidAudioEffect::DPE_SUBCMD_POST_EQ_BAND2_CUTOFFFREQUENCY, getDpeParam(DroidAudioEffect::DPE_SUBCMD_POST_EQ_BAND2_CUTOFFFREQUENCY));
    setDpeParam(DroidAudioEffect::DPE_SUBCMD_POST_EQ_BAND2_GAIN, getDpeParam(DroidAudioEffect::DPE_SUBCMD_POST_EQ_BAND2_GAIN));

    int32_t id = 0;
    //mbc band
    for (id = DroidAudioEffect::DPE_SUBCMD_MBC_BAND0_CUTOFFFREQUENCY; id <= DroidAudioEffect::DPE_SUBCMD_MBC_BAND0_POSTGAIN; id++) {
        setDpeParam(id, getDpeParam(id));
    }
    for (id = DroidAudioEffect::DPE_SUBCMD_MBC_BAND1_CUTOFFFREQUENCY; id <= DroidAudioEffect::DPE_SUBCMD_MBC_BAND1_POSTGAIN; id++) {
        setDpeParam(id, getDpeParam(id));
    }
    for (id = DroidAudioEffect::DPE_SUBCMD_MBC_BAND2_CUTOFFFREQUENCY; id <= DroidAudioEffect::DPE_SUBCMD_MBC_BAND2_POSTGAIN; id++) {
        setDpeParam(id, getDpeParam(id));
    }

    //limiter
    for (id = DroidAudioEffect::DPE_SUBCMD_LIMITER_ATTACKTIME; id <= DroidAudioEffect::DPE_SUBCMD_LIMITER_POSTGAIN; id++) {
        setDpeParam(id, getDpeParam(id));
    }
    return 0;
}

//dpe get param internal
int32_t AmlAudioEffectDpe::getDpeParamInternal(int32_t id) {
    int32_t result = 0;
    switch (id) {
        //dpe enabled
        case DroidAudioEffect::DPE_CMD_ENABLED: {
                vector<uint8_t> tempEngineParam = {(uint8_t)id,0,0,0};
                vector<uint8_t> engineBytes(4 + 36);
                getParameter(tempEngineParam, engineBytes);
                result = byteArrayToInt(engineBytes);
            }
            break;
        //dpe inputgain
        case DroidAudioEffect::DPE_CMD_INPUTGAIN: {
                vector<uint8_t> tempInputValue(4);
                vector<uint8_t> tempParam_input = {(uint8_t)id, 0, 0, 0, 0, 0, 0, 0};  //channel 0
                getParameter(tempParam_input, tempInputValue);
                result = byteArrayToInt(extractSubvector(tempInputValue, 0, 4));
                AM_LOGD("Inputgain hal Value:%d", result);
            }
            break;
        // pre eq band 0
        case DroidAudioEffect::DPE_SUBCMD_PRE_EQ_BAND0:
        case DroidAudioEffect::DPE_SUBCMD_PRE_EQ_BAND0_CUTOFFFREQUENCY:
        case DroidAudioEffect::DPE_SUBCMD_PRE_EQ_BAND0_GAIN: {
                vector<uint8_t> tempParamPreEq0Cmd = valueToByteArray(DroidAudioEffect::DPE_CMD_PRE_EQ_BAND);
                vector<uint8_t> tempParamPreEq0Ch = valueToByteArray(0); //channel 0
                vector<uint8_t> tempParamPreEq0Band = valueToByteArray(0); // band 0
                vector<uint8_t> tempParam_preEq0;
                tempParam_preEq0.insert(tempParam_preEq0.end(), tempParamPreEq0Cmd.begin(), tempParamPreEq0Cmd.end());
                tempParam_preEq0.insert(tempParam_preEq0.end(), tempParamPreEq0Ch.begin(), tempParamPreEq0Ch.end());
                tempParam_preEq0.insert(tempParam_preEq0.end(), tempParamPreEq0Band.begin(), tempParamPreEq0Band.end());
                vector<uint8_t> tempValue(4 + 4 + 4); //  PreEq0En + PreEq0Cut + PreEq0Gain
                getParameter(tempParam_preEq0, tempValue);
                //get
                if (id == DroidAudioEffect::DPE_SUBCMD_PRE_EQ_BAND0_CUTOFFFREQUENCY) {
                    result = byteArrayToInt(extractSubvector(tempValue, 0, 4));
                } else if (id == DroidAudioEffect::DPE_SUBCMD_PRE_EQ_BAND0_GAIN) {
                    result = byteArrayToInt(extractSubvector(tempValue, 4, 8));
                } else if (id == DroidAudioEffect::DPE_SUBCMD_PRE_EQ_BAND0) {
                    result = byteArrayToInt(extractSubvector(tempValue, 8, 12));
                }
            }
            break;
    }
    return result;
}

//dpe set param
int32_t AmlAudioEffectDpe::setDpeParam(int32_t id, int32_t value) {
    if (isAudioDebug()) AM_LOGD("id:%d, value:%d", id, value);
    switch (id) {
        //set dpe enabled
        case DroidAudioEffect::DPE_CMD_ENABLED: {
                setEnabled(value == 1);
                vector<uint8_t> tempEngineVa = valueToByteArray(value - 1);
                vector<uint8_t> tempEnginePref = valueToByteArray(DEFAULT_DPE_FRAME_DURATION);
                vector<uint8_t> tempEnginePreEqIu = valueToByteArray(value);
                vector<uint8_t> tempEnginePreEqBc = valueToByteArray(DEFAULT_DPE_BAND_AMOUNT);
                vector<uint8_t> tempEngineMbcIu = valueToByteArray(value);
                vector<uint8_t> tempEngineMbcBc = valueToByteArray(DEFAULT_DPE_BAND_AMOUNT);
                vector<uint8_t> tempEnginePostEqIu = valueToByteArray(value);
                vector<uint8_t> tempEnginePostEqBc = valueToByteArray(DEFAULT_DPE_BAND_AMOUNT);
                vector<uint8_t> tempEngineLimiterIu = valueToByteArray(value);

                vector<uint8_t> combinedBytes;
                combinedBytes.insert(combinedBytes.end(), tempEngineVa.begin(), tempEngineVa.end());
                combinedBytes.insert(combinedBytes.end(), tempEnginePref.begin(), tempEnginePref.end());
                combinedBytes.insert(combinedBytes.end(), tempEnginePreEqIu.begin(), tempEnginePreEqIu.end());
                combinedBytes.insert(combinedBytes.end(), tempEnginePreEqBc.begin(), tempEnginePreEqBc.end());
                combinedBytes.insert(combinedBytes.end(), tempEngineMbcIu.begin(), tempEngineMbcIu.end());
                combinedBytes.insert(combinedBytes.end(), tempEngineMbcBc.begin(), tempEngineMbcBc.end());
                combinedBytes.insert(combinedBytes.end(), tempEnginePostEqIu.begin(), tempEnginePostEqIu.end());
                combinedBytes.insert(combinedBytes.end(), tempEnginePostEqBc.begin(), tempEnginePostEqBc.end());
                combinedBytes.insert(combinedBytes.end(), tempEngineLimiterIu.begin(), tempEngineLimiterIu.end());
                setParameter(id, combinedBytes);
                setDpeToDb(id, value);
            }
            break;

        // set dpe inputgain
        case DroidAudioEffect::DPE_CMD_INPUTGAIN: {
                vector<uint8_t> tempInputValue = valueToByteArray((float)value);
                vector<uint8_t> tempParam_input = {(uint8_t)id, 0, 0, 0, 0, 0, 0, 0};  //channel 0
                setParameter(tempParam_input, tempInputValue);
                tempParam_input[4] = 1;  //channel 1
                setParameter(tempParam_input, tempInputValue);
                setDpeToDb(id, value);
            }
            break;

        // set eq mbc inuse
        case DroidAudioEffect::DPE_CMD_PRE_EQ:
        case DroidAudioEffect::DPE_CMD_MBC:
        case DroidAudioEffect::DPE_CMD_POST_EQ: {
                vector<uint8_t> tempParam_on = {(uint8_t)id, 0, 0, 0, 0, 0, 0, 0}; //channel 0
                if (value == 1) {
                    vector<uint8_t> tempValue_on = {1, 0, 0, 0, 1, 0, 0, 0, 3, 0, 0, 0};
                    setParameter(tempParam_on, tempValue_on);
                    tempParam_on[4] = 1; //channel 1
                    setParameter(tempParam_on, tempValue_on);
                } else {
                    vector<uint8_t> tempValue_on = {0, 0, 0, 0, 0, 0, 0, 0, 3, 0, 0, 0};
                    setParameter(tempParam_on, tempValue_on);
                    tempParam_on[4] = 1; //channel 1
                    setParameter(tempParam_on, tempValue_on);
                }
                setDpeToDb(id, value);
            }
            break;

        // pre eq band 0
        case DroidAudioEffect::DPE_SUBCMD_PRE_EQ_BAND0:
        case DroidAudioEffect::DPE_SUBCMD_PRE_EQ_BAND0_CUTOFFFREQUENCY:
        case DroidAudioEffect::DPE_SUBCMD_PRE_EQ_BAND0_GAIN: {
                vector<uint8_t> tempParamPreEq0Cmd = valueToByteArray(DroidAudioEffect::DPE_CMD_PRE_EQ_BAND);
                vector<uint8_t> tempParamPreEq0Ch = valueToByteArray(0); //channel 0
                vector<uint8_t> tempParamPreEq0Band = valueToByteArray(0); // band 0
                vector<uint8_t> tempParam_preEq0;
                tempParam_preEq0.insert(tempParam_preEq0.end(), tempParamPreEq0Cmd.begin(), tempParamPreEq0Cmd.end());
                tempParam_preEq0.insert(tempParam_preEq0.end(), tempParamPreEq0Ch.begin(), tempParamPreEq0Ch.end());
                tempParam_preEq0.insert(tempParam_preEq0.end(), tempParamPreEq0Band.begin(), tempParamPreEq0Band.end());
                vector<uint8_t> tempValuePreEq0En = valueToByteArray(getDpeParam(DroidAudioEffect::DPE_SUBCMD_PRE_EQ_BAND0));
                vector<uint8_t> tempValuePreEq0Cut = valueToByteArray((float)getDpeParam(DroidAudioEffect::DPE_SUBCMD_PRE_EQ_BAND0_CUTOFFFREQUENCY));
                vector<uint8_t> tempValuePreEq0Gain = valueToByteArray((float)getDpeParam(DroidAudioEffect::DPE_SUBCMD_PRE_EQ_BAND0_GAIN));

                //set
                if (id == DroidAudioEffect::DPE_SUBCMD_PRE_EQ_BAND0_CUTOFFFREQUENCY) {
                    tempValuePreEq0Cut = valueToByteArray((float)value);
                } else if (id == DroidAudioEffect::DPE_SUBCMD_PRE_EQ_BAND0_GAIN) {
                    tempValuePreEq0Gain = valueToByteArray((float)value);
                } else if (id == DroidAudioEffect::DPE_SUBCMD_PRE_EQ_BAND0) {
                    tempValuePreEq0En = valueToByteArray(value);
                }
                vector<uint8_t> tempValue_preEq0;
                tempValue_preEq0.insert(tempValue_preEq0.end(), tempValuePreEq0En.begin(), tempValuePreEq0En.end());
                tempValue_preEq0.insert(tempValue_preEq0.end(), tempValuePreEq0Cut.begin(), tempValuePreEq0Cut.end());
                tempValue_preEq0.insert(tempValue_preEq0.end(), tempValuePreEq0Gain.begin(), tempValuePreEq0Gain.end());
                setParameter(tempParam_preEq0, tempValue_preEq0);
                tempParam_preEq0[4] = 1; //channel 1
                setParameter(tempParam_preEq0, tempValue_preEq0);
                setDpeToDb(id, value);
            }
            break;

        // pre eq band 1
        case DroidAudioEffect::DPE_SUBCMD_PRE_EQ_BAND1:
        case DroidAudioEffect::DPE_SUBCMD_PRE_EQ_BAND1_CUTOFFFREQUENCY:
        case DroidAudioEffect::DPE_SUBCMD_PRE_EQ_BAND1_GAIN: {
                vector<uint8_t> tempParamPreEq1Cmd = valueToByteArray(DroidAudioEffect::DPE_CMD_PRE_EQ_BAND);
                vector<uint8_t> tempParamPreEq1Ch = valueToByteArray(0); //channel 0
                vector<uint8_t> tempParamPreEq1Band = valueToByteArray(1); // band 1
                vector<uint8_t> tempParam_preEq1;
                tempParam_preEq1.insert(tempParam_preEq1.end(), tempParamPreEq1Cmd.begin(), tempParamPreEq1Cmd.end());
                tempParam_preEq1.insert(tempParam_preEq1.end(), tempParamPreEq1Ch.begin(), tempParamPreEq1Ch.end());
                tempParam_preEq1.insert(tempParam_preEq1.end(), tempParamPreEq1Band.begin(), tempParamPreEq1Band.end());
                vector<uint8_t> tempValuePreEq1En = valueToByteArray(getDpeParam(DroidAudioEffect::DPE_SUBCMD_PRE_EQ_BAND1));
                vector<uint8_t> tempValuePreEq1Cut = valueToByteArray((float)getDpeParam(DroidAudioEffect::DPE_SUBCMD_PRE_EQ_BAND1_CUTOFFFREQUENCY));
                vector<uint8_t> tempValuePreEq1Gain = valueToByteArray((float)getDpeParam(DroidAudioEffect::DPE_SUBCMD_PRE_EQ_BAND1_GAIN));
                //set
                if (id == DroidAudioEffect::DPE_SUBCMD_PRE_EQ_BAND1_CUTOFFFREQUENCY) {
                    tempValuePreEq1Cut = valueToByteArray((float)value);
                } else if (id == DroidAudioEffect::DPE_SUBCMD_PRE_EQ_BAND1_GAIN) {
                    tempValuePreEq1Gain = valueToByteArray((float)value);
                } else if (id == DroidAudioEffect::DPE_SUBCMD_PRE_EQ_BAND1) {
                    tempValuePreEq1En = valueToByteArray(value);
                }
                vector<uint8_t> tempValue_preEq1;
                tempValue_preEq1.insert(tempValue_preEq1.end(), tempValuePreEq1En.begin(), tempValuePreEq1En.end());
                tempValue_preEq1.insert(tempValue_preEq1.end(), tempValuePreEq1Cut.begin(), tempValuePreEq1Cut.end());
                tempValue_preEq1.insert(tempValue_preEq1.end(), tempValuePreEq1Gain.begin(), tempValuePreEq1Gain.end());
                setParameter(tempParam_preEq1, tempValue_preEq1);
                tempParam_preEq1[4] = 1; //channel 1
                setParameter(tempParam_preEq1, tempValue_preEq1);
                setDpeToDb(id, value);
            }
            break;

        // pre eq band 2
        case DroidAudioEffect::DPE_SUBCMD_PRE_EQ_BAND2:
        case DroidAudioEffect::DPE_SUBCMD_PRE_EQ_BAND2_CUTOFFFREQUENCY:
        case DroidAudioEffect::DPE_SUBCMD_PRE_EQ_BAND2_GAIN: {
                vector<uint8_t> tempParamPreEq2Cmd = valueToByteArray(DroidAudioEffect::DPE_CMD_PRE_EQ_BAND);
                vector<uint8_t> tempParamPreEq2Ch = valueToByteArray(0); //channel 0
                vector<uint8_t> tempParamPreEq2Band = valueToByteArray(2); // band 2
                vector<uint8_t> tempParam_preEq2;
                tempParam_preEq2.insert(tempParam_preEq2.end(), tempParamPreEq2Cmd.begin(), tempParamPreEq2Cmd.end());
                tempParam_preEq2.insert(tempParam_preEq2.end(), tempParamPreEq2Ch.begin(), tempParamPreEq2Ch.end());
                tempParam_preEq2.insert(tempParam_preEq2.end(), tempParamPreEq2Band.begin(), tempParamPreEq2Band.end());
                vector<uint8_t> tempValuePreEq2En = valueToByteArray(getDpeParam(DroidAudioEffect::DPE_SUBCMD_PRE_EQ_BAND2));
                vector<uint8_t> tempValuePreEq2Cut = valueToByteArray((float)getDpeParam(DroidAudioEffect::DPE_SUBCMD_PRE_EQ_BAND2_CUTOFFFREQUENCY));
                vector<uint8_t> tempValuePreEq2Gain = valueToByteArray((float)getDpeParam(DroidAudioEffect::DPE_SUBCMD_PRE_EQ_BAND2_GAIN));
                //set
                if (id == DroidAudioEffect::DPE_SUBCMD_PRE_EQ_BAND2_CUTOFFFREQUENCY) {
                    tempValuePreEq2Cut = valueToByteArray((float)value);
                } else if (id == DroidAudioEffect::DPE_SUBCMD_PRE_EQ_BAND2_GAIN) {
                    tempValuePreEq2Gain = valueToByteArray((float)value);
                } else if (id == DroidAudioEffect::DPE_SUBCMD_PRE_EQ_BAND2) {
                    tempValuePreEq2En = valueToByteArray(value);
                }
                vector<uint8_t> tempValue_preEq2;
                tempValue_preEq2.insert(tempValue_preEq2.end(), tempValuePreEq2En.begin(), tempValuePreEq2En.end());
                tempValue_preEq2.insert(tempValue_preEq2.end(), tempValuePreEq2Cut.begin(), tempValuePreEq2Cut.end());
                tempValue_preEq2.insert(tempValue_preEq2.end(), tempValuePreEq2Gain.begin(), tempValuePreEq2Gain.end());
                setParameter(tempParam_preEq2, tempValue_preEq2);
                tempParam_preEq2[4] = 1; //channel 1
                setParameter(tempParam_preEq2, tempValue_preEq2);
                setDpeToDb(id, value);
            }
            break;

        // post eq band 0
        case DroidAudioEffect::DPE_SUBCMD_POST_EQ_BAND0:
        case DroidAudioEffect::DPE_SUBCMD_POST_EQ_BAND0_CUTOFFFREQUENCY:
        case DroidAudioEffect::DPE_SUBCMD_POST_EQ_BAND0_GAIN: {
                vector<uint8_t> tempParamPostEq0Cmd = valueToByteArray(DroidAudioEffect::DPE_CMD_POST_EQ_BAND);
                vector<uint8_t> tempParamPostEq0Ch = valueToByteArray(0); //channel 0
                vector<uint8_t> tempParamPostEq0Band = valueToByteArray(0); // band 0
                vector<uint8_t> tempParam_postEq0;
                tempParam_postEq0.insert(tempParam_postEq0.end(), tempParamPostEq0Cmd.begin(), tempParamPostEq0Cmd.end());
                tempParam_postEq0.insert(tempParam_postEq0.end(), tempParamPostEq0Ch.begin(), tempParamPostEq0Ch.end());
                tempParam_postEq0.insert(tempParam_postEq0.end(), tempParamPostEq0Band.begin(), tempParamPostEq0Band.end());
                vector<uint8_t> tempValuePostEq0En = valueToByteArray(getDpeParam(DroidAudioEffect::DPE_SUBCMD_POST_EQ_BAND0));
                vector<uint8_t> tempValuePostEq0Cut = valueToByteArray((float)getDpeParam(DroidAudioEffect::DPE_SUBCMD_POST_EQ_BAND0_CUTOFFFREQUENCY));
                vector<uint8_t> tempValuePostEq0Gain = valueToByteArray((float)getDpeParam(DroidAudioEffect::DPE_SUBCMD_POST_EQ_BAND0_GAIN));
                //set
                vector<uint8_t> tempValue_postEq0;
                if (id == DroidAudioEffect::DPE_SUBCMD_POST_EQ_BAND0_CUTOFFFREQUENCY) {
                    tempValuePostEq0Cut = valueToByteArray((float)value);
                } else if (id == DroidAudioEffect::DPE_SUBCMD_POST_EQ_BAND0_GAIN) {
                    tempValuePostEq0Gain = valueToByteArray((float)value);
                } else if (id == DroidAudioEffect::DPE_SUBCMD_POST_EQ_BAND0) {
                    tempValuePostEq0En = valueToByteArray(value);
                }
                tempValue_postEq0.insert(tempValue_postEq0.end(), tempValuePostEq0En.begin(), tempValuePostEq0En.end());
                tempValue_postEq0.insert(tempValue_postEq0.end(), tempValuePostEq0Cut.begin(), tempValuePostEq0Cut.end());
                tempValue_postEq0.insert(tempValue_postEq0.end(), tempValuePostEq0Gain.begin(), tempValuePostEq0Gain.end());
                setParameter(tempParam_postEq0, tempValue_postEq0);
                tempParam_postEq0[4] = 1; //channel 1
                setParameter(tempParam_postEq0, tempValue_postEq0);
                setDpeToDb(id, value);
            }
            break;

        // post eq band 1
        case DroidAudioEffect::DPE_SUBCMD_POST_EQ_BAND1:
        case DroidAudioEffect::DPE_SUBCMD_POST_EQ_BAND1_CUTOFFFREQUENCY:
        case DroidAudioEffect::DPE_SUBCMD_POST_EQ_BAND1_GAIN: {
                vector<uint8_t> tempParamPostEq1Cmd = valueToByteArray(DroidAudioEffect::DPE_CMD_POST_EQ_BAND);
                vector<uint8_t> tempParamPostEq1Ch = valueToByteArray(0); //channel 0
                vector<uint8_t> tempParamPostEq1Band = valueToByteArray(1); // band 1
                vector<uint8_t> tempParam_postEq1;
                tempParam_postEq1.insert(tempParam_postEq1.end(), tempParamPostEq1Cmd.begin(), tempParamPostEq1Cmd.end());
                tempParam_postEq1.insert(tempParam_postEq1.end(), tempParamPostEq1Ch.begin(), tempParamPostEq1Ch.end());
                tempParam_postEq1.insert(tempParam_postEq1.end(), tempParamPostEq1Band.begin(), tempParamPostEq1Band.end());
                vector<uint8_t> tempValuePostEq1En = valueToByteArray(getDpeParam(DroidAudioEffect::DPE_SUBCMD_POST_EQ_BAND1));
                vector<uint8_t> tempValuePostEq1Cut = valueToByteArray((float)getDpeParam(DroidAudioEffect::DPE_SUBCMD_POST_EQ_BAND1_CUTOFFFREQUENCY));
                vector<uint8_t> tempValuePostEq1Gain = valueToByteArray((float)getDpeParam(DroidAudioEffect::DPE_SUBCMD_POST_EQ_BAND1_GAIN));
                //set
                vector<uint8_t> tempValue_postEq1;
                if (id == DroidAudioEffect::DPE_SUBCMD_POST_EQ_BAND1_CUTOFFFREQUENCY) {
                    tempValuePostEq1Cut = valueToByteArray((float)value);
                } else if (id == DroidAudioEffect::DPE_SUBCMD_POST_EQ_BAND1_GAIN) {
                    tempValuePostEq1Gain = valueToByteArray((float)value);
                } else if (id == DroidAudioEffect::DPE_SUBCMD_POST_EQ_BAND1) {
                    tempValuePostEq1En = valueToByteArray(value);
                }
                tempValue_postEq1.insert(tempValue_postEq1.end(), tempValuePostEq1En.begin(), tempValuePostEq1En.end());
                tempValue_postEq1.insert(tempValue_postEq1.end(), tempValuePostEq1Cut.begin(), tempValuePostEq1Cut.end());
                tempValue_postEq1.insert(tempValue_postEq1.end(), tempValuePostEq1Gain.begin(), tempValuePostEq1Gain.end());
                setParameter(tempParam_postEq1, tempValue_postEq1);
                tempParam_postEq1[4] = 1; //channel 1
                setParameter(tempParam_postEq1, tempValue_postEq1);
                setDpeToDb(id, value);
            }
            break;

        // post eq band 2
        case DroidAudioEffect::DPE_SUBCMD_POST_EQ_BAND2:
        case DroidAudioEffect::DPE_SUBCMD_POST_EQ_BAND2_CUTOFFFREQUENCY:
        case DroidAudioEffect::DPE_SUBCMD_POST_EQ_BAND2_GAIN: {
                vector<uint8_t> tempParamPostEq2Cmd = valueToByteArray(DroidAudioEffect::DPE_CMD_POST_EQ_BAND);
                vector<uint8_t> tempParamPostEq2Ch = valueToByteArray(0); //channel 0
                vector<uint8_t> tempParamPostEq2Band = valueToByteArray(2); // band 2
                vector<uint8_t> tempParam_postEq2;
                tempParam_postEq2.insert(tempParam_postEq2.end(), tempParamPostEq2Cmd.begin(), tempParamPostEq2Cmd.end());
                tempParam_postEq2.insert(tempParam_postEq2.end(), tempParamPostEq2Ch.begin(), tempParamPostEq2Ch.end());
                tempParam_postEq2.insert(tempParam_postEq2.end(), tempParamPostEq2Band.begin(), tempParamPostEq2Band.end());
                vector<uint8_t> tempValuePostEq2En = valueToByteArray(getDpeParam(DroidAudioEffect::DPE_SUBCMD_POST_EQ_BAND2));
                vector<uint8_t> tempValuePostEq2Cut = valueToByteArray((float)getDpeParam(DroidAudioEffect::DPE_SUBCMD_POST_EQ_BAND2_CUTOFFFREQUENCY));
                vector<uint8_t> tempValuePostEq2Gain = valueToByteArray((float)getDpeParam(DroidAudioEffect::DPE_SUBCMD_POST_EQ_BAND2_GAIN));

                //set
                if (id == DroidAudioEffect::DPE_SUBCMD_POST_EQ_BAND2_CUTOFFFREQUENCY) {
                    tempValuePostEq2Cut = valueToByteArray((float)value);
                } else if (id == DroidAudioEffect::DPE_SUBCMD_POST_EQ_BAND2_GAIN) {
                    tempValuePostEq2Gain = valueToByteArray((float)value);
                } else if (id == DroidAudioEffect::DPE_SUBCMD_POST_EQ_BAND2) {
                    tempValuePostEq2En = valueToByteArray(value);
                }
                vector<uint8_t> tempValue_postEq2;
                tempValue_postEq2.insert(tempValue_postEq2.end(), tempValuePostEq2En.begin(), tempValuePostEq2En.end());
                tempValue_postEq2.insert(tempValue_postEq2.end(), tempValuePostEq2Cut.begin(), tempValuePostEq2Cut.end());
                tempValue_postEq2.insert(tempValue_postEq2.end(), tempValuePostEq2Gain.begin(), tempValuePostEq2Gain.end());
                setParameter(tempParam_postEq2, tempValue_postEq2);
                tempParam_postEq2[4] = 1; //channel 1
                setParameter(tempParam_postEq2, tempValue_postEq2);
                setDpeToDb(id, value);
            }
            break;

        //mbc band 0
        case DroidAudioEffect::DPE_SUBCMD_MBC_BAND0:
        case DroidAudioEffect::DPE_SUBCMD_MBC_BAND0_CUTOFFFREQUENCY:
        case DroidAudioEffect::DPE_SUBCMD_MBC_BAND0_ATTACKTIME:
        case DroidAudioEffect::DPE_SUBCMD_MBC_BAND0_RELEASETIME:
        case DroidAudioEffect::DPE_SUBCMD_MBC_BAND0_RATIO:
        case DroidAudioEffect::DPE_SUBCMD_MBC_BAND0_THRESHOLD:
        case DroidAudioEffect::DPE_SUBCMD_MBC_BAND0_KNEEWIDTH:
        case DroidAudioEffect::DPE_SUBCMD_MBC_BAND0_NOISEGATETHRESHOLD:
        case DroidAudioEffect::DPE_SUBCMD_MBC_BAND0_EXPANDERRATIO:
        case DroidAudioEffect::DPE_SUBCMD_MBC_BAND0_PREGAIN:
        case DroidAudioEffect::DPE_SUBCMD_MBC_BAND0_POSTGAIN: {
                vector<uint8_t> tempParamMbc0Cmd = valueToByteArray(DroidAudioEffect::DPE_CMD_MBC_BAND);
                vector<uint8_t> tempParamMbc0Ch = valueToByteArray(0); //channel 0
                vector<uint8_t> tempParamMbc0Band = valueToByteArray(0); // band 0
                vector<uint8_t> tempParam_mbc0;
                tempParam_mbc0.insert(tempParam_mbc0.end(), tempParamMbc0Cmd.begin(), tempParamMbc0Cmd.end());
                tempParam_mbc0.insert(tempParam_mbc0.end(), tempParamMbc0Ch.begin(), tempParamMbc0Ch.end());
                tempParam_mbc0.insert(tempParam_mbc0.end(), tempParamMbc0Band.begin(), tempParamMbc0Band.end());
                vector<uint8_t> tempValueMbc0En = valueToByteArray(getDpeParam(DroidAudioEffect::DPE_SUBCMD_MBC_BAND0));
                vector<uint8_t> tempValueMbc0Cut = valueToByteArray((float)getDpeParam(DroidAudioEffect::DPE_SUBCMD_MBC_BAND0_CUTOFFFREQUENCY));
                vector<uint8_t> tempValueMbc0Att = valueToByteArray((float)getDpeParam(DroidAudioEffect::DPE_SUBCMD_MBC_BAND0_ATTACKTIME));
                vector<uint8_t> tempValueMbc0Relea = valueToByteArray((float)getDpeParam(DroidAudioEffect::DPE_SUBCMD_MBC_BAND0_RELEASETIME));
                vector<uint8_t> tempValueMbc0Ratio = valueToByteArray((float)getDpeParam(DroidAudioEffect::DPE_SUBCMD_MBC_BAND0_RATIO));
                vector<uint8_t> tempValueMbc0Thre = valueToByteArray((float)getDpeParam(DroidAudioEffect::DPE_SUBCMD_MBC_BAND0_THRESHOLD));
                vector<uint8_t> tempValueMbc0Knee = valueToByteArray((float)getDpeParam(DroidAudioEffect::DPE_SUBCMD_MBC_BAND0_KNEEWIDTH));
                vector<uint8_t> tempValueMbc0Noise = valueToByteArray((float)getDpeParam(DroidAudioEffect::DPE_SUBCMD_MBC_BAND0_NOISEGATETHRESHOLD));
                vector<uint8_t> tempValueMbc0Exp = valueToByteArray((float)getDpeParam(DroidAudioEffect::DPE_SUBCMD_MBC_BAND0_EXPANDERRATIO));
                vector<uint8_t> tempValueMbc0Pre = valueToByteArray((float)getDpeParam(DroidAudioEffect::DPE_SUBCMD_MBC_BAND0_PREGAIN));
                vector<uint8_t> tempValueMbc0Post = valueToByteArray((float)getDpeParam(DroidAudioEffect::DPE_SUBCMD_MBC_BAND0_POSTGAIN));
                //set
                if (id == DroidAudioEffect::DPE_SUBCMD_MBC_BAND0_CUTOFFFREQUENCY) {
                    tempValueMbc0Cut = valueToByteArray((float)value);
                } else if (id == DroidAudioEffect::DPE_SUBCMD_MBC_BAND0_ATTACKTIME) {
                    tempValueMbc0Att = valueToByteArray((float)value);
                } else if (id == DroidAudioEffect::DPE_SUBCMD_MBC_BAND0_RELEASETIME) {
                    tempValueMbc0Relea = valueToByteArray((float)value);
                } else if (id == DroidAudioEffect::DPE_SUBCMD_MBC_BAND0_RATIO) {
                    tempValueMbc0Ratio = valueToByteArray((float)value);
                } else if (id == DroidAudioEffect::DPE_SUBCMD_MBC_BAND0_THRESHOLD) {
                    tempValueMbc0Thre = valueToByteArray((float)value);
                } else if (id == DroidAudioEffect::DPE_SUBCMD_MBC_BAND0_KNEEWIDTH) {
                    tempValueMbc0Knee = valueToByteArray((float)value);
                } else if (id == DroidAudioEffect::DPE_SUBCMD_MBC_BAND0_NOISEGATETHRESHOLD) {
                    tempValueMbc0Noise = valueToByteArray((float)value);
                } else if (id == DroidAudioEffect::DPE_SUBCMD_MBC_BAND0_EXPANDERRATIO) {
                    tempValueMbc0Exp = valueToByteArray((float)value);
                } else if (id == DroidAudioEffect::DPE_SUBCMD_MBC_BAND0_PREGAIN) {
                    tempValueMbc0Pre = valueToByteArray((float)value);
                } else if (id == DroidAudioEffect::DPE_SUBCMD_MBC_BAND0_POSTGAIN) {
                    tempValueMbc0Post = valueToByteArray((float)value);
                } else if (id == DroidAudioEffect::DPE_SUBCMD_MBC_BAND0) {
                    tempValueMbc0En = valueToByteArray(value);
                }
                vector<uint8_t> tempValue_mbc0;
                tempValue_mbc0.insert(tempValue_mbc0.end(), tempValueMbc0En.begin(), tempValueMbc0En.end());
                tempValue_mbc0.insert(tempValue_mbc0.end(), tempValueMbc0Cut.begin(), tempValueMbc0Cut.end());
                tempValue_mbc0.insert(tempValue_mbc0.end(), tempValueMbc0Att.begin(), tempValueMbc0Att.end());
                tempValue_mbc0.insert(tempValue_mbc0.end(), tempValueMbc0Relea.begin(), tempValueMbc0Relea.end());
                tempValue_mbc0.insert(tempValue_mbc0.end(), tempValueMbc0Ratio.begin(), tempValueMbc0Ratio.end());
                tempValue_mbc0.insert(tempValue_mbc0.end(), tempValueMbc0Thre.begin(), tempValueMbc0Thre.end());
                tempValue_mbc0.insert(tempValue_mbc0.end(), tempValueMbc0Knee.begin(), tempValueMbc0Knee.end());
                tempValue_mbc0.insert(tempValue_mbc0.end(), tempValueMbc0Noise.begin(), tempValueMbc0Noise.end());
                tempValue_mbc0.insert(tempValue_mbc0.end(), tempValueMbc0Exp.begin(), tempValueMbc0Exp.end());
                tempValue_mbc0.insert(tempValue_mbc0.end(), tempValueMbc0Pre.begin(), tempValueMbc0Pre.end());
                tempValue_mbc0.insert(tempValue_mbc0.end(), tempValueMbc0Post.begin(), tempValueMbc0Post.end());
                setParameter(tempParam_mbc0, tempValue_mbc0);
                tempParam_mbc0[4] = 1; //channel 1
                setParameter(tempParam_mbc0, tempValue_mbc0);
                setDpeToDb(id, value);
            }
            break;

        //mbc band 1
        case DroidAudioEffect::DPE_SUBCMD_MBC_BAND1:
        case DroidAudioEffect::DPE_SUBCMD_MBC_BAND1_CUTOFFFREQUENCY:
        case DroidAudioEffect::DPE_SUBCMD_MBC_BAND1_ATTACKTIME:
        case DroidAudioEffect::DPE_SUBCMD_MBC_BAND1_RELEASETIME:
        case DroidAudioEffect::DPE_SUBCMD_MBC_BAND1_RATIO:
        case DroidAudioEffect::DPE_SUBCMD_MBC_BAND1_THRESHOLD:
        case DroidAudioEffect::DPE_SUBCMD_MBC_BAND1_KNEEWIDTH:
        case DroidAudioEffect::DPE_SUBCMD_MBC_BAND1_NOISEGATETHRESHOLD:
        case DroidAudioEffect::DPE_SUBCMD_MBC_BAND1_EXPANDERRATIO:
        case DroidAudioEffect::DPE_SUBCMD_MBC_BAND1_PREGAIN:
        case DroidAudioEffect::DPE_SUBCMD_MBC_BAND1_POSTGAIN: {
                vector<uint8_t> tempParamMbc1Cmd = valueToByteArray(DroidAudioEffect::DPE_CMD_MBC_BAND);
                vector<uint8_t> tempParamMbc1Ch = valueToByteArray(0); //channel 0
                vector<uint8_t> tempParamMbc1Band = valueToByteArray(1); // band 1
                vector<uint8_t> tempParam_mbc1;
                tempParam_mbc1.insert(tempParam_mbc1.end(), tempParamMbc1Cmd.begin(), tempParamMbc1Cmd.end());
                tempParam_mbc1.insert(tempParam_mbc1.end(), tempParamMbc1Ch.begin(), tempParamMbc1Ch.end());
                tempParam_mbc1.insert(tempParam_mbc1.end(), tempParamMbc1Band.begin(), tempParamMbc1Band.end());

                vector<uint8_t> tempValueMbc1En = valueToByteArray(getDpeParam(DroidAudioEffect::DPE_SUBCMD_MBC_BAND1));
                vector<uint8_t> tempValueMbc1Cut = valueToByteArray((float)getDpeParam(DroidAudioEffect::DPE_SUBCMD_MBC_BAND1_CUTOFFFREQUENCY));
                vector<uint8_t> tempValueMbc1Att = valueToByteArray((float)getDpeParam(DroidAudioEffect::DPE_SUBCMD_MBC_BAND1_ATTACKTIME));
                vector<uint8_t> tempValueMbc1Relea = valueToByteArray((float)getDpeParam(DroidAudioEffect::DPE_SUBCMD_MBC_BAND1_RELEASETIME));
                vector<uint8_t> tempValueMbc1Ratio = valueToByteArray((float)getDpeParam(DroidAudioEffect::DPE_SUBCMD_MBC_BAND1_RATIO));
                vector<uint8_t> tempValueMbc1Thre = valueToByteArray((float)getDpeParam(DroidAudioEffect::DPE_SUBCMD_MBC_BAND1_THRESHOLD));
                vector<uint8_t> tempValueMbc1Knee = valueToByteArray((float)getDpeParam(DroidAudioEffect::DPE_SUBCMD_MBC_BAND1_KNEEWIDTH));
                vector<uint8_t> tempValueMbc1Noise = valueToByteArray((float)getDpeParam(DroidAudioEffect::DPE_SUBCMD_MBC_BAND1_NOISEGATETHRESHOLD));
                vector<uint8_t> tempValueMbc1Exp = valueToByteArray((float)getDpeParam(DroidAudioEffect::DPE_SUBCMD_MBC_BAND1_EXPANDERRATIO));
                vector<uint8_t> tempValueMbc1Pre = valueToByteArray((float)getDpeParam(DroidAudioEffect::DPE_SUBCMD_MBC_BAND1_PREGAIN));
                vector<uint8_t> tempValueMbc1Post = valueToByteArray((float)getDpeParam(DroidAudioEffect::DPE_SUBCMD_MBC_BAND1_POSTGAIN));
                //set
                if (id == DroidAudioEffect::DPE_SUBCMD_MBC_BAND1_CUTOFFFREQUENCY) {
                    tempValueMbc1Cut = valueToByteArray((float)value);
                } else if (id == DroidAudioEffect::DPE_SUBCMD_MBC_BAND1_ATTACKTIME) {
                    tempValueMbc1Att = valueToByteArray((float)value);
                } else if (id == DroidAudioEffect::DPE_SUBCMD_MBC_BAND1_RELEASETIME) {
                    tempValueMbc1Relea = valueToByteArray((float)value);
                } else if (id == DroidAudioEffect::DPE_SUBCMD_MBC_BAND1_RATIO) {
                    tempValueMbc1Ratio = valueToByteArray((float)value);
                } else if (id == DroidAudioEffect::DPE_SUBCMD_MBC_BAND1_THRESHOLD) {
                    tempValueMbc1Thre = valueToByteArray((float)value);
                } else if (id == DroidAudioEffect::DPE_SUBCMD_MBC_BAND1_KNEEWIDTH) {
                    tempValueMbc1Knee = valueToByteArray((float)value);
                } else if (id == DroidAudioEffect::DPE_SUBCMD_MBC_BAND1_NOISEGATETHRESHOLD) {
                    tempValueMbc1Noise = valueToByteArray((float)value);
                } else if (id == DroidAudioEffect::DPE_SUBCMD_MBC_BAND1_EXPANDERRATIO) {
                    tempValueMbc1Exp = valueToByteArray((float)value);
                } else if (id == DroidAudioEffect::DPE_SUBCMD_MBC_BAND1_PREGAIN) {
                    tempValueMbc1Pre = valueToByteArray((float)value);
                } else if (id == DroidAudioEffect::DPE_SUBCMD_MBC_BAND1_POSTGAIN) {
                    tempValueMbc1Post = valueToByteArray((float)value);
                } else if (id == DroidAudioEffect::DPE_SUBCMD_MBC_BAND1) {
                    tempValueMbc1En = valueToByteArray(value);
                }
                vector<uint8_t> tempValue_mbc1;
                tempValue_mbc1.insert(tempValue_mbc1.end(), tempValueMbc1En.begin(), tempValueMbc1En.end());
                tempValue_mbc1.insert(tempValue_mbc1.end(), tempValueMbc1Cut.begin(), tempValueMbc1Cut.end());
                tempValue_mbc1.insert(tempValue_mbc1.end(), tempValueMbc1Att.begin(), tempValueMbc1Att.end());
                tempValue_mbc1.insert(tempValue_mbc1.end(), tempValueMbc1Relea.begin(), tempValueMbc1Relea.end());
                tempValue_mbc1.insert(tempValue_mbc1.end(), tempValueMbc1Ratio.begin(), tempValueMbc1Ratio.end());
                tempValue_mbc1.insert(tempValue_mbc1.end(), tempValueMbc1Thre.begin(), tempValueMbc1Thre.end());
                tempValue_mbc1.insert(tempValue_mbc1.end(), tempValueMbc1Knee.begin(), tempValueMbc1Knee.end());
                tempValue_mbc1.insert(tempValue_mbc1.end(), tempValueMbc1Noise.begin(), tempValueMbc1Noise.end());
                tempValue_mbc1.insert(tempValue_mbc1.end(), tempValueMbc1Exp.begin(), tempValueMbc1Exp.end());
                tempValue_mbc1.insert(tempValue_mbc1.end(), tempValueMbc1Pre.begin(), tempValueMbc1Pre.end());
                tempValue_mbc1.insert(tempValue_mbc1.end(), tempValueMbc1Post.begin(), tempValueMbc1Post.end());
                setParameter(tempParam_mbc1, tempValue_mbc1);
                tempParam_mbc1[4] = 1; //channel 1
                setParameter(tempParam_mbc1, tempValue_mbc1);
                setDpeToDb(id, value);
            }
            break;

        // mbc band 2
        case DroidAudioEffect::DPE_SUBCMD_MBC_BAND2:
        case DroidAudioEffect::DPE_SUBCMD_MBC_BAND2_CUTOFFFREQUENCY:
        case DroidAudioEffect::DPE_SUBCMD_MBC_BAND2_ATTACKTIME:
        case DroidAudioEffect::DPE_SUBCMD_MBC_BAND2_RELEASETIME:
        case DroidAudioEffect::DPE_SUBCMD_MBC_BAND2_RATIO:
        case DroidAudioEffect::DPE_SUBCMD_MBC_BAND2_THRESHOLD:
        case DroidAudioEffect::DPE_SUBCMD_MBC_BAND2_KNEEWIDTH:
        case DroidAudioEffect::DPE_SUBCMD_MBC_BAND2_NOISEGATETHRESHOLD:
        case DroidAudioEffect::DPE_SUBCMD_MBC_BAND2_EXPANDERRATIO:
        case DroidAudioEffect::DPE_SUBCMD_MBC_BAND2_PREGAIN:
        case DroidAudioEffect::DPE_SUBCMD_MBC_BAND2_POSTGAIN: {
                vector<uint8_t> tempParamMbc2Cmd = valueToByteArray(DroidAudioEffect::DPE_CMD_MBC_BAND);
                vector<uint8_t> tempParamMbc2Ch = valueToByteArray(0); //channel 0
                vector<uint8_t> tempParamMbc2Band = valueToByteArray(2); // band 2
                vector<uint8_t> tempParam_mbc2;
                tempParam_mbc2.insert(tempParam_mbc2.end(), tempParamMbc2Cmd.begin(), tempParamMbc2Cmd.end());
                tempParam_mbc2.insert(tempParam_mbc2.end(), tempParamMbc2Ch.begin(), tempParamMbc2Ch.end());
                tempParam_mbc2.insert(tempParam_mbc2.end(), tempParamMbc2Band.begin(), tempParamMbc2Band.end());
                vector<uint8_t> tempValueMbc2En = valueToByteArray(getDpeParam(DroidAudioEffect::DPE_SUBCMD_MBC_BAND2));
                vector<uint8_t> tempValueMbc2Cut = valueToByteArray((float)getDpeParam(DroidAudioEffect::DPE_SUBCMD_MBC_BAND2_CUTOFFFREQUENCY));
                vector<uint8_t> tempValueMbc2Att = valueToByteArray((float)getDpeParam(DroidAudioEffect::DPE_SUBCMD_MBC_BAND2_ATTACKTIME));
                vector<uint8_t> tempValueMbc2Relea = valueToByteArray((float)getDpeParam(DroidAudioEffect::DPE_SUBCMD_MBC_BAND2_RELEASETIME));
                vector<uint8_t> tempValueMbc2Ratio = valueToByteArray((float)getDpeParam(DroidAudioEffect::DPE_SUBCMD_MBC_BAND2_RATIO));
                vector<uint8_t> tempValueMbc2Thre = valueToByteArray((float)getDpeParam(DroidAudioEffect::DPE_SUBCMD_MBC_BAND2_THRESHOLD));
                vector<uint8_t> tempValueMbc2Knee = valueToByteArray((float)getDpeParam(DroidAudioEffect::DPE_SUBCMD_MBC_BAND2_KNEEWIDTH));
                vector<uint8_t> tempValueMbc2Noise = valueToByteArray((float)getDpeParam(DroidAudioEffect::DPE_SUBCMD_MBC_BAND2_NOISEGATETHRESHOLD));
                vector<uint8_t> tempValueMbc2Exp = valueToByteArray((float)getDpeParam(DroidAudioEffect::DPE_SUBCMD_MBC_BAND2_EXPANDERRATIO));
                vector<uint8_t> tempValueMbc2Pre = valueToByteArray((float)getDpeParam(DroidAudioEffect::DPE_SUBCMD_MBC_BAND2_PREGAIN));
                vector<uint8_t> tempValueMbc2Post = valueToByteArray((float)getDpeParam(DroidAudioEffect::DPE_SUBCMD_MBC_BAND2_POSTGAIN));
                //set
                if (id == DroidAudioEffect::DPE_SUBCMD_MBC_BAND2_CUTOFFFREQUENCY) {
                    tempValueMbc2Cut = valueToByteArray((float)value);
                } else if (id == DroidAudioEffect::DPE_SUBCMD_MBC_BAND2_ATTACKTIME) {
                    tempValueMbc2Att = valueToByteArray((float)value);
                } else if (id == DroidAudioEffect::DPE_SUBCMD_MBC_BAND2_RELEASETIME) {
                    tempValueMbc2Relea = valueToByteArray((float)value);
                } else if (id == DroidAudioEffect::DPE_SUBCMD_MBC_BAND2_RATIO) {
                    tempValueMbc2Ratio = valueToByteArray((float)value);
                } else if (id == DroidAudioEffect::DPE_SUBCMD_MBC_BAND2_THRESHOLD) {
                    tempValueMbc2Thre = valueToByteArray((float)value);
                } else if (id == DroidAudioEffect::DPE_SUBCMD_MBC_BAND2_KNEEWIDTH) {
                    tempValueMbc2Knee = valueToByteArray((float)value);
                } else if (id == DroidAudioEffect::DPE_SUBCMD_MBC_BAND2_NOISEGATETHRESHOLD) {
                    tempValueMbc2Noise = valueToByteArray((float)value);
                } else if (id == DroidAudioEffect::DPE_SUBCMD_MBC_BAND2_EXPANDERRATIO) {
                    tempValueMbc2Exp = valueToByteArray((float)value);
                } else if (id == DroidAudioEffect::DPE_SUBCMD_MBC_BAND2_PREGAIN) {
                    tempValueMbc2Pre = valueToByteArray((float)value);
                } else if (id == DroidAudioEffect::DPE_SUBCMD_MBC_BAND2_POSTGAIN) {
                    tempValueMbc2Post = valueToByteArray((float)value);
                } else if (id == DroidAudioEffect::DPE_SUBCMD_MBC_BAND2) {
                    tempValueMbc2En = valueToByteArray(value);
                }
                vector<uint8_t> tempValue_mbc2;
                tempValue_mbc2.insert(tempValue_mbc2.end(), tempValueMbc2En.begin(), tempValueMbc2En.end());
                tempValue_mbc2.insert(tempValue_mbc2.end(), tempValueMbc2Cut.begin(), tempValueMbc2Cut.end());
                tempValue_mbc2.insert(tempValue_mbc2.end(), tempValueMbc2Att.begin(), tempValueMbc2Att.end());
                tempValue_mbc2.insert(tempValue_mbc2.end(), tempValueMbc2Relea.begin(), tempValueMbc2Relea.end());
                tempValue_mbc2.insert(tempValue_mbc2.end(), tempValueMbc2Ratio.begin(), tempValueMbc2Ratio.end());
                tempValue_mbc2.insert(tempValue_mbc2.end(), tempValueMbc2Thre.begin(), tempValueMbc2Thre.end());
                tempValue_mbc2.insert(tempValue_mbc2.end(), tempValueMbc2Knee.begin(), tempValueMbc2Knee.end());
                tempValue_mbc2.insert(tempValue_mbc2.end(), tempValueMbc2Noise.begin(), tempValueMbc2Noise.end());
                tempValue_mbc2.insert(tempValue_mbc2.end(), tempValueMbc2Exp.begin(), tempValueMbc2Exp.end());
                tempValue_mbc2.insert(tempValue_mbc2.end(), tempValueMbc2Pre.begin(), tempValueMbc2Pre.end());
                tempValue_mbc2.insert(tempValue_mbc2.end(), tempValueMbc2Post.begin(), tempValueMbc2Post.end());
                setParameter(tempParam_mbc2, tempValue_mbc2);
                tempParam_mbc2[4] = 1; //channel 1
                setParameter(tempParam_mbc2, tempValue_mbc2);
                setDpeToDb(id, value);
            }
            break;
        //limiter param
        case DroidAudioEffect::DPE_CMD_LIMITER:
        case DroidAudioEffect::DPE_SUBCMD_LIMITER_ATTACKTIME:
        case DroidAudioEffect::DPE_SUBCMD_LIMITER_RELEASETIME:
        case DroidAudioEffect::DPE_SUBCMD_LIMITER_RATIO:
        case DroidAudioEffect::DPE_SUBCMD_LIMITER_THRESHOLD:
        case DroidAudioEffect::DPE_SUBCMD_LIMITER_POSTGAIN: {
                vector<uint8_t> tempParam_limiter = {DroidAudioEffect::DPE_CMD_LIMITER, 0, 0, 0, 0, 0, 0, 0};
                vector<uint8_t> tempValueLimiterIu = valueToByteArray(getDpeParam(DroidAudioEffect::DPE_CMD_LIMITER));
                vector<uint8_t> tempValueLimiterEn = valueToByteArray(getDpeParam(DroidAudioEffect::DPE_CMD_LIMITER));
                vector<uint8_t> tempValueLimiterLink = {1, 0, 0, 0};
                vector<uint8_t> tempValueLimiterAtt = valueToByteArray((float)getDpeParam(DroidAudioEffect::DPE_SUBCMD_LIMITER_ATTACKTIME));
                vector<uint8_t> tempValueLimiterRelea = valueToByteArray((float)getDpeParam(DroidAudioEffect::DPE_SUBCMD_LIMITER_RELEASETIME));
                vector<uint8_t> tempValueLimiterRatio = valueToByteArray((float)getDpeParam(DroidAudioEffect::DPE_SUBCMD_LIMITER_RATIO));
                vector<uint8_t> tempValueLimiterThre = valueToByteArray((float)getDpeParam(DroidAudioEffect::DPE_SUBCMD_LIMITER_THRESHOLD));
                vector<uint8_t> tempValueLimiterPost = valueToByteArray((float)getDpeParam(DroidAudioEffect::DPE_SUBCMD_LIMITER_POSTGAIN));
                //set
                if (id == DroidAudioEffect::DPE_SUBCMD_LIMITER_ATTACKTIME) {
                    tempValueLimiterAtt = valueToByteArray((float)value);
                } else if (id == DroidAudioEffect::DPE_SUBCMD_LIMITER_RELEASETIME) {
                    tempValueLimiterRelea = valueToByteArray((float)value);
                } else if (id == DroidAudioEffect::DPE_SUBCMD_LIMITER_RATIO) {
                    tempValueLimiterRatio = valueToByteArray((float)value);
                } else if (id == DroidAudioEffect::DPE_SUBCMD_LIMITER_THRESHOLD) {
                    tempValueLimiterThre = valueToByteArray((float)value);
                } else if (id == DroidAudioEffect::DPE_SUBCMD_LIMITER_POSTGAIN) {
                    tempValueLimiterPost = valueToByteArray((float)value);
                } else if (id == DroidAudioEffect::DPE_CMD_LIMITER) {
                    tempValueLimiterIu = valueToByteArray(value);
                    tempValueLimiterEn = valueToByteArray(value);
                }
                vector<uint8_t> tempValue_limiter;
                tempValue_limiter.insert(tempValue_limiter.end(), tempValueLimiterIu.begin(), tempValueLimiterIu.end());
                tempValue_limiter.insert(tempValue_limiter.end(), tempValueLimiterEn.begin(), tempValueLimiterEn.end());
                tempValue_limiter.insert(tempValue_limiter.end(), tempValueLimiterLink.begin(), tempValueLimiterLink.end());
                tempValue_limiter.insert(tempValue_limiter.end(), tempValueLimiterAtt.begin(), tempValueLimiterAtt.end());
                tempValue_limiter.insert(tempValue_limiter.end(), tempValueLimiterRelea.begin(), tempValueLimiterRelea.end());
                tempValue_limiter.insert(tempValue_limiter.end(), tempValueLimiterRatio.begin(), tempValueLimiterRatio.end());
                tempValue_limiter.insert(tempValue_limiter.end(), tempValueLimiterThre.begin(), tempValueLimiterThre.end());
                tempValue_limiter.insert(tempValue_limiter.end(), tempValueLimiterPost.begin(), tempValueLimiterPost.end());
                setParameter(tempParam_limiter, tempValue_limiter);
                tempParam_limiter[4] = 1; // channel 1
                setParameter(tempParam_limiter, tempValue_limiter);
                setDpeToDb(id, value);
            }
            break;
    }
    return 0;
}

const unordered_map<int32_t, const char*> AmlAudioEffectDpe::mMapDpeDbConvert = {
    {DroidAudioEffect::DPE_CMD_ENABLED,                             DB_KEY_AM_AUDIO_EFFECT_DPE_ENABLED},
    {DroidAudioEffect::DPE_CMD_INPUTGAIN,                           DB_KEY_AM_AUDIO_EFFECT_DPE_INPUTGAIN},
    {DroidAudioEffect::DPE_CMD_PRE_EQ,                              DB_KEY_AM_AUDIO_EFFECT_DPE_PRE_EQ},
    {DroidAudioEffect::DPE_CMD_MBC,                                 DB_KEY_AM_AUDIO_EFFECT_DPE_MBC},
    {DroidAudioEffect::DPE_CMD_POST_EQ,                             DB_KEY_AM_AUDIO_EFFECT_DPE_POST_EQ},
    {DroidAudioEffect::DPE_CMD_LIMITER,                             DB_KEY_AM_AUDIO_EFFECT_DPE_LIMITER},
    {DroidAudioEffect::DPE_SUBCMD_PRE_EQ_BAND0,                     DB_KEY_AM_AUDIO_EFFECT_DPE_PRE_EQ_BAND0},
    {DroidAudioEffect::DPE_SUBCMD_PRE_EQ_BAND0_CUTOFFFREQUENCY,     DB_KEY_AM_AUDIO_EFFECT_DPE_PRE_EQ_BAND0_CUTOFFFREQUENCY},
    {DroidAudioEffect::DPE_SUBCMD_PRE_EQ_BAND0_GAIN,                DB_KEY_AM_AUDIO_EFFECT_DPE_PRE_EQ_BAND0_GAIN},
    {DroidAudioEffect::DPE_SUBCMD_PRE_EQ_BAND1,                     DB_KEY_AM_AUDIO_EFFECT_DPE_PRE_EQ_BAND1},
    {DroidAudioEffect::DPE_SUBCMD_PRE_EQ_BAND1_CUTOFFFREQUENCY,     DB_KEY_AM_AUDIO_EFFECT_DPE_PRE_EQ_BAND1_CUTOFFFREQUENCY},
    {DroidAudioEffect::DPE_SUBCMD_PRE_EQ_BAND1_GAIN,                DB_KEY_AM_AUDIO_EFFECT_DPE_PRE_EQ_BAND1_GAIN},
    {DroidAudioEffect::DPE_SUBCMD_PRE_EQ_BAND2,                     DB_KEY_AM_AUDIO_EFFECT_DPE_PRE_EQ_BAND2},
    {DroidAudioEffect::DPE_SUBCMD_PRE_EQ_BAND2_CUTOFFFREQUENCY,     DB_KEY_AM_AUDIO_EFFECT_DPE_PRE_EQ_BAND2_CUTOFFFREQUENCY},
    {DroidAudioEffect::DPE_SUBCMD_PRE_EQ_BAND2_GAIN,                DB_KEY_AM_AUDIO_EFFECT_DPE_PRE_EQ_BAND2_GAIN},
    {DroidAudioEffect::DPE_SUBCMD_MBC_BAND0,                        DB_KEY_AM_AUDIO_EFFECT_DPE_MBC_BAND0},
    {DroidAudioEffect::DPE_SUBCMD_MBC_BAND0_CUTOFFFREQUENCY,        DB_KEY_AM_AUDIO_EFFECT_DPE_MBC_BAND0_CUTOFFFREQUENCY},
    {DroidAudioEffect::DPE_SUBCMD_MBC_BAND0_ATTACKTIME,             DB_KEY_AM_AUDIO_EFFECT_DPE_MBC_BAND0_ATTACKTIME},
    {DroidAudioEffect::DPE_SUBCMD_MBC_BAND0_RELEASETIME,            DB_KEY_AM_AUDIO_EFFECT_DPE_MBC_BAND0_RELEASETIME},
    {DroidAudioEffect::DPE_SUBCMD_MBC_BAND0_RATIO,                  DB_KEY_AM_AUDIO_EFFECT_DPE_MBC_BAND0_RATIO},
    {DroidAudioEffect::DPE_SUBCMD_MBC_BAND0_THRESHOLD,              DB_KEY_AM_AUDIO_EFFECT_DPE_MBC_BAND0_THRESHOLD},
    {DroidAudioEffect::DPE_SUBCMD_MBC_BAND0_KNEEWIDTH,              DB_KEY_AM_AUDIO_EFFECT_DPE_MBC_BAND0_KNEEWIDTH},
    {DroidAudioEffect::DPE_SUBCMD_MBC_BAND0_NOISEGATETHRESHOLD,     DB_KEY_AM_AUDIO_EFFECT_DPE_MBC_BAND0_NOISEGATE_THRESHOLD},
    {DroidAudioEffect::DPE_SUBCMD_MBC_BAND0_EXPANDERRATIO,          DB_KEY_AM_AUDIO_EFFECT_DPE_MBC_BAND0_EXPANDER_RATIO},
    {DroidAudioEffect::DPE_SUBCMD_MBC_BAND0_PREGAIN,                DB_KEY_AM_AUDIO_EFFECT_DPE_MBC_BAND0_PRE_GAIN},
    {DroidAudioEffect::DPE_SUBCMD_MBC_BAND0_POSTGAIN,               DB_KEY_AM_AUDIO_EFFECT_DPE_MBC_BAND0_POST_GAIN},
    {DroidAudioEffect::DPE_SUBCMD_MBC_BAND1,                        DB_KEY_AM_AUDIO_EFFECT_DPE_MBC_BAND1},
    {DroidAudioEffect::DPE_SUBCMD_MBC_BAND1_CUTOFFFREQUENCY,        DB_KEY_AM_AUDIO_EFFECT_DPE_MBC_BAND1_CUTOFFFREQUENCY},
    {DroidAudioEffect::DPE_SUBCMD_MBC_BAND1_ATTACKTIME,             DB_KEY_AM_AUDIO_EFFECT_DPE_MBC_BAND1_ATTACKTIME},
    {DroidAudioEffect::DPE_SUBCMD_MBC_BAND1_RELEASETIME,            DB_KEY_AM_AUDIO_EFFECT_DPE_MBC_BAND1_RELEASETIME},
    {DroidAudioEffect::DPE_SUBCMD_MBC_BAND1_RATIO,                  DB_KEY_AM_AUDIO_EFFECT_DPE_MBC_BAND1_RATIO},
    {DroidAudioEffect::DPE_SUBCMD_MBC_BAND1_THRESHOLD,              DB_KEY_AM_AUDIO_EFFECT_DPE_MBC_BAND1_THRESHOLD},
    {DroidAudioEffect::DPE_SUBCMD_MBC_BAND1_KNEEWIDTH,              DB_KEY_AM_AUDIO_EFFECT_DPE_MBC_BAND1_KNEEWIDTH},
    {DroidAudioEffect::DPE_SUBCMD_MBC_BAND1_NOISEGATETHRESHOLD,     DB_KEY_AM_AUDIO_EFFECT_DPE_MBC_BAND1_NOISEGATE_THRESHOLD},
    {DroidAudioEffect::DPE_SUBCMD_MBC_BAND1_EXPANDERRATIO,          DB_KEY_AM_AUDIO_EFFECT_DPE_MBC_BAND1_EXPANDER_RATIO},
    {DroidAudioEffect::DPE_SUBCMD_MBC_BAND1_PREGAIN,                DB_KEY_AM_AUDIO_EFFECT_DPE_MBC_BAND1_PRE_GAIN},
    {DroidAudioEffect::DPE_SUBCMD_MBC_BAND1_POSTGAIN,               DB_KEY_AM_AUDIO_EFFECT_DPE_MBC_BAND1_POST_GAIN},
    {DroidAudioEffect::DPE_SUBCMD_MBC_BAND2,                        DB_KEY_AM_AUDIO_EFFECT_DPE_MBC_BAND2},
    {DroidAudioEffect::DPE_SUBCMD_MBC_BAND2_CUTOFFFREQUENCY,        DB_KEY_AM_AUDIO_EFFECT_DPE_MBC_BAND2_CUTOFFFREQUENCY},
    {DroidAudioEffect::DPE_SUBCMD_MBC_BAND2_ATTACKTIME,             DB_KEY_AM_AUDIO_EFFECT_DPE_MBC_BAND2_ATTACKTIME},
    {DroidAudioEffect::DPE_SUBCMD_MBC_BAND2_RELEASETIME,            DB_KEY_AM_AUDIO_EFFECT_DPE_MBC_BAND2_RELEASETIME},
    {DroidAudioEffect::DPE_SUBCMD_MBC_BAND2_RATIO,                  DB_KEY_AM_AUDIO_EFFECT_DPE_MBC_BAND2_RATIO},
    {DroidAudioEffect::DPE_SUBCMD_MBC_BAND2_THRESHOLD,              DB_KEY_AM_AUDIO_EFFECT_DPE_MBC_BAND2_THRESHOLD},
    {DroidAudioEffect::DPE_SUBCMD_MBC_BAND2_KNEEWIDTH,              DB_KEY_AM_AUDIO_EFFECT_DPE_MBC_BAND2_KNEEWIDTH},
    {DroidAudioEffect::DPE_SUBCMD_MBC_BAND2_NOISEGATETHRESHOLD,     DB_KEY_AM_AUDIO_EFFECT_DPE_MBC_BAND2_NOISEGATE_THRESHOLD},
    {DroidAudioEffect::DPE_SUBCMD_MBC_BAND2_EXPANDERRATIO,          DB_KEY_AM_AUDIO_EFFECT_DPE_MBC_BAND2_EXPANDER_RATIO},
    {DroidAudioEffect::DPE_SUBCMD_MBC_BAND2_PREGAIN,                DB_KEY_AM_AUDIO_EFFECT_DPE_MBC_BAND2_PRE_GAIN},
    {DroidAudioEffect::DPE_SUBCMD_MBC_BAND2_POSTGAIN,               DB_KEY_AM_AUDIO_EFFECT_DPE_MBC_BAND2_POST_GAIN},
    {DroidAudioEffect::DPE_SUBCMD_POST_EQ_BAND0,                    DB_KEY_AM_AUDIO_EFFECT_DPE_POST_EQ_BAND0},
    {DroidAudioEffect::DPE_SUBCMD_POST_EQ_BAND0_CUTOFFFREQUENCY,    DB_KEY_AM_AUDIO_EFFECT_DPE_POST_EQ_BAND0_CUTOFFFREQUENCY},
    {DroidAudioEffect::DPE_SUBCMD_POST_EQ_BAND0_GAIN,               DB_KEY_AM_AUDIO_EFFECT_DPE_POST_EQ_BAND0_GAIN},
    {DroidAudioEffect::DPE_SUBCMD_POST_EQ_BAND1,                    DB_KEY_AM_AUDIO_EFFECT_DPE_POST_EQ_BAND1},
    {DroidAudioEffect::DPE_SUBCMD_POST_EQ_BAND1_CUTOFFFREQUENCY,    DB_KEY_AM_AUDIO_EFFECT_DPE_POST_EQ_BAND1_CUTOFFFREQUENCY},
    {DroidAudioEffect::DPE_SUBCMD_POST_EQ_BAND1_GAIN,               DB_KEY_AM_AUDIO_EFFECT_DPE_POST_EQ_BAND1_GAIN},
    {DroidAudioEffect::DPE_SUBCMD_POST_EQ_BAND2,                    DB_KEY_AM_AUDIO_EFFECT_DPE_POST_EQ_BAND2},
    {DroidAudioEffect::DPE_SUBCMD_POST_EQ_BAND2_CUTOFFFREQUENCY,    DB_KEY_AM_AUDIO_EFFECT_DPE_POST_EQ_BAND2_CUTOFFFREQUENCY},
    {DroidAudioEffect::DPE_SUBCMD_POST_EQ_BAND2_GAIN,               DB_KEY_AM_AUDIO_EFFECT_DPE_POST_EQ_BAND2_GAIN},
    {DroidAudioEffect::DPE_SUBCMD_LIMITER_ATTACKTIME,               DB_KEY_AM_AUDIO_EFFECT_DPE_LIMITER_ATTACKTIME},
    {DroidAudioEffect::DPE_SUBCMD_LIMITER_RELEASETIME,              DB_KEY_AM_AUDIO_EFFECT_DPE_LIMITER_RELEASETIMR},
    {DroidAudioEffect::DPE_SUBCMD_LIMITER_RATIO,                    DB_KEY_AM_AUDIO_EFFECT_DPE_LIMITER_RATIO},
    {DroidAudioEffect::DPE_SUBCMD_LIMITER_THRESHOLD,                DB_KEY_AM_AUDIO_EFFECT_DPE_LIMITER_THRESHOLD},
    {DroidAudioEffect::DPE_SUBCMD_LIMITER_POSTGAIN,                 DB_KEY_AM_AUDIO_EFFECT_DPE_LIMITER_POST_GAIN},
};

int32_t AmlAudioEffectDpe::getDpeParam(int32_t id) {
    auto it = mMapDpeDbConvert.find(id);
    if (it == mMapDpeDbConvert.end()) {
        AM_LOGE("invalid id:%d", id);
        return 0;
    }
    int32_t value = getIntFromDb(it->second);
    if (isAudioDebug()) AM_LOGD("%d(%s) = %d", id, it->second, value);
    return value;
}

int32_t AmlAudioEffectDpe::setDpeToDb(int32_t id, int32_t value) {
    auto it = mMapDpeDbConvert.find(id);
    if (it == mMapDpeDbConvert.end()) {
        AM_LOGE("invalid id:%d", id);
        return -1;
    }
    int32_t ret = putToDb(it->second, value);
    if (isAudioDebug()) AM_LOGD("%d(%s) = %d", id, it->second, value);
    return ret;
}

const char* AmlAudioEffectDap::INI_KEY_AM_AUDIO_EFFECT_DAP_VERSION = "ini_key_am_audio_effect_dap_version";
AmlAudioEffectDap::AmlAudioEffectDap(DroidAudioEffectSetting* pSetting, int32_t dapVersion) :
    AmlAudioEffect(pSetting, DroidAudioEffect::EFFECT_ID_DAP, dapVersion == AML_AUDIO_EFFECT_ID_DAP1_3_2 ? "Dap1.3.2" : "Dap2.4",
    dapVersion == AML_AUDIO_EFFECT_ID_DAP1_3_2 ?
        "3337b21d-c8e6-4bbd-8f24-698ade8491b9" : "34033483-c5e9-4ff6-8b6b-0002a5d5c51b", ""), mDapVersion(dapVersion) {
}

void AmlAudioEffectDap::initDap_1_3_2() {
    int32_t mode = getDapParam(DroidAudioEffect::DAP_CMD_EFFECT_MODE);
    if (mpSetting->firstBoot()) {
        AM_LOGI("first boot.");
        int32_t id = 0;
        //the first time, use the param from so load from ini file
        setDapParam(DroidAudioEffect::DAP_CMD_EFFECT_MODE, DroidAudioEffect::DAP_EFFECT_MODE_USER);
        for (id = DroidAudioEffect::DAP_CMD_GEQ_ENABLE; id <= DroidAudioEffect::DAP_CMD_VIRTUALIZER_ENABLE; id++)
            saveDapParam(id, getDapParamInternal(id));
        for (id = DroidAudioEffect::DAP_SUBCMD_GEQ_BAND1; id <= DroidAudioEffect::DAP_SUBCMD_GEQ_BAND5; id++)
            saveDapParam(id, getDapParamInternal(id));
    } else {
        saveDapParam(DroidAudioEffect::DAP_CMD_EFFECT_MODE, DroidAudioEffect::DAP_EFFECT_MODE_USER);
        setDapParam(DroidAudioEffect::DAP_CMD_VL_ENABLE, getDapParam(DroidAudioEffect::DAP_CMD_VL_ENABLE));
        setDapParam(DroidAudioEffect::DAP_CMD_VL_AMOUNT, getDapParam(DroidAudioEffect::DAP_CMD_VL_AMOUNT));
        setDapParam(DroidAudioEffect::DAP_CMD_DE_ENABLE, getDapParam(DroidAudioEffect::DAP_CMD_DE_ENABLE));
        setDapParam(DroidAudioEffect::DAP_CMD_DE_AMOUNT, getDapParam(DroidAudioEffect::DAP_CMD_DE_AMOUNT));
        setDapParam(DroidAudioEffect::DAP_CMD_SURROUND_BOOST, getDapParam(DroidAudioEffect::DAP_CMD_SURROUND_BOOST));
        setDapParam(DroidAudioEffect::DAP_CMD_GEQ_ENABLE, getDapParam(DroidAudioEffect::DAP_CMD_GEQ_ENABLE));
        setDapParam(DroidAudioEffect::DAP_SUBCMD_GEQ_BAND1, getDapParam(DroidAudioEffect::DAP_SUBCMD_GEQ_BAND1));
        saveDapParam(DroidAudioEffect::DAP_CMD_EFFECT_MODE, mode);
    }
    setDapParam(DroidAudioEffect::DAP_CMD_EFFECT_MODE, mode);
    AM_LOGI("init_effect success");
}

void AmlAudioEffectDap::initDap_2_4() {
    int32_t value = 0;
    vector<uint8_t> tempValue(2);
    vector<uint8_t> tempValue2(6);
    if (mpSetting->firstBoot()) {
        AM_LOGI("first boot.");
        int32_t mode = getDapParamInternal(DroidAudioEffect::DAP_CMD_2_4_PROFILE);
        AM_LOGI("PROFILE first boot init mode:%d", mode);
        saveDbDap24Param(DroidAudioEffect::DAP_CMD_2_4_PROFILE, mode);
        for (int32_t i = DroidAudioEffect::DAP_2_4_PROFILE_MIN; i < DroidAudioEffect::DAP_2_4_PROFILE_MAX; i++) {
            setParameter(DroidAudioEffect::DAP_CMD_2_4_PROFILE - DroidAudioEffect::DAP_CMD_2_4_BASE_VALUE, i);
        }
        setParameter(DroidAudioEffect::DAP_CMD_2_4_PROFILE - DroidAudioEffect::DAP_CMD_2_4_BASE_VALUE, mode);
    } else {
        value = getDbDap24Param(DroidAudioEffect::DAP_CMD_2_4_PROFILE);
        if (value < 0) {
            value = getDapParamInternal(DroidAudioEffect::DAP_CMD_2_4_PROFILE);
            saveDbDap24Param(DroidAudioEffect::DAP_CMD_2_4_PROFILE, value);
        }
        AM_LOGI("PROFILE init value:%d", value);
        setParameter(DroidAudioEffect::DAP_CMD_2_4_PROFILE - DroidAudioEffect::DAP_CMD_2_4_BASE_VALUE, value);

        if (value == DroidAudioEffect::DAP_2_4_PROFILE_USER_SELECTABLE) {
            tempValue[0] = (uint8_t)getDbDap24Param(DroidAudioEffect::DAP_CMD_2_4_SURROUND_VIRTUALIZER);
            //AM_LOGI("SURROUND_VIRTUALIZER init value: ", tempValue[0]);
            tempValue[1] = (uint8_t)getDbDap24Param(DroidAudioEffect::DAP_SUBCMD_2_4_SURROUND_VIRTUALIZER_BOOST);
            //AM_LOGI("SURROUND_VIRTUALIZER_BOOST init value: ", tempValue[1]);
            setParameter(DroidAudioEffect::DAP_CMD_2_4_SURROUND_VIRTUALIZER - DroidAudioEffect::DAP_CMD_2_4_BASE_VALUE, tempValue);

            value = getDbDap24Param(DroidAudioEffect::DAP_CMD_2_4_DIALOGUE_ENHANCER);
            setParameter(DroidAudioEffect::DAP_CMD_2_4_DIALOGUE_ENHANCER - DroidAudioEffect::DAP_CMD_2_4_BASE_VALUE, value);

            tempValue2[0] = (uint8_t)getDbDap24Param(DroidAudioEffect::DAP_CMD_2_4_BASS_ENHANCER);
            //AM_LOGI("BASS_ENHANCER init value: ", tempValue2[0]);
            int32_t tempInt = getDbDap24Param(DroidAudioEffect::DAP_SUBCMD_2_4_BASS_ENHANCER_BOOST);
            tempValue2[1] = tempInt & 0xff;
            tempValue2[2] = (tempInt >> 8) & 0xff;
            //AM_LOGI("BASS_ENHANCER_BOOST init value: ", enboInt);
            int32_t tempInt2 = (getDbDap24Param(DroidAudioEffect::DAP_SUBCMD_2_4_BASS_ENHANCER_CUTOFF_100) + getDbDap24Param(DroidAudioEffect::DAP_SUBCMD_2_4_BASS_ENHANCER_CUTOFF_1));
            tempValue2[3] = tempInt2 & 0xff;
            tempValue2[4] = (tempInt2 >> 8) & 0xff;
            //AM_LOGI("BASS_ENHANCER_CUTOFF init value: ", encuInt);
            tempValue2[5] = (uint8_t)getDbDap24Param(DroidAudioEffect::DAP_SUBCMD_2_4_BASS_ENHANCER_WIDTH);
            //AM_LOGI("BASS_ENHANCER_WIDTH init value: ", tempValue2[5]);
            setParameter(DroidAudioEffect::DAP_CMD_2_4_BASS_ENHANCER - DroidAudioEffect::DAP_CMD_2_4_BASE_VALUE, tempValue2);

            value = getDbDap24Param(DroidAudioEffect::DAP_CMD_2_4_MI_STEERING);
            //AM_LOGI("MI_STEERING init value: ", value);
            setParameter(DroidAudioEffect::DAP_CMD_2_4_MI_STEERING - DroidAudioEffect::DAP_CMD_2_4_BASE_VALUE, value);

            value = getDbDap24Param(DroidAudioEffect::DAP_CMD_2_4_SURROUND_DECODER_ENABLE);
            //AM_LOGI("SURROUND_DECODER_ENABLE init value: ", value);
            setParameter(DroidAudioEffect::DAP_CMD_2_4_SURROUND_DECODER_ENABLE - DroidAudioEffect::DAP_CMD_2_4_BASE_VALUE, value);

            tempValue[0] = (uint8_t)getDbDap24Param(DroidAudioEffect::DAP_CMD_2_4_LEVELER);
            //AM_LOGI("LEVELER init value: ", tempValue[0]);
            tempValue[1] = (uint8_t)getDbDap24Param(DroidAudioEffect::DAP_SUBCMD_2_4_LEVELER_AMOUNT);
            //AM_LOGI("LEVELER_AMOUNT init value: ", tempValue[1]);
            setParameter(DroidAudioEffect::DAP_CMD_2_4_LEVELER - DroidAudioEffect::DAP_CMD_2_4_BASE_VALUE, tempValue);
        }
    }
}

int32_t AmlAudioEffectDap::init() {
    AM_LOGI("dap version:%d", mDapVersion);
    if (mDapVersion == AML_AUDIO_EFFECT_ID_DAP1_3_2) {
        initDap_1_3_2();
        AM_LOGI("init_effect DAP1.3.2 success");
    } else if (mDapVersion == AML_AUDIO_EFFECT_ID_DAP2_4){
        initDap_2_4();
        AM_LOGI("init_effect DAP2.4 success");
    }
    return 0;
}

int32_t AmlAudioEffectDap::setDapEnable(int32_t enable) {
    if (isAudioDebug()) AM_LOGD("enable:%d", enable);
    return setParameter(DroidAudioEffect::DAP_CMD_2_4_ENABLE, enable);
}

int32_t AmlAudioEffectDap::getDapEnable() {
    int32_t enable = getParameter(DroidAudioEffect::DAP_CMD_2_4_ENABLE);
    if (isAudioDebug()) AM_LOGD("enable:%d", enable);
    return enable;
}

int32_t AmlAudioEffectDap::getDapParamInternal(int32_t id) {
    int32_t value = 0;
    switch (id) {
        case DroidAudioEffect::DAP_SUBCMD_GEQ_BAND1:
        case DroidAudioEffect::DAP_SUBCMD_GEQ_BAND2:
        case DroidAudioEffect::DAP_SUBCMD_GEQ_BAND3:
        case DroidAudioEffect::DAP_SUBCMD_GEQ_BAND4:
        case DroidAudioEffect::DAP_SUBCMD_GEQ_BAND5: {
                vector<uint8_t> tempValue(5);
                getParameter(DroidAudioEffect::DAP_CMD_GEQ_GAINS, tempValue);
                value = tempValue[id - DroidAudioEffect::DAP_SUBCMD_GEQ_BAND1];
            }
            break;
        case DroidAudioEffect::DAP_CMD_2_4_PROFILE:
            value = getParameter(id - DroidAudioEffect::DAP_CMD_2_4_BASE_VALUE);
            if (value < DroidAudioEffect::DAP_2_4_PROFILE_MIN || value > DroidAudioEffect::DAP_2_4_PROFILE_MAX) {
                AM_LOGW("DAP 2.4 profile mode:%d invalid, set the default:%d. id:%d", value,
                        DroidAudioEffect::DAP_2_4_PROFILE_MIN, id);
                value = DroidAudioEffect::DAP_2_4_PROFILE_MIN;
            };
            AM_LOGD("DAP 2.4 profile mode:%d", value);
            break;
        case DroidAudioEffect::DAP_CMD_2_4_MI_STEERING:
            value = getParameter(DroidAudioEffect::DAP_CMD_2_4_MI_STEERING - DroidAudioEffect::DAP_CMD_2_4_BASE_VALUE);
            break;
        default:
            value = getParameter(id);
            AM_LOGD("value:%d", value);
            break;
    }
    return value;
}

int32_t AmlAudioEffectDap::setDapParam(int32_t id, int32_t value) {
    if (isAudioDebug()) AM_LOGD("id:%d, value:%d", id, value);
    vector<uint8_t> fiveband(5);
    int32_t valueHal = 0;
    switch (id) {
        case DroidAudioEffect::DAP_CMD_ENABLE:
        case DroidAudioEffect::DAP_CMD_EFFECT_MODE:
        case DroidAudioEffect::DAP_CMD_VL_ENABLE:
        case DroidAudioEffect::DAP_CMD_VL_AMOUNT:
        case DroidAudioEffect::DAP_CMD_DE_ENABLE:
        case DroidAudioEffect::DAP_CMD_DE_AMOUNT:
        case DroidAudioEffect::DAP_CMD_POST_GAIN:
        case DroidAudioEffect::DAP_CMD_GEQ_ENABLE:
        case DroidAudioEffect::DAP_CMD_SURROUND_ENABLE:
        case DroidAudioEffect::DAP_CMD_SURROUND_BOOST:
            setParameter(id, value);
            saveDapParam(id, value);
            break;
        case DroidAudioEffect::DAP_CMD_VIRTUALIZER_ENABLE:
            /*
            if (value == DAP_SURROUND_SPEAKER)
                setParameter(id, DAP_CPDP_OUTPUT_2_SPEAKER);
            else if (value == DAP_SURROUND_HEADPHONE)
                setParameter(id, DAP_CPDP_OUTPUT_2_HEADPHONE);
            */
            setParameter(id, value);
            saveDapParam(id, value);
            break;
        case DroidAudioEffect::DAP_SUBCMD_GEQ_BAND1:
        case DroidAudioEffect::DAP_SUBCMD_GEQ_BAND2:
        case DroidAudioEffect::DAP_SUBCMD_GEQ_BAND3:
        case DroidAudioEffect::DAP_SUBCMD_GEQ_BAND4:
        case DroidAudioEffect::DAP_SUBCMD_GEQ_BAND5:
            fiveband[0] = (uint8_t)getDapParam(DroidAudioEffect::DAP_SUBCMD_GEQ_BAND1);
            fiveband[1] = (uint8_t)getDapParam(DroidAudioEffect::DAP_SUBCMD_GEQ_BAND2);
            fiveband[2] = (uint8_t)getDapParam(DroidAudioEffect::DAP_SUBCMD_GEQ_BAND3);
            fiveband[3] = (uint8_t)getDapParam(DroidAudioEffect::DAP_SUBCMD_GEQ_BAND4);
            fiveband[4] = (uint8_t)getDapParam(DroidAudioEffect::DAP_SUBCMD_GEQ_BAND5);
            fiveband[id - DroidAudioEffect::DAP_SUBCMD_GEQ_BAND1] = (uint8_t)value;
            setParameter(DroidAudioEffect::DAP_CMD_GEQ_GAINS, fiveband);
            saveDapParam(id, value);
            break;
        case DroidAudioEffect::DAP_CMD_2_4_PROFILE:
            setParameter(id - DroidAudioEffect::DAP_CMD_2_4_BASE_VALUE, value);
            AM_LOGI("PROFILE  value:%d", value);
            valueHal = value;
            if (valueHal == DroidAudioEffect::DAP_2_4_PROFILE_USER_SELECTABLE) {
                vector<uint8_t> tempValueHal(2);
                vector<uint8_t> tempValueHal2(6);
                tempValueHal[0] = (uint8_t)getDbDap24Param(DroidAudioEffect::DAP_CMD_2_4_SURROUND_VIRTUALIZER);
                tempValueHal[1] = (uint8_t)getDbDap24Param(DroidAudioEffect::DAP_SUBCMD_2_4_SURROUND_VIRTUALIZER_BOOST);
                setParameter(DroidAudioEffect::DAP_CMD_2_4_SURROUND_VIRTUALIZER - DroidAudioEffect::DAP_CMD_2_4_BASE_VALUE, tempValueHal);

                tempValueHal[0] = (uint8_t)getDbDap24Param(DroidAudioEffect::DAP_CMD_2_4_DIALOGUE_ENHANCER);
                tempValueHal[1] = (uint8_t)getDbDap24Param(DroidAudioEffect::DAP_SUBCMD_2_4_DIALOGUE_ENHANCER_AMOUNT);
                setParameter(DroidAudioEffect::DAP_CMD_2_4_DIALOGUE_ENHANCER - DroidAudioEffect::DAP_CMD_2_4_BASE_VALUE, tempValueHal);

                tempValueHal2[0] = (uint8_t)getDbDap24Param(DroidAudioEffect::DAP_CMD_2_4_BASS_ENHANCER);
                int32_t tempIntHal = getDbDap24Param(DroidAudioEffect::DAP_SUBCMD_2_4_BASS_ENHANCER_BOOST);
                tempValueHal2[1] = tempIntHal & 0xff;
                tempValueHal2[2] = (tempIntHal >> 8) & 0xff;
                int32_t tempIntHal2 = (getDbDap24Param(DroidAudioEffect::DAP_SUBCMD_2_4_BASS_ENHANCER_CUTOFF_100)
                 + getDbDap24Param(DroidAudioEffect::DAP_SUBCMD_2_4_BASS_ENHANCER_CUTOFF_1));
                tempValueHal2[3] = tempIntHal2 & 0xff;
                tempValueHal2[4] = (tempIntHal2 >> 8) & 0xff;
                tempValueHal2[5] = (uint8_t)getDbDap24Param(DroidAudioEffect::DAP_SUBCMD_2_4_BASS_ENHANCER_WIDTH);
                setParameter(DroidAudioEffect::DAP_CMD_2_4_BASS_ENHANCER - DroidAudioEffect::DAP_CMD_2_4_BASE_VALUE, tempValueHal2);

                valueHal = getDbDap24Param(DroidAudioEffect::DAP_CMD_2_4_MI_STEERING);
                setParameter(DroidAudioEffect::DAP_CMD_2_4_MI_STEERING - DroidAudioEffect::DAP_CMD_2_4_BASE_VALUE, valueHal);

                valueHal = getDbDap24Param(DroidAudioEffect::DAP_CMD_2_4_SURROUND_DECODER_ENABLE);
                setParameter(DroidAudioEffect::DAP_CMD_2_4_SURROUND_DECODER_ENABLE - DroidAudioEffect::DAP_CMD_2_4_BASE_VALUE, valueHal);

                tempValueHal[0] = (uint8_t)getDbDap24Param(DroidAudioEffect::DAP_CMD_2_4_LEVELER);
                tempValueHal[1] = (uint8_t)getDbDap24Param(DroidAudioEffect::DAP_SUBCMD_2_4_LEVELER_AMOUNT);
                setParameter(DroidAudioEffect::DAP_CMD_2_4_LEVELER - DroidAudioEffect::DAP_CMD_2_4_BASE_VALUE, tempValueHal);
            }
            saveDbDap24Param(id, value);
            break;
        case DroidAudioEffect::DAP_CMD_2_4_SURROUND_VIRTUALIZER:
        case DroidAudioEffect::DAP_SUBCMD_2_4_SURROUND_VIRTUALIZER_BOOST: {
                vector<uint8_t> tempValue(2);
                tempValue[0] = (uint8_t)getDapParam(DroidAudioEffect::DAP_CMD_2_4_SURROUND_VIRTUALIZER);
                tempValue[1] = (uint8_t)getDapParam(DroidAudioEffect::DAP_SUBCMD_2_4_SURROUND_VIRTUALIZER_BOOST);
                if (id < DroidAudioEffect::DAP_SUBCMD_2_4_BASE_VALUE) {
                    tempValue[0] = (uint8_t)value;
                } else {
                    tempValue[1] = (uint8_t)value;
                }
                setParameter(DroidAudioEffect::DAP_CMD_2_4_SURROUND_VIRTUALIZER - DroidAudioEffect::DAP_CMD_2_4_BASE_VALUE, tempValue);
                if (tempValue[0] == 1 || tempValue[0] == 2) {
                    setParameter(DroidAudioEffect::DAP_CMD_2_4_SURROUND_DECODER_ENABLE - DroidAudioEffect::DAP_CMD_2_4_BASE_VALUE, 1);
                } else {
                    setParameter(DroidAudioEffect::DAP_CMD_2_4_SURROUND_DECODER_ENABLE - DroidAudioEffect::DAP_CMD_2_4_BASE_VALUE, 0);
                }
                saveDbDap24Param(id, value);
            }
            break;
        case DroidAudioEffect::DAP_CMD_2_4_DIALOGUE_ENHANCER: {
                valueHal = getDapParam(DroidAudioEffect::DAP_CMD_2_4_DIALOGUE_ENHANCER);
                setParameter(DroidAudioEffect::DAP_CMD_2_4_DIALOGUE_ENHANCER - DroidAudioEffect::DAP_CMD_2_4_BASE_VALUE, valueHal);
                saveDbDap24Param(id, value);
            }
            break;
        case DroidAudioEffect::DAP_CMD_2_4_BASS_ENHANCER:
        case DroidAudioEffect::DAP_SUBCMD_2_4_BASS_ENHANCER_BOOST:
        case DroidAudioEffect::DAP_SUBCMD_2_4_BASS_ENHANCER_CUTOFF_100:
        case DroidAudioEffect::DAP_SUBCMD_2_4_BASS_ENHANCER_CUTOFF_1:
        case DroidAudioEffect::DAP_SUBCMD_2_4_BASS_ENHANCER_WIDTH: {
                vector<uint8_t> tempValue_2(6);
                tempValue_2[0] = (uint8_t)getDapParam(DroidAudioEffect::DAP_CMD_2_4_BASS_ENHANCER);
                int32_t tempInt = getDapParam(DroidAudioEffect::DAP_SUBCMD_2_4_BASS_ENHANCER_BOOST);
                tempValue_2[1] = tempInt & 0xff;
                tempValue_2[2] = (tempInt >> 8) & 0xff;
                int32_t tempInt2 = (getDapParam(DroidAudioEffect::DAP_SUBCMD_2_4_BASS_ENHANCER_CUTOFF_100) + getDapParam(DroidAudioEffect::DAP_SUBCMD_2_4_BASS_ENHANCER_CUTOFF_1));
                tempValue_2[3] = tempInt2 & 0xff;
                tempValue_2[4] = (tempInt2 >> 8) & 0xff;
                tempValue_2[5] = (uint8_t)getDapParam(DroidAudioEffect::DAP_SUBCMD_2_4_BASS_ENHANCER_WIDTH);
                //AM_LOGD("BASS_ENHANCER getDapParam enable:", tempValue_2[0] + ", boost:", enboInt + ", cutoff:", encuInt + ", width:", tempValue_2[5]);
                if (id < DroidAudioEffect::DAP_SUBCMD_2_4_BASE_VALUE) {
                    tempValue_2[0] = (uint8_t)value;
                } else if (id == DroidAudioEffect::DAP_SUBCMD_2_4_BASS_ENHANCER_BOOST) {
                    tempValue_2[1] = value & 0xff;
                    tempValue_2[2] = (value >> 8) & 0xff;
                } else if (id == DroidAudioEffect::DAP_SUBCMD_2_4_BASS_ENHANCER_CUTOFF_100) {
                    int32_t encu = value * 100 + getDapParam(DroidAudioEffect::DAP_SUBCMD_2_4_BASS_ENHANCER_CUTOFF_1);
                    tempValue_2[3] = encu & 0xff;
                    tempValue_2[4] = (encu >> 8) & 0xff;
                } else if (id == DroidAudioEffect::DAP_SUBCMD_2_4_BASS_ENHANCER_CUTOFF_1) {
                    int32_t encu = value  + getDapParam(DroidAudioEffect::DAP_SUBCMD_2_4_BASS_ENHANCER_CUTOFF_100);
                    tempValue_2[3] = encu & 0xff;
                    tempValue_2[4] = (encu >> 8) & 0xff;
                } else {
                    tempValue_2[5] = (uint8_t)value;
                }
                setParameter(DroidAudioEffect::DAP_CMD_2_4_BASS_ENHANCER - DroidAudioEffect::DAP_CMD_2_4_BASE_VALUE, tempValue_2);
                //AM_LOGD("BASS_ENHANCER setParameter enable:", tempValue_2[0] + ", boost:", enbo2Int + ", cutoff:", encu2Int + ", width:", tempValue_2[5]);
                saveDbDap24Param(id, value);
            }
            break;
        case DroidAudioEffect::DAP_CMD_2_4_LEVELER:
        case DroidAudioEffect::DAP_SUBCMD_2_4_LEVELER_AMOUNT: {
                vector<uint8_t> tempValue_3(2);
                tempValue_3[0] = (uint8_t)getDapParam(DroidAudioEffect::DAP_CMD_2_4_LEVELER);
                tempValue_3[1] = (uint8_t)getDapParam(DroidAudioEffect::DAP_SUBCMD_2_4_LEVELER_AMOUNT);
                if (id < DroidAudioEffect::DAP_SUBCMD_2_4_BASE_VALUE) {
                    tempValue_3[0] = (uint8_t)value;
                } else {
                    tempValue_3[1] = (uint8_t)value;
                }
                setParameter(DroidAudioEffect::DAP_CMD_2_4_LEVELER - DroidAudioEffect::DAP_CMD_2_4_BASE_VALUE, tempValue_3);
                saveDbDap24Param(id, value);
            }
            break;
        case DroidAudioEffect::DAP_CMD_2_4_MI_STEERING:
        case DroidAudioEffect::DAP_CMD_2_4_SURROUND_DECODER_ENABLE:
            setParameter(id - DroidAudioEffect::DAP_CMD_2_4_BASE_VALUE, value);
            saveDbDap24Param(id, value);
            break;
    }
    return 0;
}

int32_t AmlAudioEffectDap::saveDapParam (int32_t id, int32_t value) {
    if (isAudioDebug()) AM_LOGD("id:%d, value:%d", id, value);
    int32_t param = 0;
    int32_t dapEffectMode = getDapParamInternal(DroidAudioEffect::DAP_CMD_EFFECT_MODE);
    if ((id != DroidAudioEffect::DAP_CMD_EFFECT_MODE) && (dapEffectMode != DroidAudioEffect::DAP_EFFECT_MODE_USER)) {
        AM_LOGI("id:%d is not EFFECT_MODE or effect mode:%d not user, return.", id, dapEffectMode);
        return 0;
    }
    switch (id) {
        case DroidAudioEffect::DAP_CMD_EFFECT_MODE:
            putToDb(DB_KEY_AM_AUDIO_EFFECT_DAP_MODE, value);
            break;
        case DroidAudioEffect::DAP_CMD_VL_ENABLE:
            putToDb(DB_KEY_AM_AUDIO_EFFECT_DAP_VL_ENABLE, value);
            break;
        case DroidAudioEffect::DAP_CMD_VL_AMOUNT:
            putToDb(DB_KEY_AM_AUDIO_EFFECT_DAP_VL_AMOUNT, value);
            break;
        case DroidAudioEffect::DAP_CMD_DE_ENABLE:
            putToDb(DB_KEY_AM_AUDIO_EFFECT_DAP_DE_ENABLE, value);
            break;
        case DroidAudioEffect::DAP_CMD_DE_AMOUNT:
            putToDb(DB_KEY_AM_AUDIO_EFFECT_DAP_DE_AMOUNT, value);
            break;
        case DroidAudioEffect::DAP_CMD_SURROUND_ENABLE:
            putToDb(DB_KEY_AM_AUDIO_EFFECT_DAP_SURROUND_ENABLE, value);
            break;
        case DroidAudioEffect::DAP_CMD_SURROUND_BOOST:
            putToDb(DB_KEY_AM_AUDIO_EFFECT_DAP_SURROUND_BOOST, value);
            break;
        case DroidAudioEffect::DAP_CMD_POST_GAIN:
            putToDb(DB_KEY_AM_AUDIO_EFFECT_DAP_POST_GAIN, value);
            break;
        case DroidAudioEffect::DAP_CMD_GEQ_ENABLE:
            putToDb(DB_KEY_AM_AUDIO_EFFECT_DAP_GEQ_ENABLE, value);
            break;
        case DroidAudioEffect::DAP_SUBCMD_GEQ_BAND1:
            param = getIntFromDb(DB_KEY_AM_AUDIO_EFFECT_DAP_GEQ_ENABLE);
            if (param == DroidAudioEffect::DAP_GEQ_EFFECT_MODE_USER) {
                putToDb(DB_KEY_AM_AUDIO_EFFECT_DAP_GEQ_BAND1, value);
            }
            break;
        case DroidAudioEffect::DAP_SUBCMD_GEQ_BAND2:
            param = getIntFromDb(DB_KEY_AM_AUDIO_EFFECT_DAP_GEQ_ENABLE);
            if (param == DroidAudioEffect::DAP_GEQ_EFFECT_MODE_USER) {
                putToDb(DB_KEY_AM_AUDIO_EFFECT_DAP_GEQ_BAND2, value);
            }
            break;
        case DroidAudioEffect::DAP_SUBCMD_GEQ_BAND3:
            param = getIntFromDb(DB_KEY_AM_AUDIO_EFFECT_DAP_GEQ_ENABLE);
            if (param == DroidAudioEffect::DAP_GEQ_EFFECT_MODE_USER) {
                putToDb(DB_KEY_AM_AUDIO_EFFECT_DAP_GEQ_BAND3, value);
            }
            break;
        case DroidAudioEffect::DAP_SUBCMD_GEQ_BAND4:
            param = getIntFromDb(DB_KEY_AM_AUDIO_EFFECT_DAP_GEQ_ENABLE);
            if (param == DroidAudioEffect::DAP_GEQ_EFFECT_MODE_USER) {
                putToDb(DB_KEY_AM_AUDIO_EFFECT_DAP_GEQ_BAND4, value);
            }
            break;
        case DroidAudioEffect::DAP_SUBCMD_GEQ_BAND5:
            param = getIntFromDb(DB_KEY_AM_AUDIO_EFFECT_DAP_GEQ_ENABLE);
            if (param == DroidAudioEffect::DAP_GEQ_EFFECT_MODE_USER) {
                putToDb(DB_KEY_AM_AUDIO_EFFECT_DAP_GEQ_BAND5, value);
            }
            break;
    }
    return 0;
}


int32_t AmlAudioEffectDap::getDapParam(int32_t id) {
    int32_t value = -1, param = 0;
    if (isAudioDebug()) AM_LOGD("getDapParam id:%d", id);
    if (mDapVersion == AML_AUDIO_EFFECT_ID_DAP1_3_2 && id != DroidAudioEffect::DAP_CMD_EFFECT_MODE) {
        value = getIntFromDb(DB_KEY_AM_AUDIO_EFFECT_DAP_MODE);
        if (value != DroidAudioEffect::DAP_EFFECT_MODE_USER) {
            return getDapParamInternal(id);
        }
    }
    switch (id) {
        case DroidAudioEffect::DAP_CMD_EFFECT_MODE:
            value = getIntFromDb(DB_KEY_AM_AUDIO_EFFECT_DAP_MODE);
            break;
        case DroidAudioEffect::DAP_CMD_VL_ENABLE:
            value = getIntFromDb(DB_KEY_AM_AUDIO_EFFECT_DAP_VL_ENABLE);
            break;
        case DroidAudioEffect::DAP_CMD_VL_AMOUNT:
            value = getIntFromDb(DB_KEY_AM_AUDIO_EFFECT_DAP_VL_AMOUNT);
            break;
        case DroidAudioEffect::DAP_CMD_DE_ENABLE:
            value = getIntFromDb(DB_KEY_AM_AUDIO_EFFECT_DAP_DE_ENABLE);
            break;
        case DroidAudioEffect::DAP_CMD_SURROUND_ENABLE:
            value = getIntFromDb(DB_KEY_AM_AUDIO_EFFECT_DAP_SURROUND_ENABLE);
            break;
        case DroidAudioEffect::DAP_CMD_SURROUND_BOOST:
            value = getIntFromDb(DB_KEY_AM_AUDIO_EFFECT_DAP_SURROUND_BOOST);
            break;
        case DroidAudioEffect::DAP_CMD_DE_AMOUNT:
            value = getIntFromDb(DB_KEY_AM_AUDIO_EFFECT_DAP_DE_AMOUNT);
            break;
        case DroidAudioEffect::DAP_CMD_POST_GAIN:
            value = getIntFromDb(DB_KEY_AM_AUDIO_EFFECT_DAP_POST_GAIN);
            break;
        case DroidAudioEffect::DAP_CMD_GEQ_ENABLE:
            value = getIntFromDb(DB_KEY_AM_AUDIO_EFFECT_DAP_GEQ_ENABLE);
            break;
        case DroidAudioEffect::DAP_SUBCMD_GEQ_BAND1:
            param = getIntFromDb(DB_KEY_AM_AUDIO_EFFECT_DAP_GEQ_ENABLE);
            if (param == DroidAudioEffect::DAP_GEQ_EFFECT_MODE_USER)
                value = getIntFromDb(DB_KEY_AM_AUDIO_EFFECT_DAP_GEQ_BAND1);
            else
                value = getDapParamInternal(id);
            break;
        case DroidAudioEffect::DAP_SUBCMD_GEQ_BAND2:
            param = getIntFromDb(DB_KEY_AM_AUDIO_EFFECT_DAP_GEQ_ENABLE);
            if (param == DroidAudioEffect::DAP_GEQ_EFFECT_MODE_USER)
                value = getIntFromDb(DB_KEY_AM_AUDIO_EFFECT_DAP_GEQ_BAND2);
            else
                value = getDapParamInternal(id);
            break;
        case DroidAudioEffect::DAP_SUBCMD_GEQ_BAND3:
            param = getIntFromDb(DB_KEY_AM_AUDIO_EFFECT_DAP_GEQ_ENABLE);
            if (param == DroidAudioEffect::DAP_GEQ_EFFECT_MODE_USER)
                value = getIntFromDb(DB_KEY_AM_AUDIO_EFFECT_DAP_GEQ_BAND3);
            else
                value = getDapParamInternal(id);
            break;
        case DroidAudioEffect::DAP_SUBCMD_GEQ_BAND4:
            param = getIntFromDb(DB_KEY_AM_AUDIO_EFFECT_DAP_GEQ_ENABLE);
            if (param == DroidAudioEffect::DAP_GEQ_EFFECT_MODE_USER)
                value = getIntFromDb(DB_KEY_AM_AUDIO_EFFECT_DAP_GEQ_BAND4);
            else
                value = getDapParamInternal(id);
            break;
        case DroidAudioEffect::DAP_SUBCMD_GEQ_BAND5:
            param = getIntFromDb(DB_KEY_AM_AUDIO_EFFECT_DAP_GEQ_ENABLE);
            if (param == DroidAudioEffect::DAP_GEQ_EFFECT_MODE_USER)
                value = getIntFromDb(DB_KEY_AM_AUDIO_EFFECT_DAP_GEQ_BAND5);
            else
                value = getDapParamInternal(id);
            break;
        case DroidAudioEffect::DAP_CMD_2_4_PROFILE:
            value = getIntFromDb(DB_KEY_AM_AUDIO_EFFECT_DAP_2_4_PROFILE);
            if (value < 0) {
                value = getDapParamInternal(DroidAudioEffect::DAP_CMD_2_4_PROFILE);
                //AM_LOGW("getDapParam id:2_4_PROFILE hal value:", value);
                saveDbDap24Param(id, value);
            }
            break;
        case DroidAudioEffect::DAP_CMD_2_4_MI_STEERING:
        case DroidAudioEffect::DAP_CMD_2_4_SURROUND_DECODER_ENABLE:
        case DroidAudioEffect::DAP_CMD_2_4_SURROUND_VIRTUALIZER:
        case DroidAudioEffect::DAP_SUBCMD_2_4_SURROUND_VIRTUALIZER_BOOST:
        case DroidAudioEffect::DAP_CMD_2_4_DIALOGUE_ENHANCER:
        case DroidAudioEffect::DAP_SUBCMD_2_4_DIALOGUE_ENHANCER_AMOUNT:
        case DroidAudioEffect::DAP_CMD_2_4_BASS_ENHANCER:
        case DroidAudioEffect::DAP_SUBCMD_2_4_BASS_ENHANCER_BOOST:
        case DroidAudioEffect::DAP_SUBCMD_2_4_BASS_ENHANCER_CUTOFF_100:
        case DroidAudioEffect::DAP_SUBCMD_2_4_BASS_ENHANCER_CUTOFF_1:
        case DroidAudioEffect::DAP_SUBCMD_2_4_BASS_ENHANCER_WIDTH:
        case DroidAudioEffect::DAP_CMD_2_4_LEVELER:
        case DroidAudioEffect::DAP_SUBCMD_2_4_LEVELER_AMOUNT:
            value = getDbDap24Param(id);
            //AM_LOGW("getDbDap24Param id:", id + ", value:", value);
            break;
    }
    return value;
}

int32_t AmlAudioEffectDap::getDbDap24Param(int32_t id) {
    int32_t result = -1;
    switch (id) {
        case DroidAudioEffect::DAP_CMD_2_4_PROFILE:
            result = getIntFromDb(DB_KEY_AM_AUDIO_EFFECT_DAP_2_4_PROFILE);
            break;
        case DroidAudioEffect::DAP_CMD_2_4_SURROUND_VIRTUALIZER:
            result = getIntFromDb(DB_KEY_AM_AUDIO_EFFECT_DAP_2_4_SURROUND_VIRTUALIZER_MODE);
            break;
        case DroidAudioEffect::DAP_SUBCMD_2_4_SURROUND_VIRTUALIZER_BOOST:
            result = getIntFromDb(DB_KEY_AM_AUDIO_EFFECT_DAP_2_4_SURROUND_VIRTUALIZER_BOOST);
            break;
        case DroidAudioEffect::DAP_CMD_2_4_DIALOGUE_ENHANCER:
            result = getIntFromDb(DB_KEY_AM_AUDIO_EFFECT_DAP_2_4_DIALOGUE_ENHANCER_ENABLE);
            break;
        case DroidAudioEffect::DAP_CMD_2_4_BASS_ENHANCER:
            result = getIntFromDb(DB_KEY_AM_AUDIO_EFFECT_DAP_2_4_BASS_ENHANCER_ENABLE);
            break;
        case DroidAudioEffect::DAP_SUBCMD_2_4_BASS_ENHANCER_BOOST:
            result = getIntFromDb(DB_KEY_AM_AUDIO_EFFECT_DAP_2_4_BASS_ENHANCER_BOOST);
            break;
        case DroidAudioEffect::DAP_SUBCMD_2_4_BASS_ENHANCER_CUTOFF_100:
            result = getIntFromDb(DB_KEY_AM_AUDIO_EFFECT_DAP_2_4_BASS_ENHANCER_CUTOFFX100) * 100;
            break;
        case DroidAudioEffect::DAP_SUBCMD_2_4_BASS_ENHANCER_CUTOFF_1:
            result = getIntFromDb(DB_KEY_AM_AUDIO_EFFECT_DAP_2_4_BASS_ENHANCER_CUTOFFX1);
            break;
        case DroidAudioEffect::DAP_SUBCMD_2_4_BASS_ENHANCER_WIDTH:
            result = getIntFromDb(DB_KEY_AM_AUDIO_EFFECT_DAP_2_4_BASS_ENHANCER_WIDTH);
            break;
        case DroidAudioEffect::DAP_CMD_2_4_MI_STEERING:
            result = getIntFromDb(DB_KEY_AM_AUDIO_EFFECT_DAP_2_4_MI_STEERING);
            break;
        case DroidAudioEffect::DAP_CMD_2_4_SURROUND_DECODER_ENABLE:
            result = getIntFromDb(DB_KEY_AM_AUDIO_EFFECT_DAP_2_4_SURROUND_DECODER_ENABLE);
            break;
        case DroidAudioEffect::DAP_CMD_2_4_LEVELER:
            result = getIntFromDb(DB_KEY_AM_AUDIO_EFFECT_DAP_2_4_LEVELER_MODE);
            break;
        case DroidAudioEffect::DAP_SUBCMD_2_4_LEVELER_AMOUNT:
            result = getIntFromDb(DB_KEY_AM_AUDIO_EFFECT_DAP_2_4_LEVELER_AMOUNT);
            break;
    }
    if (isAudioDebug()) {
        AM_LOGD("id:%d, value:%d", id, result);
    }
    return result;
}

int32_t AmlAudioEffectDap::saveDbDap24Param(int32_t id, int32_t value) {
    if (isAudioDebug()) AM_LOGD("id:%d, value:%d", id, value);
    switch (id) {
        case DroidAudioEffect::DAP_CMD_2_4_PROFILE:
            putToDb(DB_KEY_AM_AUDIO_EFFECT_DAP_2_4_PROFILE, value);
            break;
        case DroidAudioEffect::DAP_CMD_2_4_SURROUND_VIRTUALIZER:
            putToDb(DB_KEY_AM_AUDIO_EFFECT_DAP_2_4_SURROUND_VIRTUALIZER_MODE, value);
            break;
        case DroidAudioEffect::DAP_SUBCMD_2_4_SURROUND_VIRTUALIZER_BOOST:
            putToDb(DB_KEY_AM_AUDIO_EFFECT_DAP_2_4_SURROUND_VIRTUALIZER_BOOST, value);
            break;
        case DroidAudioEffect::DAP_CMD_2_4_DIALOGUE_ENHANCER:
            putToDb(DB_KEY_AM_AUDIO_EFFECT_DAP_2_4_DIALOGUE_ENHANCER_ENABLE, value);
            break;
        case DroidAudioEffect::DAP_CMD_2_4_BASS_ENHANCER:
            putToDb(DB_KEY_AM_AUDIO_EFFECT_DAP_2_4_BASS_ENHANCER_ENABLE, value);
            break;
        case DroidAudioEffect::DAP_SUBCMD_2_4_BASS_ENHANCER_BOOST:
            putToDb(DB_KEY_AM_AUDIO_EFFECT_DAP_2_4_BASS_ENHANCER_BOOST, value);
            break;
        case DroidAudioEffect::DAP_SUBCMD_2_4_BASS_ENHANCER_CUTOFF_100:
            putToDb(DB_KEY_AM_AUDIO_EFFECT_DAP_2_4_BASS_ENHANCER_CUTOFFX100, value);
            break;
        case DroidAudioEffect::DAP_SUBCMD_2_4_BASS_ENHANCER_CUTOFF_1:
            putToDb(DB_KEY_AM_AUDIO_EFFECT_DAP_2_4_BASS_ENHANCER_CUTOFFX1, value);
            break;
        case DroidAudioEffect::DAP_SUBCMD_2_4_BASS_ENHANCER_WIDTH:
            putToDb(DB_KEY_AM_AUDIO_EFFECT_DAP_2_4_BASS_ENHANCER_WIDTH, value);
            break;
        case DroidAudioEffect::DAP_CMD_2_4_MI_STEERING:
            putToDb(DB_KEY_AM_AUDIO_EFFECT_DAP_2_4_MI_STEERING, value);
            break;
        case DroidAudioEffect::DAP_CMD_2_4_SURROUND_DECODER_ENABLE:
            putToDb(DB_KEY_AM_AUDIO_EFFECT_DAP_2_4_SURROUND_DECODER_ENABLE, value);
            break;
        case DroidAudioEffect::DAP_CMD_2_4_LEVELER:
            putToDb(DB_KEY_AM_AUDIO_EFFECT_DAP_2_4_LEVELER_MODE, value);
            break;
        case DroidAudioEffect::DAP_SUBCMD_2_4_LEVELER_AMOUNT:
            putToDb(DB_KEY_AM_AUDIO_EFFECT_DAP_2_4_LEVELER_AMOUNT, value);
            break;
        default:
            break;
    }
    return 0;
}

int32_t AmlAudioEffectDap::toDapProfileID(int32_t mode) {
    switch (mode) {
        case DroidAudioEffect::COMMON_SOUND_MODE_STANDARD:
            return DroidAudioEffect::DAP_2_4_PROFILE_OFF;
        case DroidAudioEffect::COMMON_SOUND_MODE_MUSIC:
            return DroidAudioEffect::DAP_2_4_PROFILE_MUSIC;
        case DroidAudioEffect::COMMON_SOUND_MODE_GAME:
            return DroidAudioEffect::DAP_2_4_PROFILE_GAME;
        case DroidAudioEffect::COMMON_SOUND_MODE_MOVIE:
            return DroidAudioEffect::DAP_2_4_PROFILE_MOVIE;
        case DroidAudioEffect::COMMON_SOUND_MODE_CUSTOM:
            return DroidAudioEffect::DAP_2_4_PROFILE_USER_SELECTABLE;
        case DroidAudioEffect::COMMON_SOUND_MODE_NIGHT:
            return DroidAudioEffect::DAP_2_4_PROFILE_NIGHT;
        case DroidAudioEffect::COMMON_SOUND_MODE_NEWS:
            return DroidAudioEffect::DAP_2_4_PROFILE_VOICE;
        default: {
            AM_LOGW("Not support SoundMode:%d",mode);
            return DroidAudioEffect::DAP_2_4_PROFILE_OFF;
        }
    }
}

AmlAudioEffectVirtualX::AmlAudioEffectVirtualX(DroidAudioEffectSetting* pSetting) :
    AmlAudioEffect(pSetting, DroidAudioEffect::EFFECT_ID_VIRTUALX, "VirtualX",
        "5112a99e-b8b9-4c5e-91fd-a804d29c36b2", "61821587-ce3c-4aac-9122-86d874ea1fb1") {
    mVirtualXVersion = getIntFromIni(INI_KEY_AM_AUDIO_EFFECT_VIRTUALX_VERSION);
}

int32_t AmlAudioEffectVirtualX::init() {
    if (mVirtualXVersion == INI_VALUE_AM_AUDIO_EFFECT_VIRTUALX_VERSION_1) {
        setDtsVirtualXMode(getDtsVirtualXMode());
        setDtsTruVolumeHdEnabled(isDtsTruVolumeHdEnabled());
    } else if (mVirtualXVersion == INI_VALUE_AM_AUDIO_EFFECT_VIRTUALX_VERSION_4){
        setDtsVirtualXMode(getDtsVirtualXMode());
        //Advanced virtualx settings by user
        setDtsDialogClarityMode(getDtsDialogClarityMode());
        setDtsVirtualSurroundEnabled(isDtsVirtualSurroundEnabled());
        setDtsBassEnhancementEnabled(isDtsBassEnhancementEnabled());
        AM_LOGI("init_effect Virtualx V4 success");
    }
    return 0;
}

int32_t AmlAudioEffectVirtualX::setDtsVirtualXUserMode(int32_t mode) {
    return setParameter(PARAM_CMD_DTS_VIRTUALX_USER_MODE, mode);
}

int32_t AmlAudioEffectVirtualX::setDtsVirtualXEnabled(bool enable) {
    if (isAudioDebug()) AM_LOGD("enable: %d", enable);
    int32_t ret = setParameter(PARAM_CMD_DTS_ENABLE_V4, enable);
    R_CHECK_RET(ret,);
    putToDb(DB_KEY_AM_AUDIO_EFFECT_VIRTUALX_ENABLED, enable);
    return ret;
}

bool AmlAudioEffectVirtualX::isDtsVirtualXEnabled() {
    int32_t value = getParameter(PARAM_CMD_DTS_ENABLE_V4) != 0;
    int32_t saveResult = getIntFromDb(DB_KEY_AM_AUDIO_EFFECT_VIRTUALX_ENABLED);
    if (saveResult != value) {
        AM_LOGW("erro get:%d, saved:%d", value, saveResult);
    }
    bool enable = (saveResult != 0);
    if (isAudioDebug()) AM_LOGD("enable: %d", enable);
    return enable;
}

int32_t AmlAudioEffectVirtualX::setDtsVirtualSurroundEnabled(bool enable) {
    if (isAudioDebug()) AM_LOGD("enable: %d", enable);
    int32_t ret = setParameter(PARAM_CMD_SURROUND_MODE, enable);
    R_CHECK_RET(ret,);
    putToDb(DB_KEY_AM_AUDIO_EFFECT_VIRTUALX_VIRTUAL_SURROUND, enable);
    return ret;
}

bool AmlAudioEffectVirtualX::isDtsVirtualSurroundEnabled() {
    int32_t value = getParameter(PARAM_CMD_SURROUND_MODE);
    int32_t saveResult = getIntFromDb(DB_KEY_AM_AUDIO_EFFECT_VIRTUALX_VIRTUAL_SURROUND);
    if (saveResult != value) {
        AM_LOGW("erro get:%d, saved:%d", value, saveResult);
    }
    bool enable = (saveResult != 0);
    if (isAudioDebug()) AM_LOGD("enable: %d", enable);
    return enable;
}

int32_t AmlAudioEffectVirtualX::setDtsBassEnhancementEnabled(bool enable) {
    if (isAudioDebug()) AM_LOGD("enable: %d", enable);
    int32_t ret = setParameter(PARAM_CMD_TBHDX_PROCESS_DISCARD_I32, enable);
    R_CHECK_RET(ret,);
    putToDb(DB_KEY_AM_AUDIO_EFFECT_VIRTUALX_TRUBASS_DISCARD, enable);
    return ret;
}

bool AmlAudioEffectVirtualX::isDtsBassEnhancementEnabled() {
    int32_t value = getParameter(PARAM_CMD_TBHDX_PROCESS_DISCARD_I32);
    int32_t saveResult = getIntFromDb(DB_KEY_AM_AUDIO_EFFECT_VIRTUALX_TRUBASS_DISCARD);
    if (saveResult != value) {
        AM_LOGW("erro get:%d, saved:%d", value, saveResult);
    }
    bool enable = (saveResult != 0);
    if (isAudioDebug()) AM_LOGD("enable: %d", enable);
    return enable;
}

int32_t AmlAudioEffectVirtualX::setDtsDialogClarityMode(int32_t mode) {
    R_CHECK_PARAM_LEGAL(-1, mode, DroidAudioEffect::VIRTUALX_DIALOGCLARITY_MODE_MIN, DroidAudioEffect::VIRTUALX_DIALOGCLARITY_MODE_MAX,)
    if (isAudioDebug()) AM_LOGD("mode: %d", mode);
    int32_t ret = setParameter(PARAM_CMD_DIALOGCLARITY_MODE, mode);
    R_CHECK_RET(ret,);
    putToDb(DB_KEY_AM_AUDIO_EFFECT_VIRTUALX_DIALOG_CLARITY_MODE, mode);
    return ret;
}

int32_t AmlAudioEffectVirtualX::getDtsDialogClarityMode() {
    int32_t value = getParameter(PARAM_CMD_DIALOGCLARITY_MODE);
    int32_t saveResult = getIntFromDb(DB_KEY_AM_AUDIO_EFFECT_VIRTUALX_DIALOG_CLARITY_MODE);
    if (saveResult != value) {
        AM_LOGW("erro get:%d, saved:%d", value, saveResult);
    }
    if (isAudioDebug()) AM_LOGD("value: %d", saveResult);
    return saveResult;
}

int32_t AmlAudioEffectVirtualX::setDtsVirtualXMode(int32_t mode) {
    if (isAudioDebug()) AM_LOGD("version:%d, mode:%d", mVirtualXVersion, mode);
    if (mVirtualXVersion == INI_VALUE_AM_AUDIO_EFFECT_VIRTUALX_VERSION_1) {
        switch (mode) {
            case DroidAudioEffect::VIRTUALX_MODE_OFF:
                setParameter(PARAM_CMD_DTS_MBHL_ENABLE_I32, 0);
                setParameter(PARAM_CMD_DTS_TBHDX_ENABLE_I32, 0);
                setParameter(PARAM_CMD_DTS_VX_ENABLE_I32, 0);
                break;
            case DroidAudioEffect::VIRTUALX_MODE_BASS:
                setParameter(PARAM_CMD_DTS_MBHL_ENABLE_I32, 1);
                setParameter(PARAM_CMD_DTS_TBHDX_ENABLE_I32, 1);
                setParameter(PARAM_CMD_DTS_VX_ENABLE_I32, 0);
                break;
            case DroidAudioEffect::VIRTUALX_MODE_FULL:
                setParameter(PARAM_CMD_DTS_MBHL_ENABLE_I32, 1);
                setParameter(PARAM_CMD_DTS_TBHDX_ENABLE_I32, 1);
                setParameter(PARAM_CMD_DTS_VX_ENABLE_I32, 1);
                break;
            default:
                AM_LOGW("VirtualX effect mode invalid, mode:%d", mode);
                return -1;
        }
    } else if (mVirtualXVersion == INI_VALUE_AM_AUDIO_EFFECT_VIRTUALX_VERSION_4){
        setParameter(PARAM_CMD_DTS_VIRTUALX_USER_MODE, mode);
    }
    putToDb(DB_KEY_AM_AUDIO_EFFECT_VIRTUALX_MODE, mode);
    return 0;
}

int32_t AmlAudioEffectVirtualX::getDtsVirtualXMode() {
    int32_t enable = getIntFromDb(DB_KEY_AM_AUDIO_EFFECT_VIRTUALX_MODE);
    if (isAudioDebug()) AM_LOGD("enable:%d", enable);
    return enable;
}

int32_t AmlAudioEffectVirtualX::setDtsTruVolumeHdEnabled(bool enable) {
    if (isAudioDebug()) AM_LOGD("enable:%d", enable);
    int32_t dbSwitch = enable ? 1 : 0;
    putToDb(DB_KEY_AM_AUDIO_EFFECT_VIRTUALX_TREVOLUME_HD_ENABLE, dbSwitch);
    int32_t ret = setParameter(PARAM_CMD_DTS_LOUDNESS_CONTROL_ENABLE_I32, dbSwitch);
    R_CHECK_RET(ret,);
    return 0;
}

bool AmlAudioEffectVirtualX::isDtsTruVolumeHdEnabled() {
    int32_t dbSwitch = getIntFromDb(DB_KEY_AM_AUDIO_EFFECT_VIRTUALX_TREVOLUME_HD_ENABLE);
    bool enable = (1 == dbSwitch);
    if (dbSwitch != 1 && dbSwitch != 0) {
        AM_LOGW("DTS Tru Volume HD db value invalid, db:%d, return db: %d", dbSwitch);
    }
    return enable;
}

AmlAudioEffectAmlPeq::AmlAudioEffectAmlPeq(DroidAudioEffectSetting* pSetting) :
    AmlAudioEffect(pSetting, DroidAudioEffect::EFFECT_ID_AMLPEQ, "AmlPeq", "c70c903c-f239-6be4-5365-231c3feb135c", "2484df0d-4586-8a71-7c92-74b6543df8e6") {
}

int32_t AmlAudioEffectAmlPeq::init() {
    return 0;
}

DroidAudioEffectSetting::DroidAudioEffectSetting():
    DroidAudioDbDescriptor(DroidAudioDbDescriptor::DROIDLOGIC_DB_MODULE_ID_AM_AUDIO_EFFECT, g_VecDbString),
    mAmlEffectName({"Hpeq", "Balance", "TrebleBass", "VirtualSurround", "DPE", "DAP", "VirtualX", "AmlPeq"}), // enum AUDIO_EFFECT_ID
    mDualEffectMode(0) {
    mAmlAudioEffect.fill(nullptr);
    AM_LOGI("");
}

vector<uint8_t> DroidAudioEffectSetting::getDefaultValue(const string& key) {
    int32_t value = getIntFromIni(key);
    vector<uint8_t> valueBytes(sizeof(int32_t));
    memcpy(valueBytes.data(), &value, sizeof(int32_t));
    return valueBytes;
}

void DroidAudioEffectSetting::reloadAudio() {
    AM_LOGW("audioserver died, start reinitialization...");
    deinit();
    init();
}

int32_t DroidAudioEffectSetting::dump(int32_t fd, const char **args __unused, uint32_t numArgs __unused) {
    dprintf(fd, "------------------------ DroidAudioEffectSetting ------------------------\n");
    for (int32_t i = DroidAudioEffect::EFFECT_ID_MIN; i < DroidAudioEffect::EFFECT_ID_MAX; i++) {
        dprintf(fd, "%s: %d\n", mAmlEffectName[i].c_str(), isAudioEffectEnabled(i));
    }
    dprintf(fd, "mDolbyMS12AudioConfig:                 %10d | mDtsVirtualxAudioConfig:          %10d\n", mDolbyMS12AudioConfig, mDtsVirtualxAudioConfig);
    dprintf(fd, "mEffectBalanceAudioConfig:             %10d | mEffectTrebleBassAudioConfig:     %10d\n", mEffectBalanceAudioConfig, mEffectTrebleBassAudioConfig);
    dprintf(fd, "mEffectVirtualSurroundAudioConfig:     %10d | mEffectDPEAudioConfig:            %10d\n", mEffectVirtualSurroundAudioConfig, mEffectDPEAudioConfig);
    dprintf(fd, "mEffectEQAudioConfig:                  %10d | mEffectDolbyDRCAudioConfig:       %10d\n", mEffectEQAudioConfig, mEffectDolbyDRCAudioConfig);
    dprintf(fd, "mEffectDtsDRCAudioConfig:              %10d | mAudioLatencyConfig:              %10d\n", mEffectDtsDRCAudioConfig, mAudioLatencyConfig);
    dprintf(fd, "mForceDDPConfig:                       %10d | mEffectEngineerModeConfig:        %10d\n", mForceDDPConfig, mEffectEngineerModeConfig);
    dprintf(fd, "mPassthroughConfig:                    %10d | mAiDeConfig:                      %10d\n", mPassthroughConfig, mAiDeConfig);
    dprintf(fd, "mAiAQConfig:                           %10d | mAiVolumeEqConfig:                %10d\n", mAiAQConfig, mAiVolumeEqConfig);
    dprintf(fd, "mOttMs12Config:                        %10d | mGlobalMicDeviceTypeConfig:       %10d\n", mOttMs12Config, mGlobalMicDeviceTypeConfig);
    dprintf(fd, "mSoundMode:                            %s   | mAISoundModeEnabled:              %10d\n", DroidAudioEffect::soundMode2Str(getSoundMode()), mAISoundModeEnabled);
    return STATUS_OK;
}

int32_t DroidAudioEffectSetting::readAudioConfigFromHal() {
    mEffectEQAudioConfig = 0;
    string value = getParameters("Effect_EQ_Audio_Config");
    if (value == "1") {
        mEffectEQAudioConfig = DroidAudioEffect::HPEQ_BAND_NUM_5;
    } else if (value == "5") {
        mEffectEQAudioConfig = DroidAudioEffect::HPEQ_BAND_NUM_5;
    } else if (value == "7") {
        mEffectEQAudioConfig = DroidAudioEffect::HPEQ_BAND_NUM_7;
    } else if (value == "9") {
        mEffectEQAudioConfig = DroidAudioEffect::HPEQ_BAND_NUM_9;
    }
    auto getConfigValueFromAudioHal = [] (string halParam) {
        string retStr = getParameters(halParam);
        return retStr == "1" ? DroidAudioEffect::EFFECT_CONFIG_ON : DroidAudioEffect::EFFECT_CONFIG_OFF;
    };
    mEffectBalanceAudioConfig = getConfigValueFromAudioHal("Effect_Balance_Audio_Config");
    mEffectTrebleBassAudioConfig = getConfigValueFromAudioHal("Effect_TrebleBass_Audio_Config");
    mEffectDPEAudioConfig = getConfigValueFromAudioHal("Effect_DPE_Audio_Config");
    mEffectVirtualSurroundAudioConfig = getConfigValueFromAudioHal("Effect_VirtualSurround_Audio_Config");
    mEffectDolbyDRCAudioConfig = getConfigValueFromAudioHal("Dolby_DRC_Audio_Config");
    mEffectDtsDRCAudioConfig = getConfigValueFromAudioHal("Dts_DRC_Audio_Config");
    mEffectEngineerModeConfig = getConfigValueFromAudioHal("Engineer_Mode_Audio_Config");
    mAudioLatencyConfig = getConfigValueFromAudioHal("Audio_Latency_Config");
    mForceDDPConfig = getConfigValueFromAudioHal("Force_DDP_Config");
    mDtsVirtualxAudioConfig =  getConfigValueFromAudioHal("Dts_Virtualx_Audio_Config");
    mEffectAmlPeqAudioConfig = getConfigValueFromAudioHal("Aml_Peq_Audio_Config");
    mPassthroughConfig = getConfigValueFromAudioHal("Passthrough_Audio_Config");
    mAiDeConfig = getConfigValueFromAudioHal("Effect_Ai_De_Config");
    mAiAQConfig = getConfigValueFromAudioHal("Effect_Ai_Aq_Config");
    mAiVolumeEqConfig = getConfigValueFromAudioHal("Effect_Volume_Eq_Config");
    string retStr = getParameters("Global_Mic_Device_Type_Config");
    mGlobalMicDeviceTypeConfig = 0;
    if (retStr.length() != 0) {
        mGlobalMicDeviceTypeConfig = stoi(retStr);
    }

    value = getParameters("Dolby_MS12_Audio_Config");
    if (value != "") {
        if (value == "N") {
            mDolbyMS12AudioConfig = DroidAudioEffect::EFFECT_CONFIG_DAP_MS12_N;
        } else if (value == "Z") {
            mDolbyMS12AudioConfig = DroidAudioEffect::EFFECT_CONFIG_DAP_MS12_Z;
        } else if (value == "X") {
            mDolbyMS12AudioConfig = DroidAudioEffect::EFFECT_CONFIG_DAP_MS12_X;
        } else {
            mDolbyMS12AudioConfig = DroidAudioEffect::EFFECT_CONFIG_DAP_MS12_Y;
        }
    } else {
        mDolbyMS12AudioConfig = DroidAudioEffect::EFFECT_CONFIG_DAP_MS12_N;
    }

    value = getParameters("OTT_Dolby_MS12_Audio_Config");
    if (value != "") {
        if (value == "N") {
            mOttMs12Config = DroidAudioEffect::EFFECT_CONFIG_DAP_MS12_N;
        } else if (value == "X") {
            mOttMs12Config = DroidAudioEffect::EFFECT_CONFIG_DAP_MS12_X;
        } else {
            mOttMs12Config = DroidAudioEffect::EFFECT_CONFIG_DAP_MS12_Y;
        }
    } else {
        mOttMs12Config = DroidAudioEffect::EFFECT_CONFIG_DAP_MS12_N;
    }
    return 0;
}

int32_t DroidAudioEffectSetting::init() {
    if (mbEffectInit) {
        AM_LOGW("droidlogic effect is initialized.");
        return 0;
    }
    AM_LOGD("init");
    readAudioConfigFromHal();
    auto isEffectOn = [&] (int32_t effectId) {
        int32_t config = getEffectFunctionConfig(effectConfigId2EffectId(effectId));
        if (DroidAudioEffect::EFFECT_ID_HPEQ == effectId) {
            return config != DroidAudioEffect::EFFECT_CONFIG_OFF;
        } else if (DroidAudioEffect::EFFECT_ID_DAP == effectId) {
            if (!isSupportMs12()) {
                return false;
            }
            int32_t ottMs12Config = getEffectFunctionConfig(DroidAudioEffect::EFFECT_CONFIG_OTT_MS12);
            if (ottMs12Config == DroidAudioEffect::EFFECT_CONFIG_DAP_MS12_X || ottMs12Config == DroidAudioEffect::EFFECT_CONFIG_DAP_MS12_Y) {
                return false;
            } else {
                return (config == DroidAudioEffect::EFFECT_CONFIG_DAP_MS12_Z || config == DroidAudioEffect::EFFECT_CONFIG_DAP_MS12_X);
            }
        } else {
            return config == DroidAudioEffect::EFFECT_CONFIG_ON;
        }
    };
    for (int32_t i = DroidAudioEffect::EFFECT_ID_MIN; i <= DroidAudioEffect::EFFECT_ID_MAX; ++i) {
        if (isEffectOn(i)) {
            sp<AmlAudioEffect> effect = createAudioEffect(i);
            if (isDriverBaseProject() || effect == nullptr) {
                continue;
            }
            unique_lock<mutex> _l(mAmlEffectMutex[i]);
            int32_t ret = effect->init();
            if (ret == 0) {
                AM_LOGI("init effect:%s success", mAmlEffectName[i].c_str());
            } else {
                AM_LOGE("init effect:%s failed", mAmlEffectName[i].c_str());
            }
        }
    }
    applyAudioEffectByPlayEmptyTrack();
    if (isDriverBaseProject()) {
        AM_LOGI("driver base project, do not init");
    } else {
        initDualEffectMode();
        //should be after initDualEffectMode
        initBasicEffectMode();
    }
    setAISoundModeEnable(isAISoundModeEnabled());
    mbEffectInit = true;
    AM_LOGD("dolbyMs12AudioConfig:%d, dtsVirtualxConfig:%d", mDolbyMS12AudioConfig, mDtsVirtualxAudioConfig);
    return 0;
}

int32_t DroidAudioEffectSetting::deinit() {
    if (!mbEffectInit) {
        AM_LOGW("effect not create, return.");
        return 0;
    }
    AM_LOGI("");
    for (int32_t i = DroidAudioEffect::EFFECT_ID_MIN; i <= DroidAudioEffect::EFFECT_ID_MAX; ++i) {
        if (isAudioEffectEnabled(i)) {
            destroyAudioEffect(i);
        }
    }
    mbEffectInit = false;
    return 0;
}

int32_t DroidAudioEffectSetting::reset() {
    if (isDriverBaseProject()) {
        AM_LOGI("driver base project, do not reset");
        return 0;
    }
    readAudioConfigFromHal();
    resetDefaultValue();
    auto isEffectOn = [&] (int32_t effectId) {
        int32_t config = getEffectFunctionConfig(effectConfigId2EffectId(effectId));
        if (DroidAudioEffect::EFFECT_ID_HPEQ == effectId) {
            return config != DroidAudioEffect::EFFECT_CONFIG_OFF;
        } else if (DroidAudioEffect::EFFECT_ID_DAP == effectId) {
            if (!isSupportMs12()) {
                return false;
            }
            int32_t ottMs12Config = getEffectFunctionConfig(DroidAudioEffect::EFFECT_CONFIG_OTT_MS12);
            if (ottMs12Config == DroidAudioEffect::EFFECT_CONFIG_DAP_MS12_X || ottMs12Config == DroidAudioEffect::EFFECT_CONFIG_DAP_MS12_Y) {
                return false;
            } else {
                return (config == DroidAudioEffect::EFFECT_CONFIG_DAP_MS12_Z || config == DroidAudioEffect::EFFECT_CONFIG_DAP_MS12_X);
            }
        } else {
            return config == DroidAudioEffect::EFFECT_CONFIG_ON;
        }
    };

    for (int32_t i = DroidAudioEffect::EFFECT_ID_MIN; i <= DroidAudioEffect::EFFECT_ID_MAX; ++i) {;
        unique_lock<mutex> _l(mAmlEffectMutex[i]);
        sp<AmlAudioEffect> effect = mAmlAudioEffect[i];
        if (isEffectOn(i)) {
            if (effect == nullptr) {
                effect = createAudioEffect(i);
            }
            if (effect != nullptr) {
                effect->init();
            }
        } else {
            if (effect != nullptr) {
                destroyAudioEffect(i);
            }
        }
    }
    applyAudioEffectByPlayEmptyTrack();
    initDualEffectMode();
    initBasicEffectMode();
    setSoundMode(getIntFromDb(DB_KEY_AM_AUDIO_EFFECT_HPEQ_SOUND_MODE));
    AM_LOGI("done!");
    return 0;
}

sp<AmlAudioEffect> DroidAudioEffectSetting::createAudioEffect(int32_t index) {
    R_CHECK_PARAM_LEGAL(nullptr, index, DroidAudioEffect::EFFECT_ID_MIN, DroidAudioEffect::EFFECT_ID_MAX,);
    unique_lock<mutex> _l(mAmlEffectMutex[index]);
    if (index == DroidAudioEffect::EFFECT_ID_DAP) {
        if (mDolbyMS12AudioConfig != DroidAudioEffect::EFFECT_CONFIG_DAP_MS12_Z &&
            mDolbyMS12AudioConfig != DroidAudioEffect::EFFECT_CONFIG_DAP_MS12_X) {
            return nullptr;
        }
    }
    if (mAmlAudioEffect[index] != nullptr) {
        AM_LOGW("AudioEffect(%s) has been created.", mAmlAudioEffect[index]->mEffectName.c_str());
        return mAmlAudioEffect[index];
    }
    sp<AmlAudioEffect> pAudioEffect = nullptr;
    switch (index) {
        case DroidAudioEffect::EFFECT_ID_BALANCE:
            pAudioEffect = new AmlAudioEffectBalance(this);
            break;
        case DroidAudioEffect::EFFECT_ID_TREBLEBASS:
            pAudioEffect = new AmlAudioEffectTrebleBass(this);
            break;
        case DroidAudioEffect::EFFECT_ID_DAP:
            {
                int32_t dapVersion = getIntFromIni(AmlAudioEffectDap::INI_KEY_AM_AUDIO_EFFECT_DAP_VERSION);
                pAudioEffect = new AmlAudioEffectDap(this, dapVersion);
            }
            break;
        case DroidAudioEffect::EFFECT_ID_DPE:
            pAudioEffect = new AmlAudioEffectDpe(this);
            break;
        case DroidAudioEffect::EFFECT_ID_HPEQ:
            pAudioEffect = new AmlAudioEffectHpeq(this);
            break;
        case DroidAudioEffect::EFFECT_ID_VIRTUALSURROUND:
            pAudioEffect = new AmlAudioEffectVirtualSurround(this);
            break;
        case DroidAudioEffect::EFFECT_ID_VIRTUALX:
            pAudioEffect = new AmlAudioEffectVirtualX(this);
            mbSupportVirtualX = true;
            break;
        case DroidAudioEffect::EFFECT_ID_AMLPEQ:
            pAudioEffect = new AmlAudioEffectAmlPeq(this);
            break;
        default:
            AM_LOGE("effect id:%d is invalid!", index);
            return nullptr;
    }

    if (!pAudioEffect) {
        AM_LOGE("create AudioEffect(%s) object failed", mAmlEffectName[index].c_str());
        return nullptr;
    }
    AM_LOGI("new AudioEffect(%s) object success", mAmlEffectName[index].c_str());
    AudioDeviceTypeAddr mDevice = AudioDeviceTypeAddr(AUDIO_DEVICE_NONE, "");
    status_t status = pAudioEffect->set(pAudioEffect->mEffectType.c_str(), pAudioEffect->mEffectUuid.c_str(),
                0, nullptr, 0, AUDIO_SESSION_OUTPUT_MIX, AUDIO_IO_HANDLE_NONE, mDevice, false, true);
    if (status != NO_ERROR) {
        AM_LOGE("set AudioEffect(%s) parameters failed", pAudioEffect->mEffectName.c_str());
        return nullptr;
    }
    status = pAudioEffect->initCheck();
    if (status != NO_ERROR) {
        AM_LOGE("init AudioEffect(%s) failed", pAudioEffect->mEffectName.c_str());
        return nullptr;
    }
    mAmlAudioEffect[index] = pAudioEffect;
    if (index != DroidAudioEffect::EFFECT_ID_DPE) {
        status_t result = pAudioEffect->setEnabled(true);
        if (result != NO_ERROR) {
            AM_LOGE("enable AudioEffect(%s) fail, ret:%d", pAudioEffect->mEffectName.c_str(), result);
        }
    }
    AM_LOGD("create AudioEffect(%s) success.", pAudioEffect->mEffectName.c_str());
    return pAudioEffect;
}

int32_t DroidAudioEffectSetting::destroyAudioEffect(int32_t id) {
    R_CHECK_PARAM_LEGAL(-1, id, DroidAudioEffect::EFFECT_ID_MIN, DroidAudioEffect::EFFECT_ID_MAX,);
    unique_lock<mutex> _l(mAmlEffectMutex[id]);
    if (mAmlAudioEffect[id] == nullptr) {
        AM_LOGW("AudioEffect(%s) has been destroyed.", mAmlEffectName[id].c_str());
        return 0;
    }
    mAmlAudioEffect[id] = nullptr;
    AM_LOGD("destroy AudioEffect(%s) success.", mAmlEffectName[id].c_str());
    return 0;
}

int32_t DroidAudioEffectSetting::setAudioEffectEnabled(int32_t effectId, bool enable) {
    R_CHECK_PARAM_LEGAL(false, effectId, DroidAudioEffect::EFFECT_ID_MIN, DroidAudioEffect::EFFECT_ID_MAX,)
    if (isAudioDebug()) AM_LOGD("id:%s, enable:%d", mAmlEffectName[effectId].c_str(), enable);
    if (enable) {
        return createAudioEffect(effectId) != nullptr ? 0 : -1;
    } else {
        return destroyAudioEffect(effectId);
    }
}

bool DroidAudioEffectSetting::isAudioEffectEnabled(int32_t effectId) {
    R_CHECK_PARAM_LEGAL(false, effectId, DroidAudioEffect::EFFECT_ID_MIN, DroidAudioEffect::EFFECT_ID_MAX,)
    bool enable = true;
    if (mAmlAudioEffect[effectId] == nullptr) {
        enable = false;
    }
    if (isAudioDebug()) AM_LOGD("id:%s, enable:%d", mAmlEffectName[effectId].c_str(), enable);
    return enable;
}

int32_t DroidAudioEffectSetting::setParameter(int32_t effectId, const vector<uint8_t>& param, const vector<uint8_t>& value) {
    R_CHECK_PARAM_LEGAL(-1, effectId, DroidAudioEffect::EFFECT_ID_MIN, DroidAudioEffect::EFFECT_ID_MAX,)
    unique_lock<mutex> _l(mAmlEffectMutex[effectId]);
    auto effect = getEffect<AmlAudioEffectHpeq>(effectId);
    R_CHECK_POINTER_LEGAL(-1, effect, "effect:%s was not created", mAmlEffectName[effectId].c_str());
    return effect->setParameter(param, value);
}

int32_t DroidAudioEffectSetting::getParameter(int32_t effectId, const vector<uint8_t>& param, vector<uint8_t>* pValue) {
    R_CHECK_PARAM_LEGAL(-1, effectId, DroidAudioEffect::EFFECT_ID_MIN, DroidAudioEffect::EFFECT_ID_MAX,)
    R_CHECK_POINTER_LEGAL(-1, pValue, "effect:%s", mAmlEffectName[effectId].c_str());
    unique_lock<mutex> _l(mAmlEffectMutex[effectId]);
    auto effect = getEffect<AmlAudioEffectHpeq>(effectId);
    R_CHECK_POINTER_LEGAL(-1, effect, "effect:%s was not created", mAmlEffectName[effectId].c_str());

    vector<uint8_t> value;
    int32_t ret = effect->getParameter(param, value);
    pValue->insert(pValue->end(), value.begin(), value.end());
    return ret;
}

int32_t DroidAudioEffectSetting::setSoundMode(int32_t mode) {
    R_CHECK_PARAM_LEGAL(-1, mode, DroidAudioEffect::COMMON_SOUND_MODE_MIN, DroidAudioEffect::COMMON_SOUND_MODE_MAX,)
    {
        unique_lock<mutex> _l(mAmlEffectMutex[DroidAudioEffect::EFFECT_ID_HPEQ]);
        auto effect = getEffect<AmlAudioEffectHpeq>(DroidAudioEffect::EFFECT_ID_HPEQ);
        if (effect != nullptr) {
            int32_t hpeqMode = AmlAudioEffectHpeq::toHpeqSoundMode(mode);
            effect->setSoundMode(hpeqMode);
            if (mode == DroidAudioEffect::COMMON_SOUND_MODE_CUSTOM) {
                //set one band, at the same time the others will be set
                effect->setDifferentBandEffects(DroidAudioEffect::HPEQ_MODE_EFFECT_BAND1,
                    getIntFromDb(DB_KEY_AM_AUDIO_EFFECT_HPEQ_BAND1), false);
            }
        }
    }
    {
        unique_lock<mutex> _l(mAmlEffectMutex[DroidAudioEffect::EFFECT_ID_DAP]);
        auto effect = getEffect<AmlAudioEffectDap>(DroidAudioEffect::EFFECT_ID_DAP);
        if (effect != nullptr) {
            int32_t dapProfile = AmlAudioEffectDap::toDapProfileID(mode);
            if (dapProfile >= DroidAudioEffect::DAP_2_4_PROFILE_MIN && dapProfile <= DroidAudioEffect::DAP_2_4_PROFILE_MAX) {
                effect->setDapParam(DroidAudioEffect::DAP_CMD_2_4_PROFILE, dapProfile);
            }
        }
    }
    {
        unique_lock<mutex> _l(mAmlEffectMutex[DroidAudioEffect::EFFECT_ID_VIRTUALX]);
        auto effect = getEffect<AmlAudioEffectVirtualX>(DroidAudioEffect::EFFECT_ID_VIRTUALX);
        if (effect != nullptr) {
            effect->setDtsVirtualXUserMode(mode);
        }
    }

    if (!mAISoundModeEnabled) {
        putToDb(DB_KEY_AM_AUDIO_EFFECT_HPEQ_SOUND_MODE, mode);
    }
    if (isAudioDebug()) AM_LOGD("soundmode: %s,  mAISoundModeEnabled: %d", DroidAudioEffect::soundMode2Str(mode), mAISoundModeEnabled);
    return 0;
}

int32_t DroidAudioEffectSetting::getHpeqBandNum() {
    if (isAudioDebug()) AM_LOGD("mEffectEQAudioConfig:%d", mEffectEQAudioConfig);
    return mEffectEQAudioConfig;
}

int32_t DroidAudioEffectSetting::getOnBasicEffectCount() {
    int count = 0;
    for (int32_t i = DroidAudioEffect::EFFECT_ID_MIN; i <= DroidAudioEffect::EFFECT_ID_AMLPEQ; ++i) {
        if (isAudioEffectEnabled(i)) {
            count++;
        }
    }
    return count;
}

int32_t DroidAudioEffectSetting::initBasicEffectMode() {
    int32_t mode = (mDualEffectMode == DroidAudioEffect::DUAL_EFFECT_MODE_OFF) ?
                    (getOnBasicEffectCount() > 0 ? DroidAudioEffect::EFFECT_CONFIG_ON :  DroidAudioEffect::EFFECT_CONFIG_OFF) :
                    DroidAudioEffect::EFFECT_CONFIG_OFF;
    putToDb(DB_KEY_AM_AUDIO_EFFECT_BASIC_EFFECT_MODE, mode);
    setBasicEffectEnabled(mode);
    return 0;
}

int32_t DroidAudioEffectSetting::setBasicEffectEnabled(bool enable) {
    if (isAudioDebug()) AM_LOGD("enable:%d", enable);
    {
        unique_lock<mutex> _l(mAmlEffectMutex[DroidAudioEffect::EFFECT_ID_HPEQ]);
        auto effect = getEffect<AmlAudioEffectHpeq>(DroidAudioEffect::EFFECT_ID_HPEQ);
        if (effect != nullptr) {
            effect->seParamEnable(enable);
        }
    }
    {
        unique_lock<mutex> _l(mAmlEffectMutex[DroidAudioEffect::EFFECT_ID_TREBLEBASS]);
        auto effect = getEffect<AmlAudioEffectTrebleBass>(DroidAudioEffect::EFFECT_ID_TREBLEBASS);
        if (effect != nullptr) {
            effect->seParamEnable(enable);
        }
    }
    putToDb(DB_KEY_AM_AUDIO_EFFECT_BASIC_EFFECT_MODE, enable);
    mbBasicEffectEnabled = enable;
    return 0;
}

int32_t DroidAudioEffectSetting::initDualEffectMode() {
    int32_t curMode = getDualEffectMode();
    int32_t dbMode = getIntFromDb(DB_KEY_AM_AUDIO_EFFECT_DUAL_SOUND_EFFECT_MODE);
    if (curMode != dbMode) {
        setDualEffectMode(dbMode);
    }
    AM_LOGI("curMode:%d, dbMode:%d", curMode, dbMode);
    return 0;
}

int32_t DroidAudioEffectSetting::setDualEffectMode(int32_t mode) {
    if ((mAmlAudioEffect[DroidAudioEffect::EFFECT_ID_VIRTUALX] == nullptr) &&
        (mAmlAudioEffect[DroidAudioEffect::EFFECT_ID_DAP] == nullptr)) {
        mDualEffectMode = DroidAudioEffect::DUAL_EFFECT_MODE_OFF;
        putToDb(DB_KEY_AM_AUDIO_EFFECT_DUAL_SOUND_EFFECT_MODE, mDualEffectMode);
        AM_LOGW("mode:%d VX and DAP is NULL!", mode);
        return -1;
    }
    if (isAudioDebug()) AM_LOGD("mode:%s", DroidAudioEffect::dualEffectMode2Str(mode));
    int32_t dtsVirtualXEnalbe = 0;
    int32_t dapEnable = 0;
    switch (mode) {
        case DroidAudioEffect::DUAL_EFFECT_MODE_AUTO:
            dtsVirtualXEnalbe = 1;
            dapEnable = 1;
            break;
        case DroidAudioEffect::DUAL_EFFECT_MODE_DTS:
            dtsVirtualXEnalbe = 1;
            break;
        case DroidAudioEffect::DUAL_EFFECT_MODE_DOLBY:
            dapEnable = 1;
            break;
        case DroidAudioEffect::DUAL_EFFECT_MODE_OFF:
            break;
        default:
            AM_LOGD("mode:%d is invalid!", mode);
            return -1;
    }
    {
        unique_lock<mutex> _l(mAmlEffectMutex[DroidAudioEffect::EFFECT_ID_DAP]);
        auto effect = getEffect<AmlAudioEffectDap>(DroidAudioEffect::EFFECT_ID_DAP);
        if (effect != nullptr) {
            effect->setDapEnable(dapEnable);
            if (DroidAudioEffect::DUAL_EFFECT_MODE_DOLBY == mode) {
                mDualEffectMode = mode;
            }
        }
    }
    {
        unique_lock<mutex> _l(mAmlEffectMutex[DroidAudioEffect::EFFECT_ID_VIRTUALX]);
        auto effect = getEffect<AmlAudioEffectVirtualX>(DroidAudioEffect::EFFECT_ID_VIRTUALX);
        if (effect != nullptr) {
            effect->setDtsVirtualXEnabled(dtsVirtualXEnalbe);
            if (DroidAudioEffect::DUAL_EFFECT_MODE_DTS == mode) {
                mDualEffectMode = mode;
            }
        }
    }
    if (DroidAudioEffect::DUAL_EFFECT_MODE_AUTO == mode || DroidAudioEffect::DUAL_EFFECT_MODE_OFF == mode) {
        mDualEffectMode = mode;
    }
    putToDb(DB_KEY_AM_AUDIO_EFFECT_DUAL_SOUND_EFFECT_MODE, mDualEffectMode);
    return 0;
}

int32_t DroidAudioEffectSetting::getDualEffectMode() {
    bool isDapOn = false;
    bool isVXOn = false;
    {
        unique_lock<mutex> _l(mAmlEffectMutex[DroidAudioEffect::EFFECT_ID_DAP]);
        AmlAudioEffectDap* effect = getEffect<AmlAudioEffectDap>(DroidAudioEffect::EFFECT_ID_DAP);
        if (effect != nullptr) {
            isDapOn = effect->getDapEnable() != 0;
        }
    }
    {
        unique_lock<mutex> _l(mAmlEffectMutex[DroidAudioEffect::EFFECT_ID_VIRTUALX]);
        auto effect = getEffect<AmlAudioEffectVirtualX>(DroidAudioEffect::EFFECT_ID_VIRTUALX);
        if (effect != nullptr) {
            isVXOn = effect->isDtsVirtualXEnabled() != 0;
        }
    }
    if (isDapOn && isVXOn) {
        mDualEffectMode = DroidAudioEffect::DUAL_EFFECT_MODE_AUTO;
    } else if (isDapOn && !isVXOn) {
        mDualEffectMode = DroidAudioEffect::DUAL_EFFECT_MODE_DOLBY;
    } else if (!isDapOn && isVXOn) {
        mDualEffectMode = DroidAudioEffect::DUAL_EFFECT_MODE_DTS;
    } else {
        mDualEffectMode = DroidAudioEffect::DUAL_EFFECT_MODE_OFF;
    }
    if (isAudioDebug()) AM_LOGD("mDualEffectMode:%s", DroidAudioEffect::dualEffectMode2Str(mDualEffectMode));
    return mDualEffectMode;
}

int32_t DroidAudioEffectSetting::effectConfigId2EffectId(int32_t configId) {
    switch (configId) {
        case DroidAudioEffect::EFFECT_ID_HPEQ:
            return DroidAudioEffect::EFFECT_CONFIG_HPEQ;
        case DroidAudioEffect::EFFECT_ID_BALANCE:
            return DroidAudioEffect::EFFECT_CONFIG_BALANCE;
        case DroidAudioEffect::EFFECT_ID_TREBLEBASS:
            return DroidAudioEffect::EFFECT_CONFIG_TREBLEBASS;
        case DroidAudioEffect::EFFECT_ID_VIRTUALSURROUND:
            return DroidAudioEffect::EFFECT_CONFIG_VIRTUALSURROUND;
        case DroidAudioEffect::EFFECT_ID_DPE:
            return DroidAudioEffect::EFFECT_CONFIG_DPE;
        case DroidAudioEffect::EFFECT_ID_DAP:
            return DroidAudioEffect::EFFECT_CONFIG_DAP;
        case DroidAudioEffect::EFFECT_ID_VIRTUALX:
            return DroidAudioEffect::EFFECT_CONFIG_VIRTUALX;
        case DroidAudioEffect::EFFECT_ID_AMLPEQ:
            return DroidAudioEffect::EFFECT_CONFIG_AMLPEQ;
        default:
            AM_LOGE("unknown effect id:%d", configId);
            return DroidAudioEffect::EFFECT_CONFIG_OFF;
    }
}

//AI Sound mode APIs
int DroidAudioEffectSetting::AiRcLabel::ueventMsgToAiLabel(const std::string &msg) {
    std::string AISOUND_PATTERN = "AI_SOUND_MODE=";
    std::string_view msgView(msg);

    size_t pos = msgView.find(AISOUND_PATTERN);
    if (pos == std::string_view::npos) {
        AM_LOGW("Invalid AI event message: %s", msg.c_str());
        return 0;
    }

    auto valueView = msgView.substr(pos + AISOUND_PATTERN.length());
    char* endPtr;
    int value = std::strtol(valueView.data(), &endPtr, 10);
    if (endPtr == valueView.data() || *endPtr != '\0') {
        ALOGD("Fail to parse AI Label Value, msg: %s", msg.c_str());
        return 0;
    }

    return value;
}

int32_t DroidAudioEffectSetting::setAISoundModeEnable(bool enable) {
    ALOGI("+++%s() enable:%d", __func__, enable);
    std::string param = std::string("aq_tuning=aiaq -enable ").append(std::to_string(enable ? 1 : 0));
    setParameters(param);
    if (enable) {
        mEventObserver.open(this, nativeUeventHandle);
    } else {
        mEventObserver.close();
    }

    mAISoundModeEnabled = enable;

    //store sound mode to DB, ListPreference use it to flash UI
    putToDb(DB_KEY_AM_AUDIO_EFFECT_HPEQ_SOUND_MODE, DroidAudioEffect::COMMON_SOUND_MODE_DYNAMIC);

    putToDb(DB_KEY_AM_AUDIO_EFFECT_AI_SOUND_ENABLE, mAISoundModeEnabled ? 1 : 0);
    ALOGI("---%s() enable:%d", __func__, enable);
    return 0;
}

bool DroidAudioEffectSetting::isAISoundModeEnabled() {
    int32_t dbMode = getIntFromDb(DB_KEY_AM_AUDIO_EFFECT_AI_SOUND_ENABLE);
    return (dbMode == 1 ? true : false);
}

int32_t DroidAudioEffectSetting::localUeventProcess(const std::string& msg)
{
    int ueventLabelValue = AiRcLabel::ueventMsgToAiLabel(msg);
    if (ueventLabelValue == 0) /* 0: invalid */ {
        return -1;
    }

    AiRcLabel newAiLabel(AiRcLabel::toLabel(ueventLabelValue), AiRcLabel::toScore(ueventLabelValue));
    int newSoundMode = newAiLabel.toSoundMode();
    if (mAISoundLabel.isSoundModeChanged(newAiLabel)) {
        setSoundMode(newSoundMode);
        mAISoundLabel = newAiLabel;
        ALOGI("--- Switch to new Sound Mode: %s, AiLabel:[Label: %s, Score: %.2f]", DroidAudioEffect::soundMode2Str(newSoundMode),
                AiRcLabel::toString(newAiLabel.mLabel).c_str(), newAiLabel.mScore);
    } else {
        ALOGI("--- Ignore new Sound Mode: %s, AiLabel:[Label: %s, Score: %.2f]", DroidAudioEffect::soundMode2Str(newSoundMode),
                AiRcLabel::toString(newAiLabel.mLabel).c_str(), newAiLabel.mScore);
    }
    return 0;
}

int32_t DroidAudioEffectSetting::nativeUeventHandle(void *owner, std::string msg) {
    if (owner != nullptr) {
        DroidAudioEffectSetting *ins = (DroidAudioEffectSetting*)owner;
        ins->localUeventProcess(msg);
    }
    return 0;
}
//AI Sound mode control APIs

int32_t DroidAudioEffectSetting::getEffectFunctionConfig(int32_t id) {
    R_CHECK_PARAM_LEGAL(false, id, DroidAudioEffect::EFFECT_CONFIG_MIN, DroidAudioEffect::EFFECT_CONFIG_MAX,)
    //if (!isTvPlatform()) {
    //    AM_LOGE("UI_ID:%d  Not TV return false!", id);
    //    return DroidAudioEffect::EFFECT_CONFIG_OFF;
    //}
    switch (id) {
        case DroidAudioEffect::EFFECT_CONFIG_HPEQ:
            return mEffectEQAudioConfig;
        case DroidAudioEffect::EFFECT_CONFIG_BALANCE:
            return mEffectBalanceAudioConfig;
        case DroidAudioEffect::EFFECT_CONFIG_TREBLEBASS:
            return mEffectTrebleBassAudioConfig;
        case DroidAudioEffect::EFFECT_CONFIG_VIRTUALSURROUND:
            return mEffectVirtualSurroundAudioConfig;
        case DroidAudioEffect::EFFECT_CONFIG_DPE:
            return mEffectDPEAudioConfig;
        case DroidAudioEffect::EFFECT_CONFIG_DAP:
            return mDolbyMS12AudioConfig;
        case DroidAudioEffect::EFFECT_CONFIG_VIRTUALX:
            return mDtsVirtualxAudioConfig;
        case DroidAudioEffect::EFFECT_CONFIG_AMLPEQ:
            return mEffectAmlPeqAudioConfig;
        case DroidAudioEffect::EFFECT_CONFIG_DOLBY_DRC:
            return mEffectDolbyDRCAudioConfig;
        case DroidAudioEffect::EFFECT_CONFIG_DTS_DRC:
            return mEffectDtsDRCAudioConfig;
        case DroidAudioEffect::EFFECT_CONFIG_ENGINEER_MODE:
            return mEffectEngineerModeConfig;
        case DroidAudioEffect::EFFECT_CONFIG_AUDIO_LATENCY:
            return mAudioLatencyConfig;
        case DroidAudioEffect::EFFECT_CONFIG_FORCE_DDP:
            return mForceDDPConfig;
        case DroidAudioEffect::EFFECT_CONFIG_PASSTHROUGH:
            return mPassthroughConfig;
        case DroidAudioEffect::EFFECT_CONFIG_AI_DE:
            return mAiDeConfig;
        case DroidAudioEffect::EFFECT_CONFIG_AI_AQ:
            return mAiAQConfig;
        case DroidAudioEffect::EFFECT_CONFIG_VOLUME_EQ:
            return mAiVolumeEqConfig;
        case DroidAudioEffect::EFFECT_CONFIG_OTT_MS12:
            return mOttMs12Config;
        case DroidAudioEffect::EFFECT_CONFIG_GLOBAL_MIC_DEVICE_TYPE_CONFIG:
            return mGlobalMicDeviceTypeConfig;
        default:
            AM_LOGE("unknown UI_ID:%d", id);
            return DroidAudioEffect::EFFECT_CONFIG_OFF;
    }
}
