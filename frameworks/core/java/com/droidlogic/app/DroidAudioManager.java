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

// import android.os.ServiceManager;
import com.droidlogic.app.SystemControlManager;

import vendor.amlogic.hardware.droidaudio.IDroidAudio;
import vendor.amlogic.hardware.droidaudio.IDroidAudioClient;

public class DroidAudioManager {
    private static final String TAG = "DroidAudioManager.J";
    private static IDroidAudio mDroidAudioService;

    private SystemControlManager mSystemControl;
    private AudioManager mAudioManager;

    private Context mContext;
    private final ContentResolver mResolver;

    public static final String PROP_AUDIO_OUTPUT_STRATEGY                           = "persist.vendor.media.audio.output.strategy";
    public static final String PROP_AUDIO_OUTPUT_SPDIF_COEXIST                      = "persist.vendor.media.audio.spdif.coexist";

    /* 0: Auto  1: Semi-Auto  2: Manual (refer to: audio_output_strategy enum in Engine.cpp) */
    public static final int OUTPUT_STRATEGY_AUTO                                    = 0;
    public static final int OUTPUT_STRATEGY_SEMI_AUTO                               = 1;
    public static final int OUTPUT_STRATEGY_MANUAL                                  = 2;
    private static DroidAudioManager mInstance;
    private static IDroidAudioClient mDroidAudioServiceClient = new DroidAudioServiceClient();

    public static DroidAudioManager getInstance(Context context) {
        if (mInstance == null) {
            synchronized (DroidAudioManager.class) {
                if (mInstance == null) {
                    mInstance = new DroidAudioManager(context);
                }
            }
        }
        return mInstance;
    }

    private DroidAudioManager(Context context) {
        Log.i(TAG, "construction DroidAudioManager");
        mContext = context;
        mResolver = context.getContentResolver();
        mSystemControl = SystemControlManager.getInstance();
        mAudioManager = (AudioManager)context.getSystemService(Context.AUDIO_SERVICE);
    }

    private static class DroidAudioServiceClient extends IDroidAudioClient.Stub {
        @Override
        public int onDroidAudioEvent(int event, int[] data) {
            // TODO:
            Log.i(TAG, "onDroidAudioEvent event:" + event);
            return 0;
        }
        @Override
        public String getInterfaceHash() {
            return IDroidAudioClient.HASH;
        }
        @Override
        public int getInterfaceVersion() {
            return IDroidAudioClient.VERSION;
        }
    }

    static IBinder.DeathRecipient mDeathRecipient = new IBinder.DeathRecipient() {
        @Override
        public void binderDied() {
            Log.w(TAG, "IDroidAudio service dead !!!");
            mDroidAudioService.asBinder().unlinkToDeath(this, 0);
            mDroidAudioService = null;
        }
    };

    static IDroidAudio getService() {
        synchronized (IDroidAudio.class) {
            if (mDroidAudioService != null) {
                return mDroidAudioService;
            }
            try {
                Object object = Class.forName("android.os.ServiceManager")
                                .getMethod("getService", new Class[] { String.class })
                                .invoke(null, new Object[] { IDroidAudio.DESCRIPTOR + "/default" });
                mDroidAudioService = IDroidAudio.Stub.asInterface((IBinder)object);
            } catch (Exception ex) {
                Log.e(TAG, "getService fail:" + ex);
                return null;
            }
            Log.i(TAG, "getService get IDroidAudio service success. ^_^");
            try {
                mDroidAudioService.asBinder().linkToDeath(mDeathRecipient, 0);
            } catch (RemoteException e) {
                Log.e(TAG, "getService linkToDeath fail:" + e);
            }
            try {
                mDroidAudioServiceClient = new DroidAudioServiceClient();
                mDroidAudioService.registerClient(mDroidAudioServiceClient);
            } catch (RemoteException e) {
                Log.e(TAG, "getService registerClient fail:" + e);
            }
        }
        return mDroidAudioService;
    }

    private boolean droidAudioServiceIsNull() {
        getService();
        if (mDroidAudioService == null) {
            Log.w(TAG, "mDroidAudioService is null, pls check stack:");
            Log.w(TAG, Log.getStackTraceString(new Throwable()));
            return true;
        } else {
            return false;
        }
    }

    public static String strategyToString(int strategy) {
        switch (strategy) {
        case DroidAudioManager.OUTPUT_STRATEGY_AUTO:
            return "Auto";
        case DroidAudioManager.OUTPUT_STRATEGY_SEMI_AUTO:
            return "Semi-Auto";
        case DroidAudioManager.OUTPUT_STRATEGY_MANUAL:
            return "Manual";
        default:
            Log.w(TAG, "strategyToString invalid strategy:" + strategy);
            return "None";
        }
    }

    public static final String DB_ID_AUDIO_DRC_MODE         = "db_id_audio_drc_mode";
    public static final String DTSDRC_MODE                  = "dtsdrc_mode";
    public static final String CUSTOM_0_DRCMODE             = "0";
    public static final String CUSTOM_1_DRCMODE             = "1";
    public static final String LINE_DRCMODE                 = "2";
    public static final String RF_DRCMODE                   = "3";
    public static final String DEFAULT_DRCMODE              = LINE_DRCMODE;

    public static final String AUDIO_DSP_AC3_DRC            = "/sys/class/audiodsp/ac3_drc_control";
    private void enableDolby_DRC(boolean enable) {
        if (enable) { //open DRC
            mSystemControl.writeSysFs(AUDIO_DSP_AC3_DRC, "drchighcutscale 0x64");
            mSystemControl.writeSysFs(AUDIO_DSP_AC3_DRC, "drclowboostscale 0x64");
        } else { //close DRC
            mSystemControl.writeSysFs(AUDIO_DSP_AC3_DRC, "drchighcutscale 0");
            mSystemControl.writeSysFs(AUDIO_DSP_AC3_DRC, "drclowboostscale 0");
        }
    }

    private void setDolbyMode(String mode) {
        //"CUSTOM_0","CUSTOM_1","LINE","RF"; default use "LINE"
        int i = Integer.parseInt(mode);
        if (i >= 0 && i <= 3) {
            mSystemControl.writeSysFs(AUDIO_DSP_AC3_DRC, "drcmode" + " " + mode);
        } else {
            mSystemControl.writeSysFs(AUDIO_DSP_AC3_DRC, "drcmode" + " " + DEFAULT_DRCMODE);
        }
    }

    public static final int IS_DRC_OFF                      = 0;
    public static final int IS_DRC_LINE                     = 1;
    public static final int IS_DRC_RF                       = 2;
    public void setDrcMode(int drcMode) {
        switch (drcMode) {
            case IS_DRC_OFF:
                enableDolby_DRC(false);
                setDolbyMode(LINE_DRCMODE);
                break;
            case IS_DRC_LINE:
                enableDolby_DRC(true);
                setDolbyMode(LINE_DRCMODE);
                break;
            case IS_DRC_RF:
                enableDolby_DRC(false);
                setDolbyMode(RF_DRCMODE);
                break;
            default:
                Log.e(TAG, "not support DRC mode:" + drcMode);
                return;
            }
        Settings.Global.putInt(mResolver, DB_ID_AUDIO_DRC_MODE, drcMode);
    }

    public int getDrcMode() {
        return Settings.Global.getInt(mResolver, DB_ID_AUDIO_DRC_MODE, IS_DRC_RF);
    }

    public static final String TV_ARC_LATENCY                           = "tv_arc_latency";
    public static final String PROPERTY_LOCAL_ARC_LATENCY               = "vendor.media.amnuplayer.audio.delayus";
    public static final int TV_ARC_LATENCY_MIN                          = -200;
    public static final int TV_ARC_LATENCY_MAX                          = 200;
    public static final int TV_ARC_LATENCY_DEFAULT                      = -40;
    public void setARCLatency(int newVal) {
        Settings.Global.putInt(mResolver, TV_ARC_LATENCY, newVal);
        if (newVal > TV_ARC_LATENCY_MAX) {
            newVal = TV_ARC_LATENCY_MAX;
        } else if (newVal < TV_ARC_LATENCY_MIN) {
            newVal = TV_ARC_LATENCY_MIN;
        }
        mSystemControl.setProperty(PROPERTY_LOCAL_ARC_LATENCY, "" + (newVal * 1000));
        Settings.Global.putInt(mResolver, TV_ARC_LATENCY, newVal);
    }

    public int getARCLatency() {
        return Settings.Global.getInt(mResolver, TV_ARC_LATENCY, TV_ARC_LATENCY_DEFAULT);
    }

    public static final String AUDIO_VAD_POWER_MEM_SLEEP_NODE               = "/sys/power/mem_sleep";
    public static final String AUDIO_VAD_POWER_STATE_NODE                   = "/sys/power/state";
    public static final String AUDIO_VAD_POWER_MEM_SLEEP_DEEP               = "deep";
    public static final String AUDIO_VAD_POWER_MEM_SLEEP_S2IDLE             = "s2idle";
    public static final String AUDIO_VAD_STRING_VAD_ON                      = "on";
    public static final String AUDIO_VAD_STRING_VAD_OFF                     = "off";
    public static final String AUDIO_VAD_UBOOTENV_FFV_WAKE                  = "ubootenv.var.ffv_wake";
    public static final String AUDIO_VAD_PROPERTY_VADWAKE                   = "persist.vendor.vadwake";
    public void setVadOn(boolean enable) {
        Log.i(TAG, "setVadOn:" + enable);
        String mode = AUDIO_VAD_STRING_VAD_OFF;
        if (enable) {
            mode = AUDIO_VAD_STRING_VAD_ON;
        }
        mSystemControl.setBootenv(AUDIO_VAD_UBOOTENV_FFV_WAKE, mode);
        mSystemControl.setProperty(AUDIO_VAD_PROPERTY_VADWAKE, mode);
    }

    public boolean isVadOn() {
        String vadUbootEnable = mSystemControl.getBootenv(AUDIO_VAD_UBOOTENV_FFV_WAKE, AUDIO_VAD_STRING_VAD_OFF);
        String property = mSystemControl.getPropertyString(AUDIO_VAD_PROPERTY_VADWAKE, AUDIO_VAD_STRING_VAD_OFF);
        Log.i(TAG, "isVadOn:" + vadUbootEnable);
        return vadUbootEnable.equals(AUDIO_VAD_STRING_VAD_ON);
    }

    private static final String PARA_AUDIO_DOLBY_MS12                   = "dolby_ms12_enable";
    private static final String PARA_AUDIO_DOLBY_MS12_ENABLE            = "dolby_ms12_enable=1";

    public static final String DIGITAL_AUDIO_FORMAT                     = "digital_audio_format";
    public static final String DIGITAL_AUDIO_SUBFORMAT                  = "digital_audio_subformat";
    public static final String PARAM_HAL_AUDIO_OUTPUT_FORMAT_PCM        = "hdmi_format=0";
    public static final String PARAM_HAL_AUDIO_OUTPUT_FORMAT_AUTO       = "hdmi_format=5";
    public static final String PARAM_HAL_AUDIO_OUTPUT_FORMAT_PASSTHROUGH= "hdmi_format=6";

    // DD/DD+/DTS
    public static final String DIGITAL_AUDIO_SUBFORMAT_SPDIF            = "5,6,7";

    private static final String NRDP_EXTERNAL_SURROUND                  = "nrdp_external_surround_sound_enabled";
    private static final int NRDP_ENABLE                                = 1;
    private static final int NRDP_DISABLE                               = 0;
    public void saveDigitalAudioFormatToHal(int mode, String submode) {
        boolean isTv = DroidLogicUtils.isTv();
        int nrdpStatus = NRDP_DISABLE;
        switch (mode) {
            case DIGITAL_AUDIO_FORMAT_MANUAL:
                if (isTv) {
                    mode = DIGITAL_AUDIO_FORMAT_AUTO;
                } else {
                    Settings.Global.putString(mResolver, DIGITAL_AUDIO_SUBFORMAT, submode);
                }
                mAudioManager.setParameters(PARAM_HAL_AUDIO_OUTPUT_FORMAT_AUTO);
                break;
            case DIGITAL_AUDIO_FORMAT_AUTO:
            case DIGITAL_AUDIO_FORMAT_PASSTHROUGH:
                if (isTv && isAudioSupportMs12System()) {
                    nrdpStatus = NRDP_ENABLE;
                }
                if (mode == DIGITAL_AUDIO_FORMAT_AUTO) {
                    mAudioManager.setParameters(PARAM_HAL_AUDIO_OUTPUT_FORMAT_AUTO);
                } else {
                    mAudioManager.setParameters(PARAM_HAL_AUDIO_OUTPUT_FORMAT_PASSTHROUGH);
                }
                break;
            case DIGITAL_AUDIO_FORMAT_PCM:
            default:
                mode = DIGITAL_AUDIO_FORMAT_PCM;
                mAudioManager.setParameters(PARAM_HAL_AUDIO_OUTPUT_FORMAT_PCM);
                break;
        }
        Settings.Global.putInt(mResolver, NRDP_EXTERNAL_SURROUND, nrdpStatus);
        Settings.Global.putInt(mResolver, DIGITAL_AUDIO_FORMAT, mode);
    }

    //surround sound formats, must sync with Settings.Global
    public static final String ENCODED_SURROUND_OUTPUT                  = "encoded_surround_output";
    public static final String ENCODED_SURROUND_OUTPUT_ENABLED_FORMATS  = "encoded_surround_output_enabled_formats";
    public static final int ENCODED_SURROUND_OUTPUT_AUTO                = 0;
    public static final int ENCODED_SURROUND_OUTPUT_NEVER               = 1;
    public static final int ENCODED_SURROUND_OUTPUT_ALWAYS              = 2;
    public static final int ENCODED_SURROUND_OUTPUT_MANUAL              = 3;
    public void saveDigitalAudioFormatToAndroid(int mode, String submode) {
        String tmp;
        // trigger AudioService retrieve support audio format value. Settings.Global.ENCODED_SURROUND_OUTPUT */
        Settings.Global.putInt(mResolver, ENCODED_SURROUND_OUTPUT, -1);
        int surround = -1;
        switch (mode) {
            case DIGITAL_AUDIO_FORMAT_MANUAL:
                if (DroidLogicUtils.isTv()) {
                    break;
                }
                /* Settings.Global.ENCODED_SURROUND_OUTPUT, Settings.Global.ENCODED_SURROUND_OUTPUT_MANUAL */
                Settings.Global.putInt(mResolver, ENCODED_SURROUND_OUTPUT, ENCODED_SURROUND_OUTPUT_MANUAL);
                tmp = Settings.Global.getString(mResolver, ENCODED_SURROUND_OUTPUT_ENABLED_FORMATS);
                if (!submode.equals(tmp)) {
                    Settings.Global.putString(mResolver, ENCODED_SURROUND_OUTPUT_ENABLED_FORMATS, submode);
                }
                break;
            case DIGITAL_AUDIO_FORMAT_PASSTHROUGH:
            case DIGITAL_AUDIO_FORMAT_AUTO:
                /* Settings.Global.ENCODED_SURROUND_OUTPUT, Settings.Global.ENCODED_SURROUND_OUTPUT_AUTO */
                Settings.Global.putInt(mResolver, ENCODED_SURROUND_OUTPUT, ENCODED_SURROUND_OUTPUT_AUTO);
                break;
            case DIGITAL_AUDIO_FORMAT_PCM:
            default:
                /* Settings.Global.ENCODED_SURROUND_OUTPUT, Settings.Global.ENCODED_SURROUND_OUTPUT_NEVER */
                Settings.Global.putInt(mResolver, ENCODED_SURROUND_OUTPUT, ENCODED_SURROUND_OUTPUT_NEVER);
                break;
        }
    }

    public static final int DIGITAL_AUDIO_FORMAT_PCM                    = 0;
    public static final int DIGITAL_AUDIO_FORMAT_AUTO                   = 1;
    public static final int DIGITAL_AUDIO_FORMAT_MANUAL                 = 2;
    public static final int DIGITAL_AUDIO_FORMAT_PASSTHROUGH            = 3;
    public void setDigitalAudioFormatOut(int mode) {
        setDigitalAudioFormatOut(mode, "");
    }
    public void setDigitalAudioFormatOut(int mode, String submode) {
        Log.d(TAG, "setDigitalAudioFormatOut mode:" + mode + ", submode:" + submode);
        if (DIGITAL_AUDIO_FORMAT_MANUAL == mode && submode == null) {
            submode = "";
            Log.i(TAG, "setDigitalAudioFormatOut manual mode, submode is null.");
        }
        saveDigitalAudioFormatToHal(mode, submode);
        saveDigitalAudioFormatToAndroid(mode, submode);
    }

    public int getDigitalAudioFormatOut() {
        return Settings.Global.getInt(mResolver, DIGITAL_AUDIO_FORMAT, DIGITAL_AUDIO_FORMAT_AUTO);
    }

    public void setAudioManualFormats(int id, boolean enabled) {
        HashSet<Integer> fmts = new HashSet<>();
        String enable = getAudioManualFormats();
        if (!enable.isEmpty()) {
            try {
                Arrays.stream(enable.split(",")).mapToInt(Integer::parseInt)
                    .forEach(fmts::add);
            } catch (NumberFormatException e) {
                Log.e(TAG, "setAudioManualFormats DIGITAL_AUDIO_SUBFORMAT misformatted.");
            }
        }
        if (enabled) {
            fmts.add(id);
        } else {
            fmts.remove(id);
        }
        setDigitalAudioFormatOut(DIGITAL_AUDIO_FORMAT_MANUAL, TextUtils.join(",", fmts));
    }

    public String getAudioManualFormats() {
        String format = Settings.Global.getString(mContext.getContentResolver(), DIGITAL_AUDIO_SUBFORMAT);
        return format == null ? "" : format;
    }

    public static String audioFormatOutputToString(int value) {
        String temp = "["+value+"]";
        switch (value) {
            case DIGITAL_AUDIO_FORMAT_PCM:
                return temp + "FORMAT_PCM";
            case DIGITAL_AUDIO_FORMAT_AUTO:
                return temp + "FORMAT_AUTO";
            case DIGITAL_AUDIO_FORMAT_MANUAL:
                return temp + "FORMAT_MANUAL";
            case DIGITAL_AUDIO_FORMAT_PASSTHROUGH:
                return temp + "FORMAT_PASSTHROUGH";
            default:
                return temp + "INVALID_VALUE";
        }
    }

    public boolean isAudioSupportMs12System() {
        return mAudioManager.getParameters(PARA_AUDIO_DOLBY_MS12).contains(PARA_AUDIO_DOLBY_MS12_ENABLE);
    }

    public static final int DIALOGUE_ENHANCEMENT_OFF       = 0;
    public static final int DIALOGUE_ENHANCEMENT_LOW       = 4;
    public static final int DIALOGUE_ENHANCEMENT_MEDIUM    = 8;
    public static final int DIALOGUE_ENHANCEMENT_HIGH      = 12;
    public void setAc4DialogEnhancer(int newVal) {
        Log.d(TAG, "setAc4DialogEnhancer: " + newVal);
        switch (newVal) {
            case DIALOGUE_ENHANCEMENT_OFF:
                Settings.Global.putInt(mResolver, DIALOGUE_ENHANCEMENT_SWITCH, 0);
                mAudioManager.setParameters("dialogue_enhancement=0");
                break;
            case DIALOGUE_ENHANCEMENT_LOW:
                Settings.Global.putInt(mResolver, DIALOGUE_ENHANCEMENT_SWITCH, 4);
                mAudioManager.setParameters("dialogue_enhancement=4");
                break;
            case DIALOGUE_ENHANCEMENT_MEDIUM:
                Settings.Global.putInt(mResolver, DIALOGUE_ENHANCEMENT_SWITCH, 8);
                mAudioManager.setParameters("dialogue_enhancement=8");
                break;
            case DIALOGUE_ENHANCEMENT_HIGH:
                Settings.Global.putInt(mResolver, DIALOGUE_ENHANCEMENT_SWITCH, 12);
                mAudioManager.setParameters("dialogue_enhancement=12");
                break;
        }
    }

    public static final String FORCE_DDP_SWITCH      = "force_ddp_enable";
    public static final int FORCE_DDP_OFF   = 0;
    public static final int FORCE_DDP_ON    = 1;
    public void setForceDDPEnable(boolean newVal) {
        Log.d(TAG, "setForceDDPEnable: " + newVal);
        if (newVal) {
           Settings.Global.putInt(mResolver, FORCE_DDP_SWITCH, FORCE_DDP_ON);
           mAudioManager.setParameters("hal_param_force_ddp=1");
        } else {
           Settings.Global.putInt(mResolver, FORCE_DDP_SWITCH, FORCE_DDP_OFF);
           mAudioManager.setParameters("hal_param_force_ddp=0");
        }
    }

    //ac4 enhancer
    public static final String AC4_DIALOGUE_ENHANCEMENT_VALUE   = "ac4_dialogue_enhancer_value";
    public static final String DIALOGUE_ENHANCEMENT_SWITCH      = "dialogue_enhancement";
    public int getAc4DialogEnhancer() {
        return Settings.Global.getInt(mResolver, DIALOGUE_ENHANCEMENT_SWITCH, DIALOGUE_ENHANCEMENT_OFF);
    }

    public boolean getForceDDPEnable() {
        return Settings.Global.getInt(mResolver, FORCE_DDP_SWITCH,
                FORCE_DDP_OFF) == FORCE_DDP_ON;
    }

    public static final String DB_ID_SOUND_SPDIF_OUTPUT_ENABLE          = "db_id_sound_spdif_output_enable";
    public static final String HAL_PARAM_SPDIF_OUTPUT_ENABLE            = "hal_param_spdif_output_enable=";
    public void setSoundSpdifEnable(boolean enable) {
        Settings.Global.putInt(mResolver, DB_ID_SOUND_SPDIF_OUTPUT_ENABLE, enable ? 1 : 0);
        mAudioManager.setParameters(HAL_PARAM_SPDIF_OUTPUT_ENABLE + (enable ? 1 : 0));
    }

    public boolean getSoundSpdifEnable() {
        return Settings.Global.getInt(mResolver, DB_ID_SOUND_SPDIF_OUTPUT_ENABLE, 1) != 0;
    }

    public static final String DB_ID_SOUND_AD_SWITCH                    = "ad_switch";
    public static final String HAL_PARAM_AD_SWITCH                      = "ad_switch_enable=";
    public void setAdSupportEnable(boolean newVal) {
        DataProviderManager.putIntValue(mContext, DB_ID_SOUND_AD_SWITCH, newVal ? 1 : 0);
        mAudioManager.setParameters(HAL_PARAM_AD_SWITCH + (newVal ? 1 : 0));
    }

    public boolean getAdSupportEnable() {
        DataProviderManager.getIntValue(mContext, DB_ID_SOUND_AD_SWITCH, 0) ;
        return DataProviderManager.getIntValue(mContext, DB_ID_SOUND_AD_SWITCH, 0) != 0;
    }

    /* [setAudioOutputSpeakerDelay / setAudioOutputSpdifDelay / setAudioOutputHeadphoneDelay/
     * setAudioPrescale] output delay source define
     */
    public static final int AUDIO_OUTPUT_DELAY_SOURCE_ATV               = 0;
    public static final int AUDIO_OUTPUT_DELAY_SOURCE_DTV               = 1;
    public static final int AUDIO_OUTPUT_DELAY_SOURCE_AV                = 2;
    public static final int AUDIO_OUTPUT_DELAY_SOURCE_HDMI              = 3;
    public static final int AUDIO_OUTPUT_DELAY_SOURCE_MEDIA             = 4;
    public static final int AUDIO_OUTPUT_DELAY_SOURCE_MAX               = 5;

    public static final int HAL_AUDIO_OUT_DEV_DELAY_MIN                 = 0;       // ms
    public static final int HAL_AUDIO_OUT_DEV_DELAY_MAX                 = 200;     // ms
    public static final int HAL_AUDIO_OUT_DEV_DELAY_DEFAULT             = 0;       // ms

    // refer to audio hal aml_audio_delay_type_e enum
    private static final int HAL_AUDIO_OUT_DEV_DELAY_SPEAKER            = 0;
    private static final int HAL_AUDIO_OUT_DEV_DELAY_SPDIF              = 1;
    private static final int HAL_AUDIO_OUT_DEV_DELAY_HEADPHONE          = 2;
    private static final int HAL_AUDIO_OUT_DEV_DELAY_ALL                = 3;
    private static final String[] DB_ID_AUDIO_OUTPUT_SPEAKER_DELAY_ARRAY    = {
            "db_id_audio_output_speaker_delay_atv",
            "db_id_audio_output_speaker_delay_dtv",
            "db_id_audio_output_speaker_delay_av",
            "db_id_audio_output_speaker_delay_hdmi",
            "db_id_audio_output_speaker_delay_media",
    };
    private static final String[] DB_ID_AUDIO_OUTPUT_SPDIF_DELAY_ARRAY       = {
            "db_id_audio_output_spdif_delay_atv",
            "db_id_audio_output_spdif_delay_dtv",
            "db_id_audio_output_spdif_delay_av",
            "db_id_audio_output_spdif_delay_hdmi",
            "db_id_audio_output_spdif_delay_media",
    };
    private static final String[] DB_ID_AUDIO_OUTPUT_HEADPHONE_DELAY_ARRAY   = {
            "db_id_audio_output_headphone_delay_atv",
            "db_id_audio_output_headphone_delay_dtv",
            "db_id_audio_output_headphone_delay_av",
            "db_id_audio_output_headphone_delay_hdmi",
            "db_id_audio_output_headphone_delay_media",
    };
    private static final String[] DB_ID_AUDIO_PRESCALE_ARRAY       = {
            "db_id_audio_prescale_atv",
            "db_id_audio_prescale_dtv",
            "db_id_audio_prescale_av",
            "db_id_audio_prescale_hdmi",
            "db_id_audio_prescale_media",
    };

    private static final String PARAM_HAL_AUDIO_OUT_DEV_DELAY           = "hal_param_out_dev_delay_time_ms";
    private void setAudioOutputDelayToHal(int output, int delayMs) {
        /* High 16 - bit expression type, low 16 - bit expression delay time. refer to audio hal */
        mAudioManager.setParameters(PARAM_HAL_AUDIO_OUT_DEV_DELAY + "=" + (output << 16 | delayMs));
    }

    public static final String PROP_AUDIO_DELAY_ENABLED                 = "persist.vendor.tv.audio.delay.enabled";
    private boolean getAudioDelayEnabled() {
        return SystemControlManager.getInstance()
                .getPropertyBoolean(PROP_AUDIO_DELAY_ENABLED, false);
    }

    public static final String DB_ID_TV_SOURCE_TYPE = "db_id_tv_source_type";
    private void setTvSourceType(int source) {
        Settings.Global.putInt(mResolver, DB_ID_TV_SOURCE_TYPE, source);
    }

    private int getTvSourceType() {
        return  Settings.Global.getInt(mResolver, DB_ID_TV_SOURCE_TYPE, AUDIO_OUTPUT_DELAY_SOURCE_MEDIA);
    }

    private boolean checkSourceValid(int source, String caller) {
        if (source < AUDIO_OUTPUT_DELAY_SOURCE_ATV || source >= AUDIO_OUTPUT_DELAY_SOURCE_MAX) {
            Log.w(TAG, caller + ": unsupport tv source:" + source + ", min:" + AUDIO_OUTPUT_DELAY_SOURCE_ATV
                    + ", max:" + (AUDIO_OUTPUT_DELAY_SOURCE_MAX - 1));
            return false;
        }
        return true;
    }

    public void setAudioOutputSpeakerDelay(int source, int delayMs) {
        if (!checkSourceValid(source, "setAudioOutputSpeakerDelay")) {
            return;
        }
        if (delayMs < HAL_AUDIO_OUT_DEV_DELAY_MIN || delayMs > HAL_AUDIO_OUT_DEV_DELAY_MAX) {
            Log.w(TAG, "unsupport speaker delay time:" + delayMs + "ms, min:" + HAL_AUDIO_OUT_DEV_DELAY_MIN + "ms, max:"
                    + HAL_AUDIO_OUT_DEV_DELAY_MAX + "ms, now use max value");
            delayMs = HAL_AUDIO_OUT_DEV_DELAY_MAX;
        }
        if (DroidLogicUtils.getAudioDebugEnable()) Log.d(TAG, "setAudioOutputSpeakerDelay source:" + tvSourceToString(source) + ", delayMs:" + delayMs);
        int currentTvSource = getTvSourceType();
        if (currentTvSource == source) {
            setAudioOutputDelayToHal(HAL_AUDIO_OUT_DEV_DELAY_SPEAKER, delayMs);
        } else {
            Log.i(TAG, "setAudioOutputSpeakerDelay cur source:" + tvSourceToString(currentTvSource) +
                    " is not the same as the set source:" + tvSourceToString(source) + ", only save to DB");
        }
        Settings.Global.putInt(mResolver, DB_ID_AUDIO_OUTPUT_SPEAKER_DELAY_ARRAY[source], delayMs);
    }

    public int getAudioOutputSpeakerDelay(int source) {
        if (!checkSourceValid(source, "getAudioOutputSpeakerDelay")) {
            return 0;
        }
        int delayMs = Settings.Global.getInt(mResolver, DB_ID_AUDIO_OUTPUT_SPEAKER_DELAY_ARRAY[source], HAL_AUDIO_OUT_DEV_DELAY_DEFAULT);
        if (DroidLogicUtils.getAudioDebugEnable()) Log.d(TAG, "getAudioOutputSpeakerDelay source:" + tvSourceToString(source) + ", delayMs:" + delayMs);
        return delayMs;
    }

    public void setAudioOutputSpdifDelay(int source, int delayMs) {
        if (!checkSourceValid(source, "setAudioOutputSpdifDelay")) {
            return;
        }
        if (delayMs < HAL_AUDIO_OUT_DEV_DELAY_MIN || delayMs > HAL_AUDIO_OUT_DEV_DELAY_MAX) {
            Log.w(TAG, "unsupport spdif delay time:" + delayMs + "ms, min:" + HAL_AUDIO_OUT_DEV_DELAY_MIN + "ms, max:"
                    + HAL_AUDIO_OUT_DEV_DELAY_MAX + "ms, now use max value");
            delayMs = HAL_AUDIO_OUT_DEV_DELAY_MAX;
        }
        if (DroidLogicUtils.getAudioDebugEnable()) Log.d(TAG, "setAudioOutputSpdifDelay source:" + tvSourceToString(source) + ", delayMs:" + delayMs);
        int currentTvSource = getTvSourceType();
        if (currentTvSource == source) {
            setAudioOutputDelayToHal(HAL_AUDIO_OUT_DEV_DELAY_SPDIF, delayMs);
        } else {
            Log.i(TAG, "setAudioOutputSpdifDelay cur source:" + tvSourceToString(currentTvSource) +
                    " is not the same as the set source:" + tvSourceToString(source) + ", only save to DB");
        }
        Settings.Global.putInt(mResolver, DB_ID_AUDIO_OUTPUT_SPDIF_DELAY_ARRAY[source], delayMs);
    }

    public int getAudioOutputSpdifDelay(int source) {
        if (!checkSourceValid(source, "getAudioOutputSpdifDelay")) {
            return 0;
        }
        int delayMs = Settings.Global.getInt(mResolver, DB_ID_AUDIO_OUTPUT_SPDIF_DELAY_ARRAY[source], HAL_AUDIO_OUT_DEV_DELAY_DEFAULT);
        if (DroidLogicUtils.getAudioDebugEnable()) Log.d(TAG, "getAudioOutputSpdifDelay source:" + tvSourceToString(source) + ", delayMs:" + delayMs);
        return delayMs;
    }

    public void setAudioOutputHeadphoneDelay(int source, int delayMs) {
        if (!checkSourceValid(source, "setAudioOutputHeadphoneDelay")) {
            return;
        }
        if (delayMs < HAL_AUDIO_OUT_DEV_DELAY_MIN || delayMs > HAL_AUDIO_OUT_DEV_DELAY_MAX) {
            Log.w(TAG, "unsupport spdif delay time:" + delayMs + "ms, min:" + HAL_AUDIO_OUT_DEV_DELAY_MIN + "ms, max:"
                    + HAL_AUDIO_OUT_DEV_DELAY_MAX + "ms, now use max value");
            delayMs = HAL_AUDIO_OUT_DEV_DELAY_MAX;
        }
        if (DroidLogicUtils.getAudioDebugEnable()) Log.d(TAG, "setAudioOutputHeadphoneDelay source:" + tvSourceToString(source) + ", delayMs:" + delayMs);
        int currentTvSource = getTvSourceType();
        if (currentTvSource == source) {
            setAudioOutputDelayToHal(HAL_AUDIO_OUT_DEV_DELAY_HEADPHONE, delayMs);
        } else {
            Log.i(TAG, "setAudioOutputHeadphoneDelay cur source:" + tvSourceToString(currentTvSource) +
                    " is not the same as the set source:" + tvSourceToString(source) + ", only save to DB");
        }
        Settings.Global.putInt(mResolver, DB_ID_AUDIO_OUTPUT_HEADPHONE_DELAY_ARRAY[source], delayMs);
    }

    public int getAudioOutputHeadphoneDelay(int source) {
        if (!checkSourceValid(source, "getAudioOutputHeadphoneDelay")) {
            return 0;
        }
        int delayMs = Settings.Global.getInt(mResolver, DB_ID_AUDIO_OUTPUT_HEADPHONE_DELAY_ARRAY[source], HAL_AUDIO_OUT_DEV_DELAY_DEFAULT);
        if (DroidLogicUtils.getAudioDebugEnable()) Log.d(TAG, "getAudioOutputHeadphoneDelay source:" + tvSourceToString(source) + ", delayMs:" + delayMs);
        return delayMs;
    }

    public static final String DB_ID_AUDIO_OUTPUT_ALL_DELAY                     = "db_id_audio_output_all_delay";
    public void setAudioOutputAllDelay(int delayMs) {
        if (delayMs < HAL_AUDIO_OUT_DEV_DELAY_MIN || delayMs > HAL_AUDIO_OUT_DEV_DELAY_MAX) {
            Log.w(TAG, "unsupport delay time:" + delayMs + "ms, min:" + HAL_AUDIO_OUT_DEV_DELAY_MIN + "ms, max:"
                    + HAL_AUDIO_OUT_DEV_DELAY_MAX + "ms, now use max value");
            delayMs = HAL_AUDIO_OUT_DEV_DELAY_MAX;
        }
        if (DroidLogicUtils.getAudioDebugEnable()) Log.d(TAG, "setAudioOutputAllDelay delay " + ", delayMs:" + delayMs);
        setAudioOutputDelayToHal(HAL_AUDIO_OUT_DEV_DELAY_ALL, delayMs);
        Settings.Global.putInt(mResolver, DB_ID_AUDIO_OUTPUT_ALL_DELAY, delayMs);
    }

    public int getAudioOutputAllDelay() {
        int delayMs = Settings.Global.getInt(mResolver, DB_ID_AUDIO_OUTPUT_ALL_DELAY, HAL_AUDIO_OUT_DEV_DELAY_DEFAULT);
        if (DroidLogicUtils.getAudioDebugEnable()) Log.d(TAG, "getAudioOutputAllDelay, delayMs:" + delayMs);
        return delayMs;
    }

    private static final int AUDIO_PRESCALE_MIN                         = -150;    // -15 dB
    private static final int AUDIO_PRESCALE_MAX                         = 150;     // 15 dB
    private static final int[] AUDIO_PRESCALE_DEFAULT_ARRAY             = {0, 0, 0, 0, 0}; // ATV, DTV, AV, HDMI, MEDIA, range: [-150 - 150]
    private static final String PARAM_HAL_AUDIO_PRESCALE                = "SOURCE_GAIN";
    public void setAudioPrescale(int source,int value) {
        if (!checkSourceValid(source, "setAudioPrescale")) {
            return;
        }
        if (value < AUDIO_PRESCALE_MIN || value > AUDIO_PRESCALE_MAX) {
            Log.w(TAG, "unsupport audio prescale:" + value + ", min:" + AUDIO_PRESCALE_MIN + ", max:" + AUDIO_PRESCALE_MAX);
            return;
        }
        if (DroidLogicUtils.getAudioDebugEnable()) Log.d(TAG, "setAudioPrescale source:" + tvSourceToString(source) + ", value:" + value);
        try {
            StringBuffer parameter;
            String realValue = "";
            DecimalFormatSymbols symbols = new DecimalFormatSymbols(Locale.getDefault());
            symbols.setDecimalSeparator('.');
            DecimalFormat decimalFormat = new DecimalFormat("0.0", symbols);
            Settings.Global.putInt(mResolver, DB_ID_AUDIO_PRESCALE_ARRAY[source], value);

            // packaging "SOURCE_GAIN=1.0 1.0 1.0 1.0 1.0" [atv,dtv,hdmi,av,media]
            parameter = new StringBuffer(PARAM_HAL_AUDIO_PRESCALE + "=");
            //UI -150 - 150, audio_hal -15 - 15 db
            int tempParameter = 1;
            tempParameter = Settings.Global.getInt(mResolver, DB_ID_AUDIO_PRESCALE_ARRAY[AUDIO_OUTPUT_DELAY_SOURCE_ATV],
                    AUDIO_PRESCALE_DEFAULT_ARRAY[AUDIO_OUTPUT_DELAY_SOURCE_ATV]);
            realValue = decimalFormat.format((float) tempParameter / 10);
            parameter.append(realValue + " ");

            tempParameter = Settings.Global.getInt(mResolver, DB_ID_AUDIO_PRESCALE_ARRAY[AUDIO_OUTPUT_DELAY_SOURCE_DTV],
                    AUDIO_PRESCALE_DEFAULT_ARRAY[AUDIO_OUTPUT_DELAY_SOURCE_DTV]);
            realValue = decimalFormat.format((float) tempParameter / 10);
            parameter.append(realValue + " ");

            tempParameter = Settings.Global.getInt(mResolver, DB_ID_AUDIO_PRESCALE_ARRAY[AUDIO_OUTPUT_DELAY_SOURCE_HDMI],
                    AUDIO_PRESCALE_DEFAULT_ARRAY[AUDIO_OUTPUT_DELAY_SOURCE_HDMI]);
            realValue = decimalFormat.format((float) tempParameter / 10);
            parameter.append(realValue + " ");

            tempParameter = Settings.Global.getInt(mResolver, DB_ID_AUDIO_PRESCALE_ARRAY[AUDIO_OUTPUT_DELAY_SOURCE_AV],
                    AUDIO_PRESCALE_DEFAULT_ARRAY[AUDIO_OUTPUT_DELAY_SOURCE_AV]);
            realValue = decimalFormat.format((float) tempParameter / 10);
            parameter.append(realValue + " ");

            tempParameter = Settings.Global.getInt(mResolver, DB_ID_AUDIO_PRESCALE_ARRAY[AUDIO_OUTPUT_DELAY_SOURCE_MEDIA],
                    AUDIO_PRESCALE_DEFAULT_ARRAY[AUDIO_OUTPUT_DELAY_SOURCE_MEDIA]);
            realValue = decimalFormat.format((float) tempParameter / 10);
            parameter.append(realValue + " ");
            if (DroidLogicUtils.getAudioDebugEnable()) Log.d(TAG, "setAudioPrescale setParameters:" + parameter.toString());
            mAudioManager.setParameters(parameter.toString());
        } catch (Exception e) {
            e.printStackTrace();
        }
    }

    public int getAudioPrescale(int source) {
        if (!checkSourceValid(source, "getAudioPrescale")) {
            return 0;
        }
        int saveResult = 0;
        BigDecimal mBigDecimalBase = new BigDecimal(Float.toString(10.00f));
        BigDecimal mBigDecimal = new BigDecimal(0.0f);
        String value = mAudioManager.getParameters("SOURCE_GAIN");//atv,dtv,hdmi,av,media
        value.trim();
        if (DroidLogicUtils.getAudioDebugEnable()) Log.d(TAG, "getAudioPrescale hal param:" + value);
        saveResult = Settings.Global.getInt(mResolver, DB_ID_AUDIO_PRESCALE_ARRAY[source],
                AUDIO_PRESCALE_DEFAULT_ARRAY[AUDIO_OUTPUT_DELAY_SOURCE_ATV]);
        String[] subStrings = value.split(" ");//"source_gain = 1.0 1.0 1.0 1.0 1.0"
        if (subStrings.length == 7) {
            int driverValue = 0;
            mBigDecimal = new BigDecimal(subStrings[subStrings.length - 5].substring(0,3));
            driverValue = (int) mBigDecimal.multiply(mBigDecimalBase).floatValue();
            if (driverValue != saveResult) {
                Log.w(TAG, "getAudioPrescaleStatus driverValue:" + driverValue + ", saveResult:" + saveResult);
            }
        } else {
            Log.w(TAG, "getAudioPrescaleStatus param length:" + subStrings.length + " invalid");
        }
        if (DroidLogicUtils.getAudioDebugEnable()) Log.d(TAG, "getAudioPrescale source:" + tvSourceToString(source) + ", value:" + saveResult);
        return saveResult;
    }

    public static String tvSourceToString(int source) {
        String temp = "[" + source + "]";
        switch (source) {
            case AUDIO_OUTPUT_DELAY_SOURCE_ATV:
                return temp + "SRC_ATV";
            case AUDIO_OUTPUT_DELAY_SOURCE_DTV:
                return temp + "SRC_DTV";
            case AUDIO_OUTPUT_DELAY_SOURCE_AV:
                return temp + "SRC_AV";
            case AUDIO_OUTPUT_DELAY_SOURCE_HDMI:
                return temp + "SRC_HDMI";
            case AUDIO_OUTPUT_DELAY_SOURCE_MEDIA:
                return temp + "SRC_MEDIA";
            default:
                return temp + "invalid source:" + source;
        }
    }
    public void refreshAudioCfgBySrc(int source, boolean force) {
        if (!checkSourceValid(source, "refreshAudioCfgBySrc")) {
            return;
        }
        boolean isEnableDelay = getAudioDelayEnabled();
        setTvSourceType(source);
        Log.i(TAG, "refreshAudioCfgBySrc src:" + tvSourceToString(source) + ", delayEnable:" + isEnableDelay);
        if (isEnableDelay || force) {
            setAudioOutputSpeakerDelay(source, getAudioOutputSpeakerDelay(source));
            setAudioOutputSpdifDelay(source, getAudioOutputSpdifDelay(source));
            setAudioOutputHeadphoneDelay(source, getAudioOutputSpdifDelay(source));
        }
    }

    public static final String SYS_HDMITX_AUDIO_SOUNDBAR_EN                 = "/sys/class/amhdmitx/amhdmitx0/soundbar_en";
    private static final String DB_ID_AUDIO_SOUNDBAR_MODE_ENABLE            = "soundbar_mode";
    public void setSoundBarModeEnabled(boolean enable) {
        Log.i(TAG, "setSoundBarModeEnabled soundbar:" + enable);
        Settings.Global.putInt(mResolver, DB_ID_AUDIO_SOUNDBAR_MODE_ENABLE, enable ? 1 : 0);
        mSystemControl.setProperty("persist.vendor.media.audio.soundbar.mode", enable ? "1" : "0");
        mAudioManager.setParameters("hal_param_soundbar_mode=" + (enable ? "1" : "0"));
        mSystemControl.writeSysFs(SYS_HDMITX_AUDIO_SOUNDBAR_EN, enable ? "1" : "0");
    }

    public boolean isSoundBarModeEnabled() {
        boolean flag = mSystemControl.getPropertyBoolean("ro.vendor.platform.support.soundbar", false);
        int soundBarEnable = Settings.Global.getInt(mResolver, DB_ID_AUDIO_SOUNDBAR_MODE_ENABLE, flag ? 1 : 0);
        return (soundBarEnable == 1);
    }

    public void init() {

    }

    public static String AudioCmdToString(int cmd) {
        String temp = "[" + cmd + "]";
        switch (cmd) {
            case AUDIO_SERVICE_CMD_START_DECODE:
                return temp + "START_DECODE";
            case AUDIO_SERVICE_CMD_PAUSE_DECODE:
                return temp + "PAUSE_DECODE";
            case AUDIO_SERVICE_CMD_RESUME_DECODE:
                return temp + "RESUME_DECODE";
            case AUDIO_SERVICE_CMD_STOP_DECODE:
                return temp + "STOP_DECODE";
            case AUDIO_SERVICE_CMD_SET_DECODE_AD:
                return temp + "SET_DECODE_AD";
            case AUDIO_SERVICE_CMD_SET_VOLUME:
                return temp + "SET_VOLUME";
            case AUDIO_SERVICE_CMD_SET_MUTE:
                return temp + "SET_MUTE";
            case AUDIO_SERVICE_CMD_SET_OUTPUT_MODE:
                return temp + "SET_OUTPUT_MODE";
            case AUDIO_SERVICE_CMD_SET_PRE_GAIN:
                return temp + "SET_PRE_GAIN";
            case AUDIO_SERVICE_CMD_SET_PRE_MUTE:
                return temp + "SET_PRE_MUTE";
            case AUDIO_SERVICE_CMD_OPEN_DECODER:
                return temp + "OPEN_DECODER";
            case AUDIO_SERVICE_CMD_CLOSE_DECODER:
                return temp + "CLOSE_DECODER";
            case AUDIO_SERVICE_CMD_SET_DEMUX_INFO:
                return temp + "SET_DEMUX_INFO";
            case AUDIO_SERVICE_CMD_SET_SECURITY_MEM_LEVEL:
                return temp + "SET_SECURITY_MEM_LEVEL";

            case AUDIO_SERVICE_CMD_AD_SWITCH_ENABLE:
                return temp + "AD_SWITCH_ENABLE";
            case AUDIO_SERVICE_CMD_AD_SET_VOLUME:
                return temp + "AD_SET_VOLUME";
            case AUDIO_SERVICE_CMD_AD_DUAL_SUPPORT:
                return temp + "AD_DUAL_SUPPORT";
            case AUDIO_SERVICE_CMD_AD_MIX_SUPPORT:
                return temp + "AD_MIX_SUPPORT";
            case AUDIO_SERVICE_CMD_AD_MIX_LEVEL:
                return temp + "AD_MIX_LEVEL";
            case AUDIO_SERVICE_CMD_AD_SET_MAIN:
                return temp + "AD_SET_MAIN";
            case AUDIO_SERVICE_CMD_AD_SET_ASSOCIATE:
                return temp + "AD_SET_ASSOCIATE";
            case AUDIO_SERVICE_CMD_SET_HAS_VIDEO:
                return temp + "SET_HAS_VIDEO";
            case AUDIO_SERVICE_CMD_SET_MEDIA_PRESENTATION_ID:
                return temp + "SET_MEDIA_PRESENTATION_ID";
            case AUDIO_SERVICE_CMD_SET_AUDIO_PATCH_MANAGE_MODE:
                return temp + "SET_AUDIO_PATCH_MANAGE_MODE";
            case AUDIO_SERVICE_CMD_SET_SPDIF_PROTECTION_MODE:
                return temp + "SET_SPDIF_PROTECTION_MODE";
            case AUDIO_SERVICE_CMD_SET_TSPLAYER_CLIENT_DIED:
                return temp + "SET_TSPLAYER_CLIENT_DIED";
            case AUDIO_SERVICE_CMD_SET_MEDIA_FIRST_LANG:
                return temp + "SET_MEDIA_FIRST_LANG";
            case AUDIO_SERVICE_CMD_SET_MEDIA_SECOND_LANG:
                return temp + "SET_MEDIA_SECOND_LANG";
            default:
                return temp + "invalid cmd";
        }
    }
    public static final int AUDIO_SERVICE_CMD_START_DECODE                          = 1;
    public static final int AUDIO_SERVICE_CMD_PAUSE_DECODE                          = 2;
    public static final int AUDIO_SERVICE_CMD_RESUME_DECODE                         = 3;
    public static final int AUDIO_SERVICE_CMD_STOP_DECODE                           = 4;
    public static final int AUDIO_SERVICE_CMD_SET_DECODE_AD                         = 5;
    public static final int AUDIO_SERVICE_CMD_SET_VOLUME                            = 6;
    public static final int AUDIO_SERVICE_CMD_SET_MUTE                              = 7;
    public static final int AUDIO_SERVICE_CMD_SET_OUTPUT_MODE                       = 8;
    public static final int AUDIO_SERVICE_CMD_SET_PRE_GAIN                          = 9;
    public static final int AUDIO_SERVICE_CMD_SET_PRE_MUTE                          = 10;
    public static final int AUDIO_SERVICE_CMD_OPEN_DECODER                          = 12;
    public static final int AUDIO_SERVICE_CMD_CLOSE_DECODER                         = 13;
    public static final int AUDIO_SERVICE_CMD_SET_DEMUX_INFO                        = 14;
    public static final int AUDIO_SERVICE_CMD_SET_SECURITY_MEM_LEVEL                = 15;
    public static final int AUDIO_SERVICE_CMD_SET_HAS_VIDEO                         = 16;
    public static final int AUDIO_SERVICE_CMD_SET_MEDIA_SYCN_ID                     = 17;

    //audio ad
    public static final int AUDIO_SERVICE_CMD_AD_SWITCH_ENABLE                      = 18;
    public static final int AUDIO_SERVICE_CMD_AD_SET_VOLUME                         = 19;
    public static final int AUDIO_SERVICE_CMD_AD_DUAL_SUPPORT                       = 20;
    public static final int AUDIO_SERVICE_CMD_AD_MIX_SUPPORT                        = 21;
    public static final int AUDIO_SERVICE_CMD_AD_MIX_LEVEL                          = 22;
    public static final int AUDIO_SERVICE_CMD_AD_SET_MAIN                           = 23;
    public static final int AUDIO_SERVICE_CMD_AD_SET_ASSOCIATE                      = 24;

    public static final int AUDIO_SERVICE_CMD_SET_MEDIA_PRESENTATION_ID             = 25;
    public static final int AUDIO_SERVICE_CMD_SET_AUDIO_PATCH_MANAGE_MODE           = 26;
    public static final int AUDIO_SERVICE_CMD_SET_SPDIF_PROTECTION_MODE             = 27;
    public static final int AUDIO_SERVICE_CMD_SET_TSPLAYER_CLIENT_DIED              = 28;

    public static final int AUDIO_SERVICE_CMD_SET_MEDIA_FIRST_LANG                  = 29;
    public static final int AUDIO_SERVICE_CMD_SET_MEDIA_SECOND_LANG                 = 30;
    public static final int AUDIO_SERVICE_CMD_SET_AUDIO_PICTURE_MODE                = 31;
    public void handleAdtvAudioEvent(int cmd, int param1, int param2) {
        if (droidAudioServiceIsNull()) return;
        try {
            mDroidAudioService.setAudioCmdParam(cmd, param1, param2, 0);
        } catch (RemoteException e) {
            Log.e(TAG, "setAudioCmdParam failed:" + e);
        }
    }

    public void updateAudioPortGain(int sourceType) {
        Log.d(TAG, "updateAudioPortGain sourceType:" + sourceType);
    }

    public void openTvAudio(int sourceType) {
        Log.d(TAG, "openTvAudio sourceType:" + sourceType);
    }

    public void closeTvAudio() {
        Log.d(TAG, "closeTvAudio ");
    }

    public static final int DROID_AUDIO_FORCE_USE_NONE                              = 0;
    public static final int DROID_AUDIO_FORCE_USE_SPEAKER                           = 1;
    public static final int DROID_AUDIO_FORCE_USE_SPDIF                             = 17;
    public static final int DROID_AUDIO_FORCE_USE_HDMI_ARC                          = 16;
    public static final int DROID_AUDIO_FORCE_USE_HDMI_OUT                          = 18;
    public static final int DROID_AUDIO_FORCE_USE_HEADPHONES                        = 2;
    public static final int DROID_AUDIO_FORCE_USE_WIRED_ACCESSORY                   = 5;
    public static final int DROID_AUDIO_FORCE_USE_BT_A2DP                           = 4;
    public int setOutputDevices(int[] devices) {
        if (droidAudioServiceIsNull()) return 0;
        try {
            return mDroidAudioService.setOutputDevices(devices);
        } catch (RemoteException e) {
            Log.e(TAG, "setOutputDevices failed:" + e);
            return 0;
        }
    }

    public int[] getOutputDevices() {
        if (droidAudioServiceIsNull()) return null;
        try {
            return mDroidAudioService.getOutputDevices();
        } catch (RemoteException e) {
            Log.e(TAG, "getOutputDevices failed:" + e);
        }
        return null;
    }

    public int setCoexistSpdifOther(boolean enable) {
        if (droidAudioServiceIsNull()) return 0;
        try {
            return mDroidAudioService.setCoexistSpdifOther(enable);
        } catch (RemoteException e) {
            Log.e(TAG, "setCoexistSpdifOther failed:" + e);
        }
        return 0;
    }

    public int setMusicStreamVolume(int index) {
        if (droidAudioServiceIsNull()) return 0;
        try {
            return mDroidAudioService.setMusicStreamVolume(index);
        } catch (RemoteException e) {
            Log.e(TAG, "setMusicStreamVolume failed:" + e);
        }
        return 0;
    }
}
