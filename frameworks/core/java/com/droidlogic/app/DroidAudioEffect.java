/*
 * Copyright (C) 2024 Amlogic Corporation.
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

package com.droidlogic.app;


import java.math.BigDecimal;
import java.security.PublicKey;
import java.text.DecimalFormat;
import java.text.DecimalFormatSymbols;
import java.util.Arrays;
import java.util.HashSet;
import java.util.Locale;

import android.content.ComponentName;
import android.content.Context;
import android.content.ContentResolver;
import android.content.Intent;
import android.content.ServiceConnection;
import android.media.AudioManager;
import android.os.Handler;
import android.os.IBinder;
import android.os.Looper;
import android.os.RemoteException;
import android.provider.Settings;
import android.text.TextUtils;
import android.util.Log;

import com.droidlogic.app.DroidAudioServiceManager;

import vendor.amlogic.hardware.droidaudio.IDroidAudio;
import vendor.amlogic.hardware.droidaudio.IDroidAudioClient;

import android.provider.Settings;
import android.media.AudioManager;

public class DroidAudioEffect {
    private String TAG = "DroidAudioEffect.J";
    private static IDroidAudio mDroidAudioService;
    private Context mContext;
    private AudioManager mAudioManager;
    private final ContentResolver mResolver;

    private static DroidAudioEffect mInstance;
    public static DroidAudioEffect getInstance(Context context) {
        if (mInstance == null) {
            synchronized (DroidAudioEffect.class) {
                if (mInstance == null) {
                    mInstance = new DroidAudioEffect(context);
                }
            }
        }
        return mInstance;
    }

    private DroidAudioEffect(Context context) {
        Log.i(TAG, "construction DroidAudioEffect");
        mContext = context;
        mAudioManager = (AudioManager)context.getSystemService(Context.AUDIO_SERVICE);
        mResolver = context.getContentResolver();
    }

    @FunctionalInterface
    private interface RemoteCallExe<T> {
        T execute() throws RemoteException;
    }

    private <T> T executeRemoteCall(RemoteCallExe<T> call, String methodName, T failRetVal) {
        mDroidAudioService = DroidAudioServiceManager.getService();
        if (mDroidAudioService == null) return failRetVal;
        try {
            return call.execute();
        } catch (RemoteException e) {
            Log.e(TAG, methodName + " failed: " + e);
        }
        return failRetVal;
    }

    public static final int EFFECT_CONFIG_OFF                                                          = 0;
    public static final int EFFECT_CONFIG_ON                                                           = 1;

    public static final int EFFECT_ID_HPEQ                                                             = 0;
    public static final int EFFECT_ID_BALANCE                                                          = 1;
    public static final int EFFECT_ID_TREBLEBASS                                                       = 2;
    public static final int EFFECT_ID_VIRTUALSURROUND                                                  = 3;
    public static final int EFFECT_ID_DPE                                                              = 4;
    public static final int EFFECT_ID_AMLPEQ                                                           = 5;
    public static final int EFFECT_ID_DAP                                                              = 6;
    public static final int EFFECT_ID_VIRTUALX                                                         = 7;
    public static final int EFFECT_ID_MIN                                                              = EFFECT_ID_HPEQ;
    public static final int EFFECT_ID_MAX                                                              = EFFECT_ID_VIRTUALX;
    public int setAudioEffectEnabled(int effectId, boolean enable) {
        return executeRemoteCall(() -> mDroidAudioService.AudioEffect_setAudioEffectEnabled(effectId, enable), "setAudioEffectEnabled", 0);
    }
    public boolean isAudioEffectEnabled(int effectId) {
        return executeRemoteCall(() -> mDroidAudioService.AudioEffect_isAudioEffectEnabled(effectId), "isAudioEffectEnabled", false);
    }
    public int setParameter(int effectId, byte[] param, byte[] value) {
        return executeRemoteCall(() -> mDroidAudioService.AudioEffect_setParameter(effectId, param, value), "setParameter", 0);
    }
    public byte[] getParameter(int effectId, byte[] param) {
        return executeRemoteCall(() -> mDroidAudioService.AudioEffect_getParameter(effectId, param), "getParameter", null);
    }
    public int setBasicEffectEnabled(boolean enable) {
        return executeRemoteCall(() -> mDroidAudioService.AudioEffect_setBasicEffectEnabled(enable), "setBasicEffectEnabled", 0);
    }
    public boolean isBasicEffectEnabled() {
        return executeRemoteCall(() -> mDroidAudioService.AudioEffect_isBasicEffectEnabled(), "isBasicEffectEnabled", false);
    }
    public int initDualEffectMode() {
        return executeRemoteCall(() -> mDroidAudioService.AudioEffect_initDualEffectMode(), "initDualEffectMode", 0);
    }

    public static final int DUAL_EFFECT_MODE_AUTO                                                      = 0;
    public static final int DUAL_EFFECT_MODE_DTS                                                       = 1;
    public static final int DUAL_EFFECT_MODE_DOLBY                                                     = 2;
    public static final int DUAL_EFFECT_MODE_OFF                                                       = 3;
    public static final int DUAL_EFFECT_MODE_MIN                                                       = DUAL_EFFECT_MODE_AUTO;
    public static final int DUAL_EFFECT_MODE_MAX                                                       = DUAL_EFFECT_MODE_OFF;
    public int setDualEffectMode(int mode) {
        return executeRemoteCall(() -> mDroidAudioService.AudioEffect_setDualEffectMode(mode), "setDualEffectMode", 0);
    }
    public int getDualEffectMode() {
        return executeRemoteCall(() -> mDroidAudioService.AudioEffect_getDualEffectMode(), "getDualEffectMode", 0);
    }

    /*
    * Dolby ms12 audio config
    * -N: Not support MS12
    * -X: support MS12, config = x
    * -Y: support MS12, config = y
    * -X: support MS12, config = z
    */
    // for EFFECT_CONFIG_DAP
    public static final int EFFECT_CONFIG_DAP_MS12_N                                                   = -1;
    public static final int EFFECT_CONFIG_DAP_MS12_X                                                   = 1;
    public static final int EFFECT_CONFIG_DAP_MS12_Y                                                   = 0;
    public static final int EFFECT_CONFIG_DAP_MS12_Z                                                   = 2;


    //karaoke source MIC config
    public static final int MIC_SOURCE_CONFIG_USB_IN                                                   = (1 << 0);
    public static final int MIC_SOURCE_CONFIG_LINE_IN                                                  = (1 << 1);

    public static final int EFFECT_CONFIG_HPEQ                                                         = 0;
    public static final int EFFECT_CONFIG_BALANCE                                                      = 1;
    public static final int EFFECT_CONFIG_TREBLEBASS                                                   = 2;
    public static final int EFFECT_CONFIG_VIRTUALSURROUND                                              = 3;
    public static final int EFFECT_CONFIG_DPE                                                          = 4;
    public static final int EFFECT_CONFIG_DAP                                                          = 5;
    public static final int EFFECT_CONFIG_VIRTUALX                                                     = 6;
    public static final int EFFECT_CONFIG_AMLPEQ                                                       = 7;
    public static final int EFFECT_CONFIG_DOLBY_DRC                                                    = 8;
    public static final int EFFECT_CONFIG_DTS_DRC                                                      = 9;
    public static final int EFFECT_CONFIG_ENGINEER_MODE                                                = 10;
    public static final int EFFECT_CONFIG_AUDIO_LATENCY                                                = 11;
    public static final int EFFECT_CONFIG_FORCE_DDP                                                    = 12;
    public static final int EFFECT_CONFIG_PASSTHROUGH                                                  = 13;
    public static final int EFFECT_CONFIG_AI_DE                                                        = 14;
    public static final int EFFECT_CONFIG_AI_AQ                                                        = 15;
    public static final int EFFECT_CONFIG_VOLUME_EQ                                                    = 16;
    public static final int EFFECT_CONFIG_OTT_MS12                                                     = 17;
    public static final int EFFECT_CONFIG_GLOBAL_MIC_DEVICE_TYPE_CONFIG                                = 18;
    public static final int EFFECT_CONFIG_MIN                                                          = EFFECT_CONFIG_HPEQ;
    public static final int EFFECT_CONFIG_MAX                                                          = EFFECT_CONFIG_GLOBAL_MIC_DEVICE_TYPE_CONFIG;
    public int getEffectFunctionConfig(int id) {
        return executeRemoteCall(() -> mDroidAudioService.AudioEffect_getEffectFunctionConfig(id), "getEffectFunctionConfig", 0);
    }

    public static final int EFFECT_CONFIG_STEP_MIN                                                     = 0;
    public static final int EFFECT_CONFIG_STEP_MAX                                                     = 100;
    public int setBalance(int step) {
        return executeRemoteCall(() -> mDroidAudioService.AudioEffect_setBalance(step), "setBalance", 0);
    }
    public int getBalance() {
        return executeRemoteCall(() -> mDroidAudioService.AudioEffect_getBalance(), "getBalance", 0);
    }

    public int setTreble(int step) {
        return executeRemoteCall(() -> mDroidAudioService.AudioEffect_setTreble(step), "setTreble", 0);
    }
    public int getTreble() {
        return executeRemoteCall(() -> mDroidAudioService.AudioEffect_getTreble(), "getTreble", 0);
    }
    public int setBass(int step) {
        return executeRemoteCall(() -> mDroidAudioService.AudioEffect_setBass(step), "setBass", 0);
    }
    public int getBass() {
        return executeRemoteCall(() -> mDroidAudioService.AudioEffect_getBass(), "getBass", 0);
    }

    public static final int DAP_2_4_PROFILE_MOVIE                                                      = 0;
    public static final int DAP_2_4_PROFILE_MUSIC                                                      = 1;
    public static final int DAP_2_4_PROFILE_GAME                                                       = 2;
    public static final int DAP_2_4_PROFILE_NIGHT                                                      = 3;
    public static final int DAP_2_4_PROFILE_VOICE                                                      = 4;
    public static final int DAP_2_4_PROFILE_USER_SELECTABLE                                            = 5;
    public static final int DAP_2_4_PROFILE_OFF                                                        = 6;
    public static final int DAP_2_4_PROFILE_MIN                                                        = DAP_2_4_PROFILE_MOVIE;
    public static final int DAP_2_4_PROFILE_MAX                                                        = DAP_2_4_PROFILE_OFF;

    // for DAP_CMD_2_4_SURROUND_VIRTUALIZER
    public static final int DAP_2_4_SURROUND_VIRTUALIZER_OFF                                           = 0;
    public static final int DAP_2_4_SURROUND_VIRTUALIZER_ON                                            = 1;
    public static final int DAP_2_4_SURROUND_VIRTUALIZER_AUTO                                          = 2;

    // for DAP_CMD_2_4_LEVELER
    public static final int DAP_2_4_LEVELER_OFF                                                        = 0;
    public static final int DAP_2_4_LEVELER_ON                                                         = 1;
    public static final int DAP_2_4_LEVELER_AUTO                                                       = 2;

    // for DAP_CMD_EFFECT_MODE
    public static final int DAP_EFFECT_MODE_OFF                                                        = 0;
    public static final int DAP_EFFECT_MODE_MOVIE                                                      = 1;
    public static final int DAP_EFFECT_MODE_MUSIC                                                      = 2;
    public static final int DAP_EFFECT_MODE_NIGHT                                                      = 3;
    public static final int DAP_EFFECT_MODE_USER                                                       = 4;

    // for DAP_SUBCMD_GEQ_BAND[x]
    public static final int DAP_GEQ_EFFECT_MODE_OFF                                                    = 0;
    public static final int DAP_GEQ_EFFECT_MODE_INIT                                                   = 1;
    public static final int DAP_GEQ_EFFECT_MODE_OPEN                                                   = 2;
    public static final int DAP_GEQ_EFFECT_MODE_RICH                                                   = 3;
    public static final int DAP_GEQ_EFFECT_MODE_FOCUSED                                                = 4;
    public static final int DAP_GEQ_EFFECT_MODE_USER                                                   = 5;

    /* [DAP 1.3.2] */
    public static final int DAP_CMD_1_3_2_BASE_VALUE                                                   = 0;
    public static final int DAP_CMD_ENABLE                                                             = 0;
    public static final int DAP_CMD_EFFECT_MODE                                                        = 1;
    public static final int DAP_CMD_GEQ_GAINS                                                          = 2;
    public static final int DAP_CMD_GEQ_ENABLE                                                         = 3;
    public static final int DAP_CMD_POST_GAIN                                                          = 4;
    public static final int DAP_CMD_VL_ENABLE                                                          = 5;
    public static final int DAP_CMD_VL_AMOUNT                                                          = 6;
    public static final int DAP_CMD_DE_ENABLE                                                          = 7;
    public static final int DAP_CMD_DE_AMOUNT                                                          = 8;
    public static final int DAP_CMD_SURROUND_ENABLE                                                    = 9;
    public static final int DAP_CMD_SURROUND_BOOST                                                     = 10;
    public static final int DAP_CMD_VIRTUALIZER_ENABLE                                                 = 11;
    public static final int DAP_SUBCMD_GEQ_BAND1                                                       = 0x100;
    public static final int DAP_SUBCMD_GEQ_BAND2                                                       = 0x101;
    public static final int DAP_SUBCMD_GEQ_BAND3                                                       = 0x102;
    public static final int DAP_SUBCMD_GEQ_BAND4                                                       = 0x103;
    public static final int DAP_SUBCMD_GEQ_BAND5                                                       = 0x104;
    /* [DAP 2.4] */
    public static final int DAP_CMD_2_4_BASE_VALUE                                                     = 1000;
    public static final int DAP_CMD_2_4_PROFILE                                                        = 1000;
    public static final int DAP_CMD_2_4_SURROUND_VIRTUALIZER                                           = 1004;
    public static final int DAP_CMD_2_4_DIALOGUE_ENHANCER                                              = 1007;
    public static final int DAP_CMD_2_4_BASS_ENHANCER                                                  = 1008;
    public static final int DAP_CMD_2_4_MI_STEERING                                                    = 1012;
    public static final int DAP_CMD_2_4_SURROUND_DECODER_ENABLE                                        = 1013;
    public static final int DAP_CMD_2_4_LEVELER                                                        = 1015;
    public static final int DAP_SUBCMD_2_4_BASE_VALUE                                                  = 2000;
    public static final int DAP_SUBCMD_2_4_SURROUND_VIRTUALIZER_BOOST                                  = 2000;
    public static final int DAP_SUBCMD_2_4_DIALOGUE_ENHANCER_AMOUNT                                    = 2001;
    public static final int DAP_SUBCMD_2_4_BASS_ENHANCER_BOOST                                         = 2002;
    public static final int DAP_SUBCMD_2_4_BASS_ENHANCER_CUTOFF_100                                    = 2003;
    public static final int DAP_SUBCMD_2_4_BASS_ENHANCER_CUTOFF_1                                      = 2004;
    public static final int DAP_SUBCMD_2_4_BASS_ENHANCER_WIDTH                                         = 2005;
    public static final int DAP_SUBCMD_2_4_LEVELER_AMOUNT                                              = 2006;
    public int setDapParam(int id, int value) {
        return executeRemoteCall(() -> mDroidAudioService.AudioEffect_setDapParam(id, value), "setDapParam", 0);
    }
    public int getDapParam(int id) {
        return executeRemoteCall(() -> mDroidAudioService.AudioEffect_getDapParam(id), "getDapParam", 0);
    }

    //dpe effect param define
    public static final int DPE_CMD_ENABLED                                                            = 48;
    public static final int DPE_CMD_INPUTGAIN                                                          = 32;
    public static final int DPE_CMD_PRE_EQ                                                             = 64;
    public static final int DPE_CMD_PRE_EQ_BAND                                                        = 69;
    public static final int DPE_CMD_MBC                                                                = 80;
    public static final int DPE_CMD_MBC_BAND                                                           = 85;
    public static final int DPE_CMD_POST_EQ                                                            = 96;
    public static final int DPE_CMD_POST_EQ_BAND                                                       = 101;
    public static final int DPE_CMD_LIMITER                                                            = 112;
    // dpe effect sub pre eq band 0 param define
    public static final int DPE_SUBCMD_PRE_EQ_BAND0                                                    = 100000;
    public static final int DPE_SUBCMD_PRE_EQ_BAND0_CUTOFFFREQUENCY                                    = 10000;
    public static final int DPE_SUBCMD_PRE_EQ_BAND0_GAIN                                               = 10001;
    // dpe effect sub pre eq band 1 param define
    public static final int DPE_SUBCMD_PRE_EQ_BAND1                                                    = 100100;
    public static final int DPE_SUBCMD_PRE_EQ_BAND1_CUTOFFFREQUENCY                                    = 10010;
    public static final int DPE_SUBCMD_PRE_EQ_BAND1_GAIN                                               = 10011;
    // dpe effect sub pre eq band 2 param define
    public static final int DPE_SUBCMD_PRE_EQ_BAND2                                                    = 100200;
    public static final int DPE_SUBCMD_PRE_EQ_BAND2_CUTOFFFREQUENCY                                    = 10020;
    public static final int DPE_SUBCMD_PRE_EQ_BAND2_GAIN                                               = 10021;
    // dpe effect sub mbc param band 0 define
    public static final int DPE_SUBCMD_MBC_BAND0                                                       = 110000;
    public static final int DPE_SUBCMD_MBC_BAND0_CUTOFFFREQUENCY                                       = 11000;
    public static final int DPE_SUBCMD_MBC_BAND0_ATTACKTIME                                            = 11001;
    public static final int DPE_SUBCMD_MBC_BAND0_RELEASETIME                                           = 11002;
    public static final int DPE_SUBCMD_MBC_BAND0_RATIO                                                 = 11003;
    public static final int DPE_SUBCMD_MBC_BAND0_THRESHOLD                                             = 11004;
    public static final int DPE_SUBCMD_MBC_BAND0_KNEEWIDTH                                             = 11005;
    public static final int DPE_SUBCMD_MBC_BAND0_NOISEGATETHRESHOLD                                    = 11006;
    public static final int DPE_SUBCMD_MBC_BAND0_EXPANDERRATIO                                         = 11007;
    public static final int DPE_SUBCMD_MBC_BAND0_PREGAIN                                               = 11008;
    public static final int DPE_SUBCMD_MBC_BAND0_POSTGAIN                                              = 11009;
    // dpe effect sub mbc param band 1 define
    public static final int DPE_SUBCMD_MBC_BAND1                                                       = 110100;
    public static final int DPE_SUBCMD_MBC_BAND1_CUTOFFFREQUENCY                                       = 11010;
    public static final int DPE_SUBCMD_MBC_BAND1_ATTACKTIME                                            = 11011;
    public static final int DPE_SUBCMD_MBC_BAND1_RELEASETIME                                           = 11012;
    public static final int DPE_SUBCMD_MBC_BAND1_RATIO                                                 = 11013;
    public static final int DPE_SUBCMD_MBC_BAND1_THRESHOLD                                             = 11014;
    public static final int DPE_SUBCMD_MBC_BAND1_KNEEWIDTH                                             = 11015;
    public static final int DPE_SUBCMD_MBC_BAND1_NOISEGATETHRESHOLD                                    = 11016;
    public static final int DPE_SUBCMD_MBC_BAND1_EXPANDERRATIO                                         = 11017;
    public static final int DPE_SUBCMD_MBC_BAND1_PREGAIN                                               = 11018;
    public static final int DPE_SUBCMD_MBC_BAND1_POSTGAIN                                              = 11019;
    // dpe effect sub mbc param band 2 define
    public static final int DPE_SUBCMD_MBC_BAND2                                                       = 110200;
    public static final int DPE_SUBCMD_MBC_BAND2_CUTOFFFREQUENCY                                       = 11020;
    public static final int DPE_SUBCMD_MBC_BAND2_ATTACKTIME                                            = 11021;
    public static final int DPE_SUBCMD_MBC_BAND2_RELEASETIME                                           = 11022;
    public static final int DPE_SUBCMD_MBC_BAND2_RATIO                                                 = 11023;
    public static final int DPE_SUBCMD_MBC_BAND2_THRESHOLD                                             = 11024;
    public static final int DPE_SUBCMD_MBC_BAND2_KNEEWIDTH                                             = 11025;
    public static final int DPE_SUBCMD_MBC_BAND2_NOISEGATETHRESHOLD                                    = 11026;
    public static final int DPE_SUBCMD_MBC_BAND2_EXPANDERRATIO                                         = 11027;
    public static final int DPE_SUBCMD_MBC_BAND2_PREGAIN                                               = 11028;
    public static final int DPE_SUBCMD_MBC_BAND2_POSTGAIN                                              = 11029;
    // dpe effect sub post eq band 0 param define
    public static final int DPE_SUBCMD_POST_EQ_BAND0                                                   = 120000;
    public static final int DPE_SUBCMD_POST_EQ_BAND0_CUTOFFFREQUENCY                                   = 1200 ;
    public static final int DPE_SUBCMD_POST_EQ_BAND0_GAIN                                              = 12001;
    // dpe effect sub post eq band 1 param define
    public static final int DPE_SUBCMD_POST_EQ_BAND1                                                   = 120100;
    public static final int DPE_SUBCMD_POST_EQ_BAND1_CUTOFFFREQUENCY                                   = 12010;
    public static final int DPE_SUBCMD_POST_EQ_BAND1_GAIN                                              = 12011;
    // dpe effect sub post eq band 2 param define
    public static final int DPE_SUBCMD_POST_EQ_BAND2                                                   = 120200;
    public static final int DPE_SUBCMD_POST_EQ_BAND2_CUTOFFFREQUENCY                                   = 12020;
    public static final int DPE_SUBCMD_POST_EQ_BAND2_GAIN                                              = 12021;
    //dpe limiter sub  param
    public static final int DPE_SUBCMD_LIMITER_ATTACKTIME                                              = 13001;
    public static final int DPE_SUBCMD_LIMITER_RELEASETIME                                             = 13002;
    public static final int DPE_SUBCMD_LIMITER_RATIO                                                   = 13003;
    public static final int DPE_SUBCMD_LIMITER_THRESHOLD                                               = 13004;
    public static final int DPE_SUBCMD_LIMITER_POSTGAIN                                                = 13005;
    public int setDpeParam(int id, int value) {
        return executeRemoteCall(() -> mDroidAudioService.AudioEffect_setDpeParam(id, value), "setDpeParam", 0);
    }
    public int getDpeParam(int id) {
        return executeRemoteCall(() -> mDroidAudioService.AudioEffect_getDpeParam(id), "getDpeParam", 0);
    }

    public static final int COMMON_SOUND_MODE_DYNAMIC                                                   = 0;
    public static final int COMMON_SOUND_MODE_STANDARD                                                  = 1;
    public static final int COMMON_SOUND_MODE_MUSIC                                                     = 2;
    public static final int COMMON_SOUND_MODE_NEWS                                                      = 3;
    public static final int COMMON_SOUND_MODE_MOVIE                                                     = 4;
    public static final int COMMON_SOUND_MODE_GAME                                                      = 5;
    public static final int COMMON_SOUND_MODE_CUSTOM                                                    = 6;
    public static final int COMMON_SOUND_MODE_NIGHT                                                     = 7;
    public static final int COMMON_SOUND_MODE_MIN                                                      = COMMON_SOUND_MODE_DYNAMIC;
    public static final int COMMON_SOUND_MODE_MAX                                                      = COMMON_SOUND_MODE_CUSTOM;
    public int setSoundMode(int mode) {
        return executeRemoteCall(() -> mDroidAudioService.AudioEffect_setSoundMode(mode), "setSoundMode", 0);
    }
    public int getSoundMode() {
        return executeRemoteCall(() -> mDroidAudioService.AudioEffect_getSoundMode(), "getSoundMode", 0);
    }

    public static final int HPEQ_MODE_EFFECT_BAND1                                                     = 0;
    public static final int HPEQ_MODE_EFFECT_BAND2                                                     = 1;
    public static final int HPEQ_MODE_EFFECT_BAND3                                                     = 2;
    public static final int HPEQ_MODE_EFFECT_BAND4                                                     = 3;
    public static final int HPEQ_MODE_EFFECT_BAND5                                                     = 4;
    public static final int HPEQ_MODE_EFFECT_BAND6                                                     = 5;
    public static final int HPEQ_MODE_EFFECT_BAND7                                                     = 6;
    public static final int HPEQ_MODE_EFFECT_BAND8                                                     = 7;
    public static final int HPEQ_MODE_EFFECT_BAND9                                                     = 8;
    public static final int HPEQ_MODE_EFFECT_BAND_MIN                                                  = HPEQ_MODE_EFFECT_BAND1;
    public static final int HPEQ_MODE_EFFECT_BAND_MAX                                                  = HPEQ_MODE_EFFECT_BAND9;
    public int setUserSoundModeParam(int bandNumber, int value, int bandSum) {
        return executeRemoteCall(() -> mDroidAudioService.AudioEffect_setUserSoundModeParam(bandNumber, value, bandSum), "setUserSoundModeParam", 0);
    }
    public int getUserSoundModeParam(int bandNumber) {
        return executeRemoteCall(() -> mDroidAudioService.AudioEffect_getUserSoundModeParam(bandNumber), "getUserSoundModeParam", 0);
    }

    public static final int HPEQ_BAND_NUM_5                                                            = 5;
    public static final int HPEQ_BAND_NUM_7                                                            = 7;
    public static final int HPEQ_BAND_NUM_9                                                            = 9;
    public int setHpeqBandNum(int value) {
        return executeRemoteCall(() -> mDroidAudioService.AudioEffect_setHpeqBandNum(value), "setHpeqBandNum", 0);
    }
    public int getHpeqBandNum() {
        return executeRemoteCall(() -> mDroidAudioService.AudioEffect_getHpeqBandNum(), "getHpeqBandNum", 0);
    }

    public int setVirtualSurroundEnabled(boolean enable) {
        return executeRemoteCall(() -> mDroidAudioService.AudioEffect_setVirtualSurroundEnabled(enable), "setVirtualSurroundEnabled", 0);
    }
    public boolean isVirtualSurroundEnabled() {
        return executeRemoteCall(() -> mDroidAudioService.AudioEffect_isVirtualSurroundEnabled(), "isVirtualSurroundEnabled", false);
    }

    public boolean isDtsVXValidEnabled() {
        return executeRemoteCall(() -> mDroidAudioService.AudioEffect_isDtsVXValidEnabled(), "isDtsVXValidEnabled", false);
    }
    public int setDtsVirtualXEnabled(boolean enable) {
        return executeRemoteCall(() -> mDroidAudioService.AudioEffect_setDtsVirtualXEnabled(enable), "setDtsVirtualXEnabled", 0);
    }
    public boolean isDtsVirtualXEnabled() {
        return executeRemoteCall(() -> mDroidAudioService.AudioEffect_isDtsVirtualXEnabled(), "isDtsVirtualXEnabled", false);
    }
    public int setDtsVirtualSurroundEnabled(boolean enable) {
        return executeRemoteCall(() -> mDroidAudioService.AudioEffect_setDtsVirtualSurroundEnabled(enable), "setDtsVirtualSurroundEnabled", 0);
    }
    public boolean isDtsVirtualSurroundEnabled() {
        return executeRemoteCall(() -> mDroidAudioService.AudioEffect_isDtsVirtualSurroundEnabled(), "isDtsVirtualSurroundEnabled", false);
    }
    public int setDtsBassEnhancementEnabled(boolean enable) {
        return executeRemoteCall(() -> mDroidAudioService.AudioEffect_setDtsVirtualSurroundEnabled(enable), "setDtsBassEnhancementEnabled", 0);
    }
    public boolean isDtsBassEnhancementEnabled() {
        return executeRemoteCall(() -> mDroidAudioService.AudioEffect_isDtsBassEnhancementEnabled(), "isDtsBassEnhancementEnabled", false);
    }

    public static final int VIRTUALX_DIALOGCLARITY_MODE_OFF                                            = 0;
    public static final int VIRTUALX_DIALOGCLARITY_MODE_LOW                                            = 1;
    public static final int VIRTUALX_DIALOGCLARITY_MODE_MIDDLE                                         = 2;
    public static final int VIRTUALX_DIALOGCLARITY_MODE_HIGH                                           = 3;
    public static final int VIRTUALX_DIALOGCLARITY_MODE_MIN                                            = VIRTUALX_DIALOGCLARITY_MODE_OFF;
    public static final int VIRTUALX_DIALOGCLARITY_MODE_MAX                                            = VIRTUALX_DIALOGCLARITY_MODE_HIGH;
    public int setDtsDialogClarityMode(int mode) {
        return executeRemoteCall(() -> mDroidAudioService.AudioEffect_setDtsDialogClarityMode(mode), "setDtsDialogClarityMode", 0);
    }
    public int getDtsDialogClarityMode() {
        return executeRemoteCall(() -> mDroidAudioService.AudioEffect_getDtsDialogClarityMode(), "getDtsDialogClarityMode", 0);
    }

    public static final int VIRTUALX_MODE_OFF                                                          = 0;
    public static final int VIRTUALX_MODE_BASS                                                         = 1;
    public static final int VIRTUALX_MODE_FULL                                                         = 2;
    public int setDtsVirtualXMode(int mode) {
        return executeRemoteCall(() -> mDroidAudioService.AudioEffect_setDtsVirtualXMode(mode), "setDtsVirtualXMode", 0);
    }
    public int getDtsVirtualXMode() {
        return executeRemoteCall(() -> mDroidAudioService.AudioEffect_getDtsVirtualXMode(), "getDtsVirtualXMode", 0);
    }
    public int setDtsTruVolumeHdEnabled(boolean enable) {
        return executeRemoteCall(() -> mDroidAudioService.AudioEffect_setDtsTruVolumeHdEnabled(enable), "setDtsTruVolumeHdEnabled", 0);
    }
    public boolean isDtsTruVolumeHdEnabled() {
        return executeRemoteCall(() -> mDroidAudioService.AudioEffect_isDtsTruVolumeHdEnabled(), "isDtsTruVolumeHdEnabled", false);
    }

    public int setAISoundModeEnable(boolean enable) {
        return executeRemoteCall(() -> mDroidAudioService.AudioEffect_setAISoundModeEnable(enable), "setAISoundModeEnable", 0);
    }
    public boolean isAISoundModeEnabled() {
        return executeRemoteCall(() -> mDroidAudioService.AudioEffect_isAISoundModeEnabled(), "isAISoundModeEnabled", false);
    }

}
