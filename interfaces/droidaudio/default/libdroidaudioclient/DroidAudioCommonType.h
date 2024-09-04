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

#define PROP_AUDIO_OUTPUT_FORCEUSE                              "persist.vendor.media.audio.forceuse"
#define PROP_AUDIO_OUTPUT_STRATEGY                              "persist.vendor.media.audio.output.strategy"
#define PROP_AUDIO_OUTPUT_SPDIF_COEXIST                         "persist.vendor.media.audio.spdif.coexist"

typedef enum {
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
} DROID_AUDIO_CMD_TYPE_E;

// audio_policy_forced_cfg_t (system\media\audio\include\system\audio_policy.h)
typedef enum {
    DROID_AUDIO_FORCE_USE_NONE                                  = 0, // AUDIO_POLICY_FORCE_NONE
    DROID_AUDIO_FORCE_USE_SPEAKER                               = 1, // AUDIO_POLICY_FORCE_SPEAKER
    DROID_AUDIO_FORCE_USE_HEADPHONES                            = 2, // AUDIO_POLICY_FORCE_HEADPHONES
    DROID_AUDIO_FORCE_USE_BT_A2DP                               = 4, // AUDIO_POLICY_FORCE_BT_A2DP
    DROID_AUDIO_FORCE_USE_USB                                   = 5, // AUDIO_POLICY_FORCE_WIRED_ACCESSORY
    DROID_AUDIO_FORCE_USE_HDMI                                  = 9, // AUDIO_POLICY_FORCE_DIGITAL_DOCK
    DROID_AUDIO_FORCE_USE_SPDIF                                 = 8, // AUDIO_POLICY_FORCE_ANALOG_DOCK
} DROID_AUDIO_FORCE_USE_TYPE_E;

/* 0: Auto  1: Semi-Auto  2: Manual (refer to: audio_output_strategy enum in Engine.cpp) */
typedef enum {
    DROID_AUDIO_OUTPUT_STRATEGY_AUTO                            = 0,
    DROID_AUDIO_OUTPUT_STRATEGY_SEMI_AUTO                       = 1,
    DROID_AUDIO_OUTPUT_STRATEGY_MANUAL                          = 2,
} DROID_AUDIO_OUTPUT_STRATEGY_TYPE_E;

inline const char* audioCmd2Str(int type)
{
    ENUM_TYPE_TO_STR_START("DROID_AUDIO_CMD_");
    ENUM_TYPE_TO_STR(DROID_AUDIO_CMD_START_DECODE)
    ENUM_TYPE_TO_STR(DROID_AUDIO_CMD_PAUSE_DECODE)
    ENUM_TYPE_TO_STR(DROID_AUDIO_CMD_RESUME_DECODE)
    ENUM_TYPE_TO_STR(DROID_AUDIO_CMD_STOP_DECODE)
    ENUM_TYPE_TO_STR(DROID_AUDIO_CMD_SET_DECODE_AD)
    ENUM_TYPE_TO_STR(DROID_AUDIO_CMD_SET_VOLUME)
    ENUM_TYPE_TO_STR(DROID_AUDIO_CMD_SET_MUTE)
    ENUM_TYPE_TO_STR(DROID_AUDIO_CMD_SET_OUTPUT_MODE)
    ENUM_TYPE_TO_STR(DROID_AUDIO_CMD_SET_PRE_GAIN)
    ENUM_TYPE_TO_STR(DROID_AUDIO_CMD_SET_PRE_MUTE)
    ENUM_TYPE_TO_STR(DROID_AUDIO_CMD_OPEN_DECODER)
    ENUM_TYPE_TO_STR(DROID_AUDIO_CMD_CLOSE_DECODER)
    ENUM_TYPE_TO_STR(DROID_AUDIO_CMD_SET_DEMUX_INFO)
    ENUM_TYPE_TO_STR(DROID_AUDIO_CMD_SET_SECURITY_MEM_LEVEL)
    ENUM_TYPE_TO_STR(DROID_AUDIO_CMD_SET_HAS_VIDEO)
    ENUM_TYPE_TO_STR(DROID_AUDIO_CMD_SET_MEDIA_SYCN_ID)
    ENUM_TYPE_TO_STR(DROID_AUDIO_CMD_AD_SWITCH_ENABLE)
    ENUM_TYPE_TO_STR(DROID_AUDIO_CMD_AD_SET_VOLUME)
    ENUM_TYPE_TO_STR(DROID_AUDIO_CMD_AD_DUAL_SUPPORT)
    ENUM_TYPE_TO_STR(DROID_AUDIO_CMD_AD_MIX_SUPPORT)
    ENUM_TYPE_TO_STR(DROID_AUDIO_CMD_AD_MIX_LEVEL)
    ENUM_TYPE_TO_STR(DROID_AUDIO_CMD_AD_SET_MAIN)
    ENUM_TYPE_TO_STR(DROID_AUDIO_CMD_AD_SET_ASSOCIATE)
    ENUM_TYPE_TO_STR(DROID_AUDIO_CMD_SET_MEDIA_PRESENTATION_ID)
    ENUM_TYPE_TO_STR(DROID_AUDIO_CMD_SET_AUDIO_PATCH_MANAGE_MODE)
    ENUM_TYPE_TO_STR(DROID_AUDIO_CMD_SET_SPDIF_PROTECTION_MODE)
    ENUM_TYPE_TO_STR(DROID_AUDIO_CMD_SET_TSPLAYER_CLIENT_DIED)
    ENUM_TYPE_TO_STR(DROID_AUDIO_CMD_SET_MEDIA_FIRST_LANG)
    ENUM_TYPE_TO_STR(DROID_AUDIO_CMD_SET_MEDIA_SECOND_LANG)
    ENUM_TYPE_TO_STR(DROID_AUDIO_CMD_SET_AUDIO_PICTURE_MODE)
    ENUM_TYPE_TO_STR(DROID_AUDIO_CMD_SET_AUDIO_PLAYBACK_MODE)
    ENUM_TYPE_TO_STR_END
}

