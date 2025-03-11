/*
 * Copyright (c) 2019 Amlogic, Inc. All rights reserved.
 *
 * This source code is subject to the terms and conditions defined in the
 * file 'LICENSE' which is part of this source code package.
 *
 * Description: JAVA file
 */

package com.droidlogic.app;

import android.content.Context;
import android.content.Intent;
import android.content.ServiceConnection;
import android.content.ComponentName;
import android.os.IBinder;
import android.os.RemoteCallbackList;
import android.os.RemoteException;
import android.util.Log;

import com.droidlogic.audioservice.services.IAudioEffectsService;

public class AudioEffectManager {
    private String TAG = "AudioEffectManager";
    public static final String SERVICE_NANME = "com.droidlogic.audioservice.services.AudioEffectsService";
    private IAudioEffectsService mAudioEffectsService = null;
    private Context mContext;

    private boolean mDebug = true;
    private int RETRY_MAX = 10;

    /* [setSoundMode] EQ sound mode type */
    public static final int COMMON_SOUND_MODE_STANDARD                  = 0;
    public static final int COMMON_SOUND_MODE_MUSIC                     = 1;
    public static final int COMMON_SOUND_MODE_NEWS                      = 2;
    public static final int COMMON_SOUND_MODE_MOVIE                     = 3;
    public static final int COMMON_SOUND_MODE_GAME                      = 4;
    public static final int COMMON_SOUND_MODE_NIGHT                     = 5;
    public static final int COMMON_SOUND_MODE_CUSTOM                    = 6;


    /* [setUserSoundModeParam] custom sound mode EQ band type */
    public static final int EQ_SOUND_MODE_EFFECT_BAND1                  = 0;
    public static final int EQ_SOUND_MODE_EFFECT_BAND2                  = 1;
    public static final int EQ_SOUND_MODE_EFFECT_BAND3                  = 2;
    public static final int EQ_SOUND_MODE_EFFECT_BAND4                  = 3;
    public static final int EQ_SOUND_MODE_EFFECT_BAND5                  = 4;
    public static final int EQ_SOUND_MODE_EFFECT_BAND6                  = 5;
    public static final int EQ_SOUND_MODE_EFFECT_BAND7                  = 6;
    public static final int EQ_SOUND_MODE_EFFECT_BAND8                  = 7;
    public static final int EQ_SOUND_MODE_EFFECT_BAND9                  = 8;

    /* [setDtsVirtualXMode] VirtualX effect mode */
    public static final int SOUND_EFFECT_VIRTUALX_MODE_OFF              = 0;
    public static final int SOUND_EFFECT_VIRTUALX_MODE_BASS             = 1;
    public static final int SOUND_EFFECT_VIRTUALX_MODE_FULL             = 2;

    /* init value for first boot */
    public static final int EFFECT_BASS_DEFAULT                         = 50;   // 0 - 100
    public static final int EFFECT_TREBLE_DEFAULT                       = 50;   // 0 - 100
    public static final int EFFECT_BALANCE_DEFAULT                      = 50;   // 0 - 100

    public static final int SOUND_EFFECT_VIRTUAL_SURROUND_OFF           = 0;
    public static final int SOUND_EFFECT_VIRTUAL_SURROUND_ON            = 1;        // ON

    public static final int SOUND_EFFECT_VIRTUALX_MODE_DEFAULT          = SOUND_EFFECT_VIRTUALX_MODE_OFF;
    public static final int SOUND_EFFECT_TRUVOLUME_HD_ENABLE_DEFAULT    = 0;        // OFF
    public static final int SOUND_EFFECT_VIRTUALX_DIALOG_MODE_DEFAULT   = 0;        // OFF
    public static final int SOUND_EFFECT_VIRTUALX_TRU_BASS_DEFAULT      = 0;        // OFF
    public static final int SOUND_EFFECT_VIRTUAL_SURROUND_DEFAULT       = 0;        // OFF

    /****************************DAP effect cmd*******************************/
    public static final int SOUND_EFFECT_DAP_VERSION_1_3_2  = 0;
    public static final int SOUND_EFFECT_DAP_VERSION_2_4    = 1;
    public static final int SOUND_EFFECT_DAP_VERSION        = SOUND_EFFECT_DAP_VERSION_2_4;

    /* [DAP 1.3.2] */
    public static final int CMD_DAP_1_3_2_BASE_VALUE        = 0;
    public static final int CMD_DAP_ENABLE                  = 0;
    public static final int CMD_DAP_EFFECT_MODE             = 1;
    public static final int CMD_DAP_GEQ_GAINS               = 2;
    public static final int CMD_DAP_GEQ_ENABLE              = 3;
    public static final int CMD_DAP_POST_GAIN               = 4;
    public static final int CMD_DAP_VL_ENABLE               = 5;
    public static final int CMD_DAP_VL_AMOUNT               = 6;
    public static final int CMD_DAP_DE_ENABLE               = 7;
    public static final int CMD_DAP_DE_AMOUNT               = 8;
    public static final int CMD_DAP_SURROUND_ENABLE         = 9;
    public static final int CMD_DAP_SURROUND_BOOST          = 10;
    public static final int CMD_DAP_VIRTUALIZER_ENABLE      = 11;

    public static final int SUBCMD_DAP_GEQ_BAND1            = 0x100;
    public static final int SUBCMD_DAP_GEQ_BAND2            = 0x101;
    public static final int SUBCMD_DAP_GEQ_BAND3            = 0x102;
    public static final int SUBCMD_DAP_GEQ_BAND4            = 0x103;
    public static final int SUBCMD_DAP_GEQ_BAND5            = 0x104;

    /* [DAP 2.4] */
    public static final int CMD_DAP_2_4_BASE_VALUE                      = 1000;

    public static final int CMD_DAP_2_4_PROFILE                         = 1000;
    public static final int CMD_DAP_2_4_SURROUND_VIRTUALIZER            = 1004;
    public static final int CMD_DAP_2_4_DIALOGUE_ENHANCER               = 1007;
    public static final int CMD_DAP_2_4_BASS_ENHANCER                   = 1008;
    public static final int CMD_DAP_2_4_MI_STEERING                     = 1012;
    public static final int CMD_DAP_2_4_SURROUND_DECODER_ENABLE         = 1013;
    public static final int CMD_DAP_2_4_LEVELER                         = 1015;

    public static final int SUBCMD_DAP_2_4_BASE_VALUE                   = 2000;

    public static final int SUBCMD_DAP_2_4_SURROUND_VIRTUALIZER_BOOST   = 2000;
    public static final int SUBCMD_DAP_2_4_BASS_ENHANCER_BOOST          = 2002;
    public static final int SUBCMD_DAP_2_4_BASS_ENHANCER_CUTOFF_100     = 2003;
    public static final int SUBCMD_DAP_2_4_BASS_ENHANCER_CUTOFF_1       = 2004;
    public static final int SUBCMD_DAP_2_4_BASS_ENHANCER_WIDTH          = 2005;
    public static final int SUBCMD_DAP_2_4_LEVELER_AMOUNT               = 2006;

    /*************************************************************************/

    /* [CMD_DAP_2_4_PROFILE] sound mode */
    public static final int SOUND_EFFECT_DAP_2_4_PROFILE_MOVIE           = 0;
    public static final int SOUND_EFFECT_DAP_2_4_PROFILE_MUSIC           = 1;
    public static final int SOUND_EFFECT_DAP_2_4_PROFILE_GAME            = 2;
    public static final int SOUND_EFFECT_DAP_2_4_PROFILE_NIGHT           = 3;
    public static final int SOUND_EFFECT_DAP_2_4_PROFILE_VOICE           = 4;
    public static final int SOUND_EFFECT_DAP_2_4_PROFILE_USER_SELECTABLE = 5;
    public static final int SOUND_EFFECT_DAP_2_4_PROFILE_OFF             = 6;
    public static final int SOUND_EFFECT_DAP_2_4_PROFILE_MIN             = SOUND_EFFECT_DAP_2_4_PROFILE_MOVIE;
    public static final int SOUND_EFFECT_DAP_2_4_PROFILE_MAX             = SOUND_EFFECT_DAP_2_4_PROFILE_OFF;

    public static final int SOUND_EFFECT_DAP_2_4_SURROUND_VIRTUALIZER_OFF      = 0;
    public static final int SOUND_EFFECT_DAP_2_4_SURROUND_VIRTUALIZER_ON       = 1;
    public static final int SOUND_EFFECT_DAP_2_4_SURROUND_VIRTUALIZER_AUTO     = 2;

    public static final int SOUND_EFFECT_DAP_2_4_LEVELER_OFF                   = 0;
    public static final int SOUND_EFFECT_DAP_2_4_LEVELER_ON                    = 1;
    public static final int SOUND_EFFECT_DAP_2_4_LEVELER_AUTO                  = 2;


    /* DAP 2.4 default setting */
    public static final int DAP_2_4_SURROUND_VIRTUALIZER_DEFAULT               = 0;
    public static final int DAP_2_4_SURROUND_VIRTUALIZER_BOOST_DEFAULT         = 96;
    public static final int DAP_2_4_DIALOGUE_ENHANCER_DEFAULT                  = 0;
    public static final int DAP_2_4_DIALOGUE_ENHANCER_AMOUNT_DEFAULT           = 0;
    public static final int DAP_2_4_BASS_ENHANCER_DEFAULT                      = 0;
    public static final int DAP_2_4_BASS_ENHANCER_BOOST_DEFAULT                = 192;
    public static final int DAP_2_4_BASS_ENHANCER_CUTOFFX100_DEFAULT           = 2;//X100
    public static final int DAP_2_4_BASS_ENHANCER_CUTOFFX1_DEFAULT             = 0;//X1
    public static final int DAP_2_4_BASS_ENHANCER_WIDTH_DEFAULT                = 16;
    public static final int DAP_2_4_MI_STEERING_DEFAULT                        = 0;
    public static final int DAP_2_4_SURROUND_DECODER_ENABLE_DEFAULT            = 1;
    public static final int DAP_2_4_LEVELER_DEFAULT                            = 0;
    public static final int DAP_2_4_LEVELER_AMOUNT_DEFAULT                     = 4;

    /* DAP 1.3.2 sound mode */
    public static final int DAP_MODE_OFF                    = 0;
    public static final int DAP_MODE_MOVIE                  = 1;
    public static final int DAP_MODE_MUSIC                  = 2;
    public static final int DAP_MODE_NIGHT                  = 3;
    public static final int DAP_MODE_USER                   = 4;
    public static final int DAP_MODE_DEFAULT                = DAP_MODE_MUSIC;

    public static final int DAP_SURROUND_SPEAKER            = 0;
    public static final int DAP_SURROUND_HEADPHONE          = 1;
    public static final int DAP_SURROUND_DEFAULT            = DAP_SURROUND_SPEAKER;

    public static final int DAP_GEQ_OFF                     = 0;
    public static final int DAP_GEQ_INIT                    = 1;
    public static final int DAP_GEQ_OPEN                    = 2;
    public static final int DAP_GEQ_RICH                    = 3;
    public static final int DAP_GEQ_FOCUSED                 = 4;
    public static final int DAP_GEQ_USER                    = 5;
    public static final int DAP_GEQ_DEFAULT                 = DAP_GEQ_INIT;

    public static final int DAP_OFF                         = 0;
    public static final int DAP_ON                          = 1;

    public static final int DAP_VL_DEFAULT                  = DAP_ON;
    public static final int DAP_VL_AMOUNT_DEFAULT           = 0;
    public static final int DAP_DE_DEFAULT                  = DAP_OFF;
    public static final int DAP_DE_AMOUNT_DEFAULT           = 0;
    public static final int DAP_SURROUND_BOOST_DEFAULT      = 0;
    public static final int DAP_POST_GAIN_DEFAULT           = 0;
    public static final int DAP_GEQ_GAIN_DEFAULT            = 0;

    //dpe effect param define
    public static final int CMD_DPE_ENABLED                                  = 48;
    public static final int CMD_DPE_INPUTGAIN                                = 32;
    public static final int CMD_DPE_PRE_EQ                                   = 64;
    public static final int CMD_DPE_PRE_EQ_BAND                              = 69;
    public static final int CMD_DPE_MBC                                      = 80;
    public static final int CMD_DPE_MBC_BAND                                 = 85;
    public static final int CMD_DPE_POST_EQ                                  = 96;
    public static final int CMD_DPE_POST_EQ_BAND                             = 101;
    public static final int CMD_DPE_LIMITER                                  = 112;

    // dpe effect sub pre eq band 0 param define
    public static final int SUBCMD_DPE_PRE_EQ_BAND0                          = 100000;
    public static final int SUBCMD_DPE_PRE_EQ_BAND0_CUTOFFFREQUENCY          = 10000;
    public static final int SUBCMD_DPE_PRE_EQ_BAND0_GAIN                     = 10001;
    // dpe effect sub pre eq band 1 param define
    public static final int SUBCMD_DPE_PRE_EQ_BAND1                          = 100100;
    public static final int SUBCMD_DPE_PRE_EQ_BAND1_CUTOFFFREQUENCY          = 10010;
    public static final int SUBCMD_DPE_PRE_EQ_BAND1_GAIN                     = 10011;
    // dpe effect sub pre eq band 2 param define
    public static final int SUBCMD_DPE_PRE_EQ_BAND2                          = 100200;
    public static final int SUBCMD_DPE_PRE_EQ_BAND2_CUTOFFFREQUENCY          = 10020;
    public static final int SUBCMD_DPE_PRE_EQ_BAND2_GAIN                     = 10021;

    // dpe effect sub mbc param band 0 define
    public static final int SUBCMD_DPE_MBC_BAND0                             = 110000;
    public static final int SUBCMD_DPE_MBC_BAND0_CUTOFFFREQUENCY             = 11000;
    public static final int SUBCMD_DPE_MBC_BAND0_ATTACKTIME                  = 11001;
    public static final int SUBCMD_DPE_MBC_BAND0_RELEASETIME                 = 11002;
    public static final int SUBCMD_DPE_MBC_BAND0_RATIO                       = 11003;
    public static final int SUBCMD_DPE_MBC_BAND0_THRESHOLD                   = 11004;
    public static final int SUBCMD_DPE_MBC_BAND0_KNEEWIDTH                   = 11005;
    public static final int SUBCMD_DPE_MBC_BAND0_NOISEGATETHRESHOLD          = 11006;
    public static final int SUBCMD_DPE_MBC_BAND0_EXPANDERRATIO               = 11007;
    public static final int SUBCMD_DPE_MBC_BAND0_PREGAIN                     = 11008;
    public static final int SUBCMD_DPE_MBC_BAND0_POSTGAIN                    = 11009;
    // dpe effect sub mbc param band 1 define
    public static final int SUBCMD_DPE_MBC_BAND1                             = 110100;
    public static final int SUBCMD_DPE_MBC_BAND1_CUTOFFFREQUENCY             = 11010;
    public static final int SUBCMD_DPE_MBC_BAND1_ATTACKTIME                  = 11011;
    public static final int SUBCMD_DPE_MBC_BAND1_RELEASETIME                 = 11012;
    public static final int SUBCMD_DPE_MBC_BAND1_RATIO                       = 11013;
    public static final int SUBCMD_DPE_MBC_BAND1_THRESHOLD                   = 11014;
    public static final int SUBCMD_DPE_MBC_BAND1_KNEEWIDTH                   = 11015;
    public static final int SUBCMD_DPE_MBC_BAND1_NOISEGATETHRESHOLD          = 11016;
    public static final int SUBCMD_DPE_MBC_BAND1_EXPANDERRATIO               = 11017;
    public static final int SUBCMD_DPE_MBC_BAND1_PREGAIN                     = 11018;
    public static final int SUBCMD_DPE_MBC_BAND1_POSTGAIN                    = 11019;
    // dpe effect sub mbc param band 2 define
    public static final int SUBCMD_DPE_MBC_BAND2                             = 110200;
    public static final int SUBCMD_DPE_MBC_BAND2_CUTOFFFREQUENCY             = 11020;
    public static final int SUBCMD_DPE_MBC_BAND2_ATTACKTIME                  = 11021;
    public static final int SUBCMD_DPE_MBC_BAND2_RELEASETIME                 = 11022;
    public static final int SUBCMD_DPE_MBC_BAND2_RATIO                       = 11023;
    public static final int SUBCMD_DPE_MBC_BAND2_THRESHOLD                   = 11024;
    public static final int SUBCMD_DPE_MBC_BAND2_KNEEWIDTH                   = 11025;
    public static final int SUBCMD_DPE_MBC_BAND2_NOISEGATETHRESHOLD          = 11026;
    public static final int SUBCMD_DPE_MBC_BAND2_EXPANDERRATIO               = 11027;
    public static final int SUBCMD_DPE_MBC_BAND2_PREGAIN                     = 11028;
    public static final int SUBCMD_DPE_MBC_BAND2_POSTGAIN                    = 11029;

    // dpe effect sub post eq band 0 param define
    public static final int SUBCMD_DPE_POST_EQ_BAND0                         = 120000;
    public static final int SUBCMD_DPE_POST_EQ_BAND0_CUTOFFFREQUENCY         = 12000;
    public static final int SUBCMD_DPE_POST_EQ_BAND0_GAIN                    = 12001;
    // dpe effect sub post eq band 1 param define
    public static final int SUBCMD_DPE_POST_EQ_BAND1                         = 120100;
    public static final int SUBCMD_DPE_POST_EQ_BAND1_CUTOFFFREQUENCY         = 12010;
    public static final int SUBCMD_DPE_POST_EQ_BAND1_GAIN                    = 12011;
    // dpe effect sub post eq band 2 param define
    public static final int SUBCMD_DPE_POST_EQ_BAND2                         = 120200;
    public static final int SUBCMD_DPE_POST_EQ_BAND2_CUTOFFFREQUENCY         = 12020;
    public static final int SUBCMD_DPE_POST_EQ_BAND2_GAIN                    = 12021;

    //dpe limiter sub  param
    public static final int SUBCMD_DPE_LIMITER_ATTACKTIME                    = 13001;
    public static final int SUBCMD_DPE_LIMITER_RELEASETIME                   = 13002;
    public static final int SUBCMD_DPE_LIMITER_RATIO                         = 13003;
    public static final int SUBCMD_DPE_LIMITER_THRESHOLD                     = 13004;
    public static final int SUBCMD_DPE_LIMITER_POSTGAIN                      = 13005;

    //dpe default value
    public static final int DEFAULT_DPE_CHANNEL_AMOUNT                       = 2;
    public static final int DEFAULT_DPE_BAND_AMOUNT                          = 3;
    public static final float DEFAULT_DPE_FRAME_DURATION                     = 5f;
    public static final int DEFAULT_DPE_ENABLE                               = 0;
    public static final int DEFAULT_DPE_INPUTGAIN                            = 0;     //db
    public static final int DEFAULT_DPE_BAND0_CUTOFFFREQUENCY                = 3000;  //HZ
    public static final int DEFAULT_DPE_BAND1_CUTOFFFREQUENCY                = 6000;
    public static final int DEFAULT_DPE_BAND2_CUTOFFFREQUENCY                = 9000;
    public static final int DEFAULT_DPE_EQ_GAIN                              = 0;     //db
    public static final int DEFAULT_DPE_ATTACKTIME                           = 150;    //ms
    public static final int DEFAULT_DPE_RELEASETIME                          = 320;   //ms
    public static final int DEFAULT_DPE_MBC_RATIO                            = 1;
    public static final int DEFAULT_DPE_MBC_THRESHOLD                        = -10;  //dBFS
    public static final int DEFAULT_DPE_MBC_KNEEWIDTH                        = 0;    //db
    public static final int DEFAULT_DPE_MBC_NOISEGATETHRESHOLD               = -80;  //dBFS
    public static final int DEFAULT_DPE_MBC_EXPANDERRATIO                    = 1;
    public static final int DEFAULT_DPE_MBC_PREGAIN                          = 0;    //db
    public static final int DEFAULT_DPE_MBC_POSTGAIN                         = 0;    //db
    public static final int DEFAULT_DPE_LIMITER_RATIO                        = 30;
    public static final int DEFAULT_DPE_LIMITER_THRESHOLD                    = -25;  //dBFS
    public static final int DEFAULT_DPE_LIMITER_POSTGAIN                     = 0;    //db

    public static final int DPE_ON                                           = 1;
    public static final int DPE_OFF                                          = 0;

    public static final int DPE_PRE_EQ_OFF                                   = 0;
    public static final int DPE_PRE_EQ_ON                                    = 1;
    public static final int DPE_POST_EQ_OFF                                  = 0;
    public static final int DPE_POST_EQ_ON                                   = 1;
    public static final int DPE_MBC_OFF                                      = 0;
    public static final int DPE_MBC_ON                                       = 1;
    public static final int DPE_LIMITER_OFF                                  = 0;
    public static final int DPE_LIMITER_ON                                   = 1;

    //debug audio UI
    public static final int DEBUG_UI_ON                                      = 1;
    public static final int DEBUG_UI_OFF                                     = 0;
    public static final int EFFECT_UI_ON                                     = 1;
    public static final int EFFECT_UI_OFF                                    = 0;

    public static final int HPEQ_5_BAND                                      = 5;
    public static final int HPEQ_7_BAND                                      = 7;
    public static final int HPEQ_9_BAND                                      = 9;


    public static final int EFFECT_HPEQ_UI_ID                                = 0;
    public static final int EFFECT_BALANCE_UI_ID                             = 1;
    public static final int EFFECT_TREBLEBASS_UI_ID                          = 2;
    public static final int EFFECT_VIRTUAL_SURROUND_UI_ID                    = 3;
    public static final int EFFECT_DPE_UI_ID                                 = 4;
    public static final int EFFECT_VIRTUALX_UI_ID                            = 5;
    public static final int EFFECT_DAP2_UI_ID                                = 6;
    public static final int EFFECT_HPEQ_BAND_UI_ID                           = 7;
    public static final int DOLBY_DRC_UI_ID                                  = 8;
    public static final int DTS_DRC_UI_ID                                    = 9;
    public static final int ENGINEER_MODE_UI_ID                              = 10;
    public static final int AUDIO_LATENCY_UI_ID                              = 11;
    public static final int FORCE_DDP_UI_ID                                  = 12;
    public static final int PASSTHROUGH_UI_ID                                = 13;
    public static final int VAD_UI_ID                                        = 14;
    public static final int FUNCTION_UI_NUM                                  = 15;


    /*
    * Dolby ms12 audio config
    * -N: Not support MS12
    * -X: support MS12, config = x
    * -Y: support MS12, config = y
    * -X: support MS12, config = z
    */
    public static final int DOLBY_MS12_AUDIO_CONFIG_N                        = -1;
    public static final int DOLBY_MS12_AUDIO_CONFIG_X                        = 1;
    public static final int DOLBY_MS12_AUDIO_CONFIG_Y                        = 0;
    public static final int DOLBY_MS12_AUDIO_CONFIG_Z                        = 2;
    //DTS audio config
    public static final int DTS_VIRTUALX_AUDIO_CONFIG_ON                     = 1;
    public static final int DTS_VIRTUALX_AUDIO_CONFIG_OFF                    = 0;
    //Dual Effect mode
    public static final int EFFECT_MODE_AUTO                                 = 0;
    public static final int EFFECT_MODE_DTS                                  = 1;
    public static final int EFFECT_MODE_DOLBY                                = 2;
    public static final int EFFECT_MODE_OFF                                  = 3;
    public static final int EFFECT_MODE_MAX                                  = 4;
    //Basic Effect mode
    public static final int BASIC_EFFECT_MODE_OFF                            = 0;
    public static final int BASIC_EFFECT_MODE_ON                             = 1;

    private static AudioEffectManager mInstance;

    public static AudioEffectManager getInstance(Context context) {
        if (null == mInstance) {
            mInstance = new AudioEffectManager(context);
        }
        return mInstance;
    }

    public AudioEffectManager(Context context) {
        mContext = context;
        LOGI("construction AudioEffectManager");
        getService();
    }

    private void LOGI(String msg) {
        if (mDebug) Log.i(TAG, msg);
    }

    private void getService() {
        LOGI("=====[getService]");
        int retry = RETRY_MAX;
        boolean mIsBind = false;
        try {
            synchronized (this) {
                while (true) {
                    Intent intent = new Intent();
                    intent.setAction(SERVICE_NANME);
                    intent.setPackage("com.droidlogic");
                    mIsBind = mContext.bindService(intent, mContext.BIND_AUTO_CREATE, Runnable::run, serConn);
                    LOGI("=====[getService] mIsBind: " + mIsBind + ", retry:" + retry);
                    if (mIsBind || retry <= 0) {
                        break;
                    }
                    retry --;
                    Thread.sleep(500);
                }
            }
        } catch (InterruptedException e){}
    }

    private ServiceConnection serConn = new ServiceConnection() {
        @Override
        public void onServiceDisconnected(ComponentName name) {
            LOGI("[onServiceDisconnected] mAudioEffectsService: " + mAudioEffectsService);
            mAudioEffectsService = null;

        }
        @Override
        public void onServiceConnected(ComponentName name, IBinder service) {
            mAudioEffectsService = IAudioEffectsService.Stub.asInterface(service);
            LOGI("[onServiceConnected] mAudioEffectsService: " + mAudioEffectsService);
        }
    };

    public static String debugUiIndexToString(int value) {
        String temp = "[" + value + "]";
        switch (value) {
            case EFFECT_HPEQ_UI_ID:
                return temp + "HPEQ";
            case EFFECT_BALANCE_UI_ID:
                return temp + "BALANCE";
            case EFFECT_TREBLEBASS_UI_ID:
                return temp + "TREBLEBASS";
            case EFFECT_VIRTUAL_SURROUND_UI_ID:
                return temp + "VIRTUAL_SURROUND";
            case EFFECT_DPE_UI_ID:
                return temp + "DPE";
            case EFFECT_VIRTUALX_UI_ID:
                return temp + "VIRTUAL_X";
            case EFFECT_DAP2_UI_ID:
                return temp + "DAP_2";
            case EFFECT_HPEQ_BAND_UI_ID:
                return temp + "HPEQ_BAND_NUM";
            default:
                return temp + "invalid value";
        }
    }

    public void unBindService() {
        mContext.unbindService(serConn);
    }

    private boolean audioEffectServiceIsNull() {
        if (mAudioEffectsService == null) {
            Log.w(TAG, "mAudioEffectsService is null!");
            Log.w(TAG, Log.getStackTraceString(new Throwable()));
            return true;
        } else {
            return false;
        }
    }

    public void init() {
        if (audioEffectServiceIsNull()) return;
        try {
            mAudioEffectsService.init();
        } catch (RemoteException e) {
            Log.e(TAG, "init failed:" + e);
        }
    }

    public void deinit() {
        if (audioEffectServiceIsNull()) return;
        try {
            mAudioEffectsService.deinit();
        } catch (RemoteException e) {
            Log.e(TAG, "deinit failed:" + e);
        }
    }

    public void reset() {
        if (audioEffectServiceIsNull()) return;
        try {
            mAudioEffectsService.reset();
        } catch (RemoteException e) {
            Log.e(TAG, "reset failed:" + e);
        }
    }

    public int getSoundModeStatus() {
        if (audioEffectServiceIsNull()) return 0;
        try {
            return mAudioEffectsService.getSoundModeStatus();
        } catch (RemoteException e) {
            Log.e(TAG, "getSoundModeStatus failed:" + e);
        }
        return -1;
    }

    public void setSoundMode(int mode) {
        if (audioEffectServiceIsNull()) return;
        try {
            mAudioEffectsService.setSoundMode(mode);
        } catch (RemoteException e) {
            Log.e(TAG, "setSoundMode failed:" + e);
        }
    }

    public void setUserSoundModeParam(int bandNumber, int value, int bandSum) {
        if (audioEffectServiceIsNull()) return;
        try {
            mAudioEffectsService.setUserSoundModeParam(bandNumber, value, bandSum);
        } catch (RemoteException e) {
            Log.e(TAG, "setUserSoundModeParam failed:" + e);
        }
    }

    public int getUserSoundModeParam(int bandNumber) {
        if (audioEffectServiceIsNull()) return 0;
        try {
            return mAudioEffectsService.getUserSoundModeParam(bandNumber);
        } catch (RemoteException e) {
            Log.e(TAG, "getUserSoundModeParam failed:" + e);
        }
        return -1;
    }

    //create/release the specified effect by Effect Type: id
    public void setAudioEffectOn(int id, boolean dbSwitch) {
        if (audioEffectServiceIsNull()) return;
        try {
            mAudioEffectsService.setAudioEffectOn(id, dbSwitch);
        } catch (RemoteException e) {
            Log.e(TAG, "setAudioEffectOn failed:" + e);
        }
    }

    //whether is the specified effect enabled
    public boolean isAudioEffectOn(int id) {
        if (audioEffectServiceIsNull()) return false;
        try {
            return mAudioEffectsService.isAudioEffectOn(id);
        } catch (RemoteException e) {
            Log.e(TAG, "isAudioEffectOn failed:" + e);
        }
        return false;
    }

    public boolean isSupportVirtualX() {
        if (audioEffectServiceIsNull()) return false;
        try {
            return mAudioEffectsService.isSupportVirtualX();
        } catch (RemoteException e) {
            Log.e(TAG, "isSupportVirtualX failed:" + e);
        }
        return false;
    }

    //1.Virtualx parameter set/get
    public void setDtsVirtualXMode(int virtualXMode) {
        if (audioEffectServiceIsNull()) return;
        try {
            mAudioEffectsService.setDtsVirtualXMode(virtualXMode);
        } catch (RemoteException e) {
            Log.e(TAG, "setDtsVirtualXMode failed:" + e);
        }
    }

    public int getDtsVirtualXMode() {
        if (audioEffectServiceIsNull()) return 0;
        try {
            return mAudioEffectsService.getDtsVirtualXMode();
        } catch (RemoteException e) {
            Log.e(TAG, "getDtsVirtualXMode failed:" + e);
        }
        return -1;
    }

    public void setDtsTruVolumeHdEnable(boolean enable) {
        if (audioEffectServiceIsNull()) return;
        try {
            mAudioEffectsService.setDtsTruVolumeHdEnable(enable);
        } catch (RemoteException e) {
            Log.e(TAG, "setDtsTruVolumeHdEnable failed:" + e);
        }
    }

    public boolean getDtsTruVolumeHdEnable() {
        if (audioEffectServiceIsNull()) return false;
        try {
            return mAudioEffectsService.getDtsTruVolumeHdEnable();
        } catch (RemoteException e) {
            Log.e(TAG, "getDtsTruVolumeHdEnable failed:" + e);
        }
        return false;
    }

    //2.TrebleBass parameter set/get
    public int getTrebleStatus() {
        if (audioEffectServiceIsNull()) return 0;
        try {
            return mAudioEffectsService.getTrebleStatus();
        } catch (RemoteException e) {
            Log.e(TAG, "getTrebleStatus failed:" + e);
        }
        return -1;
    }

    public int getBassStatus() {
        if (audioEffectServiceIsNull()) return 0;
        try {
            return mAudioEffectsService.getBassStatus();
        } catch (RemoteException e) {
            Log.e(TAG, "getBassStatus failed:" + e);
        }
        return -1;
    }

    public void setTreble(int step) {
        if (audioEffectServiceIsNull()) return;
        try {
            mAudioEffectsService.setTreble(step);
        } catch (RemoteException e) {
            Log.e(TAG, "setTreble failed:" + e);
        }
    }

    public void setBass(int step) {
        if (audioEffectServiceIsNull()) return;
        try {
            mAudioEffectsService.setBass(step);
        } catch (RemoteException e) {
            Log.e(TAG, "setBass failed:" + e);
        }
    }

    //3.Balance parameter set/get
    public void setBalance(int step) {
        if (audioEffectServiceIsNull()) return;
        try {
            mAudioEffectsService.setBalance(step);
        } catch (RemoteException e) {
            Log.e(TAG, "setBalance failed:" + e);
        }
    }

    public int getBalanceStatus() {
        if (audioEffectServiceIsNull()) return 0;
        try {
            return mAudioEffectsService.getBalanceStatus();
        } catch (RemoteException e) {
            Log.e(TAG, "getBalanceStatus failed:" + e);
        }
        return -1;
    }

    //4.Virtual parameter set/get
    public int getVirtualSurroundStatus() {
        if (audioEffectServiceIsNull()) return 0;
        try {
            return mAudioEffectsService.getVirtualSurroundStatus();
        } catch (RemoteException e) {
            Log.e(TAG, "getVirtualSurroundStatus failed:" + e);
        }
        return -1;
    }

    public void setVirtualSurround(int mode) {
        if (audioEffectServiceIsNull()) return;
        try {
            mAudioEffectsService.setVirtualSurround(mode);
        } catch (RemoteException e) {
            Log.e(TAG, "setVirtualSurround failed:" + e);
        }
    }

    //5.DAP parameter set/get
    public void setDapParam(int id, int value) {
        if (audioEffectServiceIsNull()) return;
        try {
            mAudioEffectsService.setDapParam(id, value);
        } catch (RemoteException e) {
            Log.e(TAG, "setDapParam failed:" + e);
        }
    }

    public int getDapParam(int id) {
        if (audioEffectServiceIsNull()) return 0;
        try {
            return mAudioEffectsService.getDapParam(id);
        } catch (RemoteException e) {
            Log.e(TAG, "getDapParam failed:" + e);
        }
        return 0;
    }

    public void initDapAudioEffect() {
        if (audioEffectServiceIsNull()) return;
        try {
            mAudioEffectsService.initDapAudioEffect();
        } catch (RemoteException e) {
            Log.e(TAG, "initDapAudioEffect failed:" + e);
        }
    }

    //6.DPE parameter set/get
    public void initDpeAudioEffect() {
        if (audioEffectServiceIsNull()) return;
        try {
            mAudioEffectsService.initDpeAudioEffect();
        } catch (RemoteException e) {
            Log.e(TAG, "initDpeAudioEffect failed:" + e);
        }
    }

    public void setDpeParam(int id, int value) {
        if (audioEffectServiceIsNull()) return;
        try {
            mAudioEffectsService.setDpeParam(id, value);
        } catch (RemoteException e) {
            Log.e(TAG, "setDpeParam failed:" + e);
        }
    }

    public int getDpeParam(int id) {
        if (audioEffectServiceIsNull()) return 0;
        try {
            return mAudioEffectsService.getDpeParam(id);
        } catch (RemoteException e) {
            Log.e(TAG, "getDpeParam failed:" + e);
        }
        return 0;
    }

    //7.HPEQ parameter set/get
    public void setHpeqBandNum(int id, int value) {
        if (audioEffectServiceIsNull()) return;
        try {
            mAudioEffectsService.setHpeqBandNum(id, value);
        } catch (RemoteException e) {
            Log.e(TAG, "setHpeqBandNum failed:" + e);
        }
    }

    public int getHpeqBandNum(int id) {
        if (audioEffectServiceIsNull()) return 0;
        try {
            return mAudioEffectsService.getHpeqBandNum(id);
        } catch (RemoteException e) {
            Log.e(TAG, "getHpeqBandNum failed:" + e);
        }
        return 0;
    }


    public static class HPEQ {
        public static final int CMD_HPEQ_PARAM_ENABLE = 0;

        static void init() {
            mInstance.setAudioEffectOn(EFFECT_HPEQ_UI_ID, true);
        }

        static void deInit() {
            mInstance.setAudioEffectOn(EFFECT_HPEQ_UI_ID, true);
        }

        static void setHpeqBandNum(int id, int value) {
            mInstance.setHpeqBandNum(id, value);
        }

        static int getHpeqBandNum(int id) {
            return mInstance.getHpeqBandNum(id);
        }
    }

    public static class Balance {
        public static final int CMD_BALANCE_PARAM_ENABLE = 1;

    }

    public static class TrebleBass {
        public static final int CMD_TREBLEBASS_PARAM_ENABLE = 2;
    }


    public void setDtsVirtualXStatus(int enable) {
        if (audioEffectServiceIsNull()) return;
        try {
            mAudioEffectsService.setDtsVirtualXStatus(enable);
        } catch (RemoteException e) {
            Log.e(TAG, "setDtsVirtualXStatus failed:" + e);
        }
    }

    public int getDtsVirtualXStatus() {
        if (audioEffectServiceIsNull()) return 0;
        try {
            return mAudioEffectsService.getDtsVirtualXStatus();
        } catch (RemoteException e) {
            Log.e(TAG, "setDtsVirtualXStatus failed:" + e);
        }
        return 0;
    }

    public void setDtsVirtualSurround(int enable) {
        if (audioEffectServiceIsNull()) return;
        try {
            mAudioEffectsService.setDtsVirtualSurround(enable);
        } catch (RemoteException e) {
            Log.e(TAG, "setDtsVirtualXStatus failed:" + e);
        }
    }

    public int getDtsVirtualSurround() {
        if (audioEffectServiceIsNull()) return 0;
        try {
            return mAudioEffectsService.getDtsVirtualSurround();
        } catch (RemoteException e) {
            Log.e(TAG, "setDtsVirtualXStatus failed:" + e);
        }
        return 0;
    }

    public void setDtsDialogClarityMode(int mode) {
        if (audioEffectServiceIsNull()) return;
        try {
            mAudioEffectsService.setDtsDialogClarityMode(mode);
        } catch (RemoteException e) {
            Log.e(TAG, "setDtsVirtualXStatus failed:" + e);
        }
    }

    public int getDtsDialogClarityMode() {
        if (audioEffectServiceIsNull()) return 0;
        try {
            return mAudioEffectsService.getDtsDialogClarityMode();
        } catch (RemoteException e) {
            Log.e(TAG, "setDtsVirtualXStatus failed:" + e);
        }
        return 0;
    }

    public void setDtsBassEnhancement(boolean enable) {
        if (audioEffectServiceIsNull()) return;
        try {
            mAudioEffectsService.setDtsBassEnhancement(enable);
        } catch (RemoteException e) {
            Log.e(TAG, "setDtsVirtualXStatus failed:" + e);
        }
    }

    public int getDtsBassEnhancement() {
        if (audioEffectServiceIsNull()) return 0;
        try {
            return mAudioEffectsService.getDtsBassEnhancement();
        } catch (RemoteException e) {
            Log.e(TAG, "setDtsVirtualXStatus failed:" + e);
        }
        return 0;
    }

    public static class VirtualX {
        public static final int SOUND_EFFECT_VIRTUALX_VERSION_1         = 0;
        public static final int SOUND_EFFECT_VIRTUALX_VERSION_4         = 1;
        public static final int SOUND_EFFECT_VIRTUALX_VERSION           = SOUND_EFFECT_VIRTUALX_VERSION_4;

        // Definition of virtualx command values
        public static final int CMD_DTS_MBHL_ENABLE_I32                 = 0;
        public static final int CMD_DTS_TBHDX_ENABLE_I32                = 35;
        public static final int CMD_DTS_VX_ENABLE_I32                   = 46;
        public static final int CMD_DTS_LOUDNESS_CONTROL_ENABLE_I32     = 67;
        public static final int CMD_DTS_ENABLE_V4                       = 82;
        public static final int CMD_DIALOGCLARITY_MODE                  = 83;
        public static final int CMD_SURROUND_MODE                       = 84;
        public static final int CMD_DTS_VIRTUALX_USER_MODE              = 96;
        public static final int CMD_TBHDX_PROCESS_DISCARD_I32           = 182;
        // Definition of virtualx enum values
        public static final int PARAM_DIALOGCLARITY_MODE_OFF            = 0;
        public static final int PARAM_DIALOGCLARITY_MODE_LOW            = 1;
        public static final int PARAM_DIALOGCLARITY_MODE_MEDIUM         = 2;
        public static final int PARAM_DIALOGCLARITY_MODE_HIGH           = 3;
    }

    public static class DAP {
        public static final int CMD_DAP_ENABLE                           = 16;

        public static int toDapProfileID(int mode) {
            int ret;
            switch (mode) {
                case AudioEffectManager.COMMON_SOUND_MODE_STANDARD:
                    ret = AudioEffectManager.SOUND_EFFECT_DAP_2_4_PROFILE_OFF;
                    break;
                case AudioEffectManager.COMMON_SOUND_MODE_MUSIC:
                    ret = AudioEffectManager.SOUND_EFFECT_DAP_2_4_PROFILE_MUSIC;
                    break;
                case AudioEffectManager.COMMON_SOUND_MODE_GAME:
                    ret = AudioEffectManager.SOUND_EFFECT_DAP_2_4_PROFILE_GAME;
                    break;
                case AudioEffectManager.COMMON_SOUND_MODE_MOVIE:
                    ret = AudioEffectManager.SOUND_EFFECT_DAP_2_4_PROFILE_MOVIE;
                    break;
                case AudioEffectManager.COMMON_SOUND_MODE_CUSTOM:
                    ret = AudioEffectManager.SOUND_EFFECT_DAP_2_4_PROFILE_USER_SELECTABLE;
                    break;
                case AudioEffectManager.COMMON_SOUND_MODE_NIGHT:
                    ret = AudioEffectManager.SOUND_EFFECT_DAP_2_4_PROFILE_NIGHT;
                    break;
                case AudioEffectManager.COMMON_SOUND_MODE_NEWS:
                    ret = AudioEffectManager.SOUND_EFFECT_DAP_2_4_PROFILE_VOICE;
                    break;
                default: {
                    Log.w("DAP_2_4", "toDapProfileID() Not support SoundMode: " + mode);
                    ret = AudioEffectManager.SOUND_EFFECT_DAP_2_4_PROFILE_OFF;
                }
            }
            return ret;
        }
    }

    public void setBasicEffectMode(int mode) {
        if (audioEffectServiceIsNull()) return;
        try {
            mAudioEffectsService.setBasicEffectMode(mode);
        } catch (RemoteException e) {
            Log.e(TAG, "setBasicEffectMode failed:" + e);
        }
    }

    public int getBasicEffectMode() {
        if (audioEffectServiceIsNull()) return BASIC_EFFECT_MODE_OFF;
        try {
            return mAudioEffectsService.getBasicEffectMode();
        } catch (RemoteException e) {
            Log.e(TAG, "getBasicEffectMode failed:" + e);
        }
        return BASIC_EFFECT_MODE_OFF;
    }

    public int getDolbyMS12AudioConfig() {
        if (audioEffectServiceIsNull()) return DOLBY_MS12_AUDIO_CONFIG_Y;
        try {
            return mAudioEffectsService.getDolbyMS12AudioConfig();
        } catch (RemoteException e) {
            Log.e(TAG, "getDolbyMS12AudioConfig failed:" + e);
        }
        return DOLBY_MS12_AUDIO_CONFIG_Y;
    }

    public String getDolbyMS12AudioConfigStr() {
        int val = mInstance.getDolbyMS12AudioConfig();
        switch (val) {
        case DOLBY_MS12_AUDIO_CONFIG_N:
            return "N";
        case DOLBY_MS12_AUDIO_CONFIG_X:
            return "X";
        case DOLBY_MS12_AUDIO_CONFIG_Y:
            return "Y";
        case DOLBY_MS12_AUDIO_CONFIG_Z:
            return "Z";
        default:
            Log.w(TAG, "getDolbyMS12AudioConfigStr() Invalid val:" + val);
            return "Unknown";
        }
    }

    public void initDualEffectMode() {
        if (audioEffectServiceIsNull()) return;
        try {
            mAudioEffectsService.initDualEffectMode();
        } catch (RemoteException e) {
            Log.e(TAG, "setDualEffectMode failed:" + e);
        }
        return;
    }

    public void setDualEffectMode(int mode) {
        if (audioEffectServiceIsNull()) return;
        try {
            mAudioEffectsService.setDualEffectMode(mode);
        } catch (RemoteException e) {
            Log.e(TAG, "setDualEffectMode failed:" + e);
        }
        return;
    }

    public int getDualEffectMode() {
        if (audioEffectServiceIsNull()) return 0;
        try {
            return mAudioEffectsService.getDualEffectMode();
        } catch (RemoteException e) {
            Log.e(TAG, "getDualEffectMode failed:" + e);
        }
        return 0;
    }

    public int getEffectFunctionConfig(int id) {
        if (audioEffectServiceIsNull()) return 0;
        try {
            return mAudioEffectsService.getEffectFunctionConfig(id);
        } catch (RemoteException e) {
            Log.e(TAG, "getEffectFunctionConfig failed:" + e);
        }
        return 0;
    }

}
