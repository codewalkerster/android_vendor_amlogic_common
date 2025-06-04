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

import java.util.Arrays;
import java.util.HashSet;
import java.util.Locale;

import android.content.ComponentName;
import android.content.Context;
import android.content.ContentResolver;
import android.content.Intent;
import android.content.ServiceConnection;
import android.media.AudioFormat;
import android.media.AudioManager;
import android.os.Handler;
import android.os.IBinder;
import android.os.Looper;
import android.os.RemoteException;
import android.provider.Settings;
import android.text.TextUtils;
import android.util.Log;


import com.droidlogic.app.DroidAudioServiceManager;
import com.droidlogic.app.SystemControlManager;

import vendor.amlogic.hardware.droidaudio.IDroidAudio;
import vendor.amlogic.hardware.droidaudio.IDroidAudioClient;

public class DroidAudioManager {
    private static final String TAG = "DroidAudioManager.J";

    private Context mContext;
    private final ContentResolver mResolver;

    private static DroidAudioManager mInstance;
    private static IDroidAudio mDroidAudioService;
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

    public int reset() {
        setDigitalAudioMode(DIGITAL_AUDIO_MODE_AUTO);
        return executeRemoteCall(() -> mDroidAudioService.AudioManager_reset(), "reset", 0);
    }

    public void init() {
        int audioFormat = getDigitalAudioMode();
        switch (audioFormat) {
        case DIGITAL_AUDIO_MODE_MANUAL:
            setDigitalAudioMode(DIGITAL_AUDIO_MODE_MANUAL, getAudioManualFormats());
            break;
        case DIGITAL_AUDIO_MODE_PCM:
        case DIGITAL_AUDIO_MODE_AUTO:
        case DIGITAL_AUDIO_MODE_PASSTHROUGH:
        case DIGITAL_AUDIO_MODE_ALWAYS:
        default:
            setDigitalAudioMode(audioFormat);
            break;
        }
    }

    public static String AudioCmdToString(int cmd) {
        String temp = "[" + cmd + "]";
        switch (cmd) {
            case DROID_AUDIO_CMD_START_DECODE:
                return temp + "START_DECODE";
            case DROID_AUDIO_CMD_PAUSE_DECODE:
                return temp + "PAUSE_DECODE";
            case DROID_AUDIO_CMD_RESUME_DECODE:
                return temp + "RESUME_DECODE";
            case DROID_AUDIO_CMD_STOP_DECODE:
                return temp + "STOP_DECODE";
            case DROID_AUDIO_CMD_SET_DECODE_AD:
                return temp + "SET_DECODE_AD";
            case DROID_AUDIO_CMD_SET_VOLUME:
                return temp + "SET_VOLUME";
            case DROID_AUDIO_CMD_SET_MUTE:
                return temp + "SET_MUTE";
            case DROID_AUDIO_CMD_SET_OUTPUT_MODE:
                return temp + "SET_OUTPUT_MODE";
            case DROID_AUDIO_CMD_SET_PRE_GAIN:
                return temp + "SET_PRE_GAIN";
            case DROID_AUDIO_CMD_SET_PRE_MUTE:
                return temp + "SET_PRE_MUTE";
            case DROID_AUDIO_CMD_OPEN_DECODER:
                return temp + "OPEN_DECODER";
            case DROID_AUDIO_CMD_CLOSE_DECODER:
                return temp + "CLOSE_DECODER";
            case DROID_AUDIO_CMD_SET_DEMUX_INFO:
                return temp + "SET_DEMUX_INFO";
            case DROID_AUDIO_CMD_SET_SECURITY_MEM_LEVEL:
                return temp + "SET_SECURITY_MEM_LEVEL";

            case DROID_AUDIO_CMD_AD_SWITCH_ENABLE:
                return temp + "AD_SWITCH_ENABLE";
            case DROID_AUDIO_CMD_AD_SET_VOLUME:
                return temp + "AD_SET_VOLUME";
            case DROID_AUDIO_CMD_AD_DUAL_SUPPORT:
                return temp + "AD_DUAL_SUPPORT";
            case DROID_AUDIO_CMD_AD_MIX_SUPPORT:
                return temp + "AD_MIX_SUPPORT";
            case DROID_AUDIO_CMD_AD_MIX_LEVEL:
                return temp + "AD_MIX_LEVEL";
            case DROID_AUDIO_CMD_AD_SET_MAIN:
                return temp + "AD_SET_MAIN";
            case DROID_AUDIO_CMD_AD_SET_ASSOCIATE:
                return temp + "AD_SET_ASSOCIATE";
            case DROID_AUDIO_CMD_SET_HAS_VIDEO:
                return temp + "SET_HAS_VIDEO";
            case DROID_AUDIO_CMD_SET_MEDIA_PRESENTATION_ID:
                return temp + "SET_MEDIA_PRESENTATION_ID";
            case DROID_AUDIO_CMD_SET_AUDIO_PATCH_MANAGE_MODE:
                return temp + "SET_AUDIO_PATCH_MANAGE_MODE";
            case DROID_AUDIO_CMD_SET_SPDIF_PROTECTION_MODE:
                return temp + "SET_SPDIF_PROTECTION_MODE";
            case DROID_AUDIO_CMD_SET_TSPLAYER_CLIENT_DIED:
                return temp + "SET_TSPLAYER_CLIENT_DIED";
            case DROID_AUDIO_CMD_SET_MEDIA_FIRST_LANG:
                return temp + "SET_MEDIA_FIRST_LANG";
            case DROID_AUDIO_CMD_SET_MEDIA_SECOND_LANG:
                return temp + "SET_MEDIA_SECOND_LANG";
            default:
                return temp + "invalid cmd";
        }
    }
    public static final int DROID_AUDIO_CMD_START_DECODE                          = 1;
    public static final int DROID_AUDIO_CMD_PAUSE_DECODE                          = 2;
    public static final int DROID_AUDIO_CMD_RESUME_DECODE                         = 3;
    public static final int DROID_AUDIO_CMD_STOP_DECODE                           = 4;
    public static final int DROID_AUDIO_CMD_SET_DECODE_AD                         = 5;
    public static final int DROID_AUDIO_CMD_SET_VOLUME                            = 6;
    public static final int DROID_AUDIO_CMD_SET_MUTE                              = 7;
    public static final int DROID_AUDIO_CMD_SET_OUTPUT_MODE                       = 8;
    public static final int DROID_AUDIO_CMD_SET_PRE_GAIN                          = 9;
    public static final int DROID_AUDIO_CMD_SET_PRE_MUTE                          = 10;
    public static final int DROID_AUDIO_CMD_OPEN_DECODER                          = 12;
    public static final int DROID_AUDIO_CMD_CLOSE_DECODER                         = 13;
    public static final int DROID_AUDIO_CMD_SET_DEMUX_INFO                        = 14;
    public static final int DROID_AUDIO_CMD_SET_SECURITY_MEM_LEVEL                = 15;
    public static final int DROID_AUDIO_CMD_SET_HAS_VIDEO                         = 16;
    public static final int DROID_AUDIO_CMD_SET_MEDIA_SYCN_ID                     = 17;
    //audio ad
    public static final int DROID_AUDIO_CMD_AD_SWITCH_ENABLE                      = 18;
    public static final int DROID_AUDIO_CMD_AD_SET_VOLUME                         = 19;
    public static final int DROID_AUDIO_CMD_AD_DUAL_SUPPORT                       = 20;
    public static final int DROID_AUDIO_CMD_AD_MIX_SUPPORT                        = 21;
    public static final int DROID_AUDIO_CMD_AD_MIX_LEVEL                          = 22;
    public static final int DROID_AUDIO_CMD_AD_SET_MAIN                           = 23;
    public static final int DROID_AUDIO_CMD_AD_SET_ASSOCIATE                      = 24;
    public static final int DROID_AUDIO_CMD_SET_MEDIA_PRESENTATION_ID             = 25;
    public static final int DROID_AUDIO_CMD_SET_AUDIO_PATCH_MANAGE_MODE           = 26;
    public static final int DROID_AUDIO_CMD_SET_SPDIF_PROTECTION_MODE             = 27;
    public static final int DROID_AUDIO_CMD_SET_TSPLAYER_CLIENT_DIED              = 28;
    public static final int DROID_AUDIO_CMD_SET_MEDIA_FIRST_LANG                  = 29;
    public static final int DROID_AUDIO_CMD_SET_MEDIA_SECOND_LANG                 = 30;
    public static final int DROID_AUDIO_CMD_SET_AUDIO_PICTURE_MODE                = 31;
    public int setAudioCmdParam(int cmd, int param1, int param2, int param3) {
        return executeRemoteCall(() -> mDroidAudioService.AudioManager_setAudioCmdParam(cmd, param1, param2, param3), "setAudioCmdParam", 0);
    }

    public static final String DB_ID_DROIDLOGIC_AUDIO_OUTPUT_DEVICE                 = "db_id_droidlogic_audio_output_device";
    public static final String PROP_AUDIO_OUTPUT_STRATEGY                           = "persist.vendor.media.audio.output.strategy";
    /* 0: Auto  1: Semi-Auto  2: Manual (refer to: audio_output_strategy enum in Engine.cpp) */
    public static final int OUTPUT_STRATEGY_AUTO                                    = 0;
    public static final int OUTPUT_STRATEGY_SEMI_AUTO                               = 1;
    public static final int OUTPUT_STRATEGY_MANUAL                                  = 2;
    // audio_policy_forced_cfg_t (system\media\audio\include\system\audio_policy.h)
    public static final int DROID_AUDIO_FORCE_USE_NONE                              = 0; // AUDIO_POLICY_FORCE_NONE
    public static final int DROID_AUDIO_FORCE_USE_SPEAKER                           = 1; // AUDIO_POLICY_FORCE_SPEAKER
    public static final int DROID_AUDIO_FORCE_USE_SPDIF                             = 8; // AUDIO_POLICY_FORCE_ANALOG_DOCK
    public static final int DROID_AUDIO_FORCE_USE_HDMI                              = 9; // AUDIO_POLICY_FORCE_DIGITAL_DOCK
    public static final int DROID_AUDIO_FORCE_USE_HEADPHONES                        = 2; // AUDIO_POLICY_FORCE_HEADPHONES
    public static final int DROID_AUDIO_FORCE_USE_USB                               = 5; // AUDIO_POLICY_FORCE_WIRED_ACCESSORY
    public static final int DROID_AUDIO_FORCE_USE_BT_A2DP                           = 4; // AUDIO_POLICY_FORCE_BT_A2DP
    public int setOutputDevices(int[] devices) {
        // TODO: for Netlifx, switch netflix profile.
        if (devices != null & devices.length != 0) {
            Settings.Global.putInt(mResolver, DB_ID_DROIDLOGIC_AUDIO_OUTPUT_DEVICE, devices[0]);
        }
        return executeRemoteCall(() -> mDroidAudioService.AudioManager_setOutputDevices(devices), "setOutputDevices", 0);
    }
    public int[] getOutputDevices() {
        return executeRemoteCall(() -> mDroidAudioService.AudioManager_getOutputDevices(), "getOutputDevices", null);
    }

    public static final String PROP_AUDIO_OUTPUT_SPDIF_COEXIST                      = "persist.vendor.media.audio.spdif.coexist";
    public static final String PROP_AUDIO_OUTPUT_SPDIF_SUPPORT                      = "persist.vendor.media.audio.spdif.support";
    public int setCoexistSpdifOtherEnabled(boolean enable) {
        return executeRemoteCall(() -> mDroidAudioService.AudioManager_setCoexistSpdifOtherEnabled(enable), "setCoexistSpdifOtherEnabled", 0);
    }
    public boolean isCoexistSpdifOtherEnabled() {
        return executeRemoteCall(() -> mDroidAudioService.AudioManager_isCoexistSpdifOtherEnabled(), "isCoexistSpdifOtherEnabled", false);
    }

    private static final String DB_ID_AUDIO_SOUNDBAR_MODE_ENABLE            = "soundbar_mode";
    public int setSoundBarModeEnabled(boolean enable) {
        Settings.Global.putInt(mResolver, DB_ID_AUDIO_SOUNDBAR_MODE_ENABLE, enable ? 1 : 0);
        return executeRemoteCall(() -> mDroidAudioService.AudioManager_setSoundBarModeEnabled(enable), "setSoundBarModeEnabled", 0);
    }
    public boolean isSoundBarModeEnabled() {
        return executeRemoteCall(() -> mDroidAudioService.AudioManager_isSoundBarModeEnabled(), "isSoundBarModeEnabled", false);
    }

    public int setSoundSpdifEnabled(boolean enable) {
        return executeRemoteCall(() -> mDroidAudioService.AudioManager_setSoundSpdifEnabled(enable), "setSoundSpdifEnabled", 0);
    }
    public boolean isSoundSpdifEnabled() {
        return executeRemoteCall(() -> mDroidAudioService.AudioManager_isSoundSpdifEnabled(), "isSoundSpdifEnabled", false);
    }

    public int setSpeakerEnabled(boolean enable) {
        return executeRemoteCall(() -> mDroidAudioService.AudioManager_setSpeakerEnabled(enable), "setSpeakerEnabled", 0);
    }
    public boolean isSpeakerEnabled() {
        return executeRemoteCall(() -> mDroidAudioService.AudioManager_isSpeakerEnabled(), "isSpeakerEnabled", false);
    }

    public static final int AUDIO_OUTPUT_DELAY_SOURCE_ATV                              = 0;
    public static final int AUDIO_OUTPUT_DELAY_SOURCE_DTV                              = 1;
    public static final int AUDIO_OUTPUT_DELAY_SOURCE_AV                               = 2;
    public static final int AUDIO_OUTPUT_DELAY_SOURCE_HDMI                             = 3;
    public static final int AUDIO_OUTPUT_DELAY_SOURCE_MEDIA                            = 4;
    public static final int AUDIO_OUTPUT_DELAY_SOURCE_MIN                              = AUDIO_OUTPUT_DELAY_SOURCE_ATV;
    public static final int AUDIO_OUTPUT_DELAY_SOURCE_MAX                              = AUDIO_OUTPUT_DELAY_SOURCE_MEDIA;
    // device
    public static final int AUDIO_OUT_DELAY_DEV_HAL_SPEAKER                            = 0;
    public static final int AUDIO_OUT_DELAY_DEV_HAL_SPDIF                              = 1;
    public static final int AUDIO_OUT_DELAY_DEV_HAL_HEADPHONE                          = 2;
    public static final int AUDIO_OUT_DELAY_DEV_HAL_ALL                                = 3;
    // delayMs
    public static final int AUDIO_OUT_DELAY_HAL_MIN                                    = 0;       // ms
    public static final int AUDIO_OUT_DELAY_HAL_MAX                                    = 260;     // ms
    public int setOutputDeviceDelay(int source, int device, int delayMs) {
        return executeRemoteCall(() -> mDroidAudioService.AudioManager_setOutputDeviceDelay(source, device, delayMs), "setOutputDeviceDelay", 0);
    }
    public int getOutputDeviceDelay(int source, int device) {
        return executeRemoteCall(() -> mDroidAudioService.AudioManager_getOutputDeviceDelay(source, device), "getOutputDeviceDelay", 0);
    }

    public int setAudioOutputAllDelay(int delayMs) {
        return executeRemoteCall(() -> mDroidAudioService.AudioManager_setAudioOutputAllDelay(delayMs), "setAudioOutputAllDelay", 0);
    }
    public int getAudioOutputAllDelay() {
        return executeRemoteCall(() -> mDroidAudioService.AudioManager_getAudioOutputAllDelay(), "getAudioOutputAllDelay", 0);
    }

    public int setTvSourceType(int source) {
        return executeRemoteCall(() -> mDroidAudioService.AudioManager_setTvSourceType(source), "setTvSourceType", 0);
    }
    public int getTvSourceType() {
        return executeRemoteCall(() -> mDroidAudioService.AudioManager_getTvSourceType(), "getTvSourceType", 0);
    }

    public int setAudioApplyToAll() {
        return executeRemoteCall(() -> mDroidAudioService.AudioManager_setAudioApplyToAll(), "setAudioApplyToAll", 0);
    }

    public static String audioFormatToName(int audioFormat) {
        switch (audioFormat) {
            case AudioFormat.ENCODING_AC3:
                return "Dolby Digital";
            case AudioFormat.ENCODING_E_AC3:
                return "Dolby Digital Plus";
            case AudioFormat.ENCODING_DTS:
                return "DTS";
            case AudioFormat.ENCODING_DTS_HD:
                return "DTS HD";
            case AudioFormat.ENCODING_AAC_LC:
                return "AAC";
            case AudioFormat.ENCODING_DOLBY_TRUEHD:
                return "Dolby TrueHD";
            case AudioFormat.ENCODING_AC4:
                return "Dolby AC-4";
            case AudioFormat.ENCODING_E_AC3_JOC:
                return "Dolby Atmos in Dolby Digital Plus";
            case AudioFormat.ENCODING_DOLBY_MAT:
                return "Dolby MAT";
            case AudioFormat.ENCODING_MPEGH_BL_L3:
                return "MPEG-H 3D Audio baseline profile level 3";
            case AudioFormat.ENCODING_MPEGH_BL_L4:
                return "MPEG-H 3D Audio baseline profile level 4";
            case AudioFormat.ENCODING_MPEGH_LC_L3:
                return "MPEG-H 3D Audio low complexity profile level 3";
            case AudioFormat.ENCODING_MPEGH_LC_L4:
                return "MPEG-H 3D Audio low complexity profile level 4";
            case AudioFormat.ENCODING_DTS_UHD_P1:
                return "DTS UHD Profile 1";
            case AudioFormat.ENCODING_DRA:
                return "DRA";
            case AudioFormat.ENCODING_DTS_HD_MA:
                return "DTS HD Master Audio";
            case AudioFormat.ENCODING_DTS_UHD_P2:
                return "DTS UHD Profile 2";
            default:
                return "Unknown surround sound format";
        }
    }
    public static String surroundModeToString(int value) {
        String temp = "[" + value + "]";
        switch (value) {
            case AudioManager.ENCODED_SURROUND_OUTPUT_AUTO:
                return temp + "AUTO";
            case AudioManager.ENCODED_SURROUND_OUTPUT_NEVER:
                return temp + "NEVER";
            case AudioManager.ENCODED_SURROUND_OUTPUT_ALWAYS:
                return temp + "ALWAYS";
            case AudioManager.ENCODED_SURROUND_OUTPUT_MANUAL:
                return temp + "MANUAL";
            default:
                return temp + "INVALID_VALUE";
        }
    }
    //surround sound formats, must sync with Settings.Global
    static final int[] SURROUND_SOUND_DISPLAY_ORDER = {
        AudioFormat.ENCODING_AC3, AudioFormat.ENCODING_E_AC3, AudioFormat.ENCODING_DOLBY_TRUEHD,
        AudioFormat.ENCODING_E_AC3_JOC, AudioFormat.ENCODING_DOLBY_MAT,
        AudioFormat.ENCODING_DTS, AudioFormat.ENCODING_DTS_HD, AudioFormat.ENCODING_DTS_UHD,
        AudioFormat.ENCODING_DRA
    };
    private static final String SURROUND_SOUND_ALWAYS_FORMATS                           =
                    Arrays.toString(SURROUND_SOUND_DISPLAY_ORDER).replace("[", "").
                    replace("]", "").replace(" ", "");
    public static final String ENCODED_SURROUND_OUTPUT                  = "encoded_surround_output";
    public static final String ENCODED_SURROUND_OUTPUT_ENABLED_FORMATS  = "encoded_surround_output_enabled_formats";
    public static final int ENCODED_SURROUND_OUTPUT_AUTO                = 0;
    public static final int ENCODED_SURROUND_OUTPUT_NEVER               = 1;
    public static final int ENCODED_SURROUND_OUTPUT_ALWAYS              = 2;
    public static final int ENCODED_SURROUND_OUTPUT_MANUAL              = 3;
    public void setSurroundModeToAndroid(int mode, String formats) {
        int surroundMode = AudioManager.ENCODED_SURROUND_OUTPUT_AUTO;
        switch (mode) {
            case DIGITAL_AUDIO_MODE_MANUAL:
                Settings.Global.putInt(mResolver, ENCODED_SURROUND_OUTPUT, ENCODED_SURROUND_OUTPUT_MANUAL);
                String subFormats = getAudioManualFormats();
                if (!formats.equals(subFormats)) {
                    Settings.Global.putString(mResolver, ENCODED_SURROUND_OUTPUT_ENABLED_FORMATS, formats);
                }
                surroundMode = AudioManager.ENCODED_SURROUND_OUTPUT_MANUAL;
                break;
            case DIGITAL_AUDIO_MODE_ALWAYS:
                surroundMode = AudioManager.ENCODED_SURROUND_OUTPUT_ALWAYS;
                break;
            case DIGITAL_AUDIO_MODE_PASSTHROUGH:
            case DIGITAL_AUDIO_MODE_AUTO:
                surroundMode = AudioManager.ENCODED_SURROUND_OUTPUT_AUTO;
                break;
            case DIGITAL_AUDIO_MODE_PCM:
                surroundMode = AudioManager.ENCODED_SURROUND_OUTPUT_NEVER;
                break;
            default:
                Log.w(TAG, "setSurroundModeToAndroid not support digital mode:" + mode);
                return;
        }
        Log.i(TAG, "setSurroundModeToAndroid set digital mode:" + surroundModeToString(surroundMode));
        AudioManager mAudioManager = (AudioManager)mContext.getSystemService(Context.AUDIO_SERVICE);
        mAudioManager.setEncodedSurroundMode(surroundMode);
    }

    public static String digitalModeToString(int value) {
        String temp = "[" + value + "]";
        switch (value) {
            case DIGITAL_AUDIO_MODE_PCM:
                return temp + "MODE_PCM";
            case DIGITAL_AUDIO_MODE_AUTO:
                return temp + "MODE_AUTO";
            case DIGITAL_AUDIO_MODE_MANUAL:
                return temp + "MODE_MANUAL";
            case DIGITAL_AUDIO_MODE_PASSTHROUGH:
                return temp + "MODE_PASSTHROUGH";
            case DIGITAL_AUDIO_MODE_ALWAYS:
                return temp + "MODE_ALWAYS";
            default:
                return temp + "INVALID_VALUE";
        }
    }
    public static final int DIGITAL_AUDIO_MODE_PCM                                          = 0;
    public static final int DIGITAL_AUDIO_MODE_AUTO                                         = 1;
    public static final int DIGITAL_AUDIO_MODE_MANUAL                                       = 2;
    public static final int DIGITAL_AUDIO_MODE_PASSTHROUGH                                  = 3;
    public static final int DIGITAL_AUDIO_MODE_ALWAYS                                       = 4;
    public static final int DIGITAL_AUDIO_MODE_MIN                                          = DIGITAL_AUDIO_MODE_PCM;
    public static final int DIGITAL_AUDIO_MODE_MAX                                          = DIGITAL_AUDIO_MODE_ALWAYS;
    public int setDigitalAudioModeToHal(int mode, String formats) {
        return executeRemoteCall(() -> mDroidAudioService.AudioManager_setDigitalAudioMode(mode, formats), "setDigitalAudioModeToHal", 0);
    }
    public void setDigitalAudioMode(int mode) {
        setDigitalAudioMode(mode, "");
    }
    public void setDigitalAudioMode(int mode, String formats) {
        if (mode < DIGITAL_AUDIO_MODE_MIN || mode > DIGITAL_AUDIO_MODE_MAX) {
            Log.e(TAG, "setDigitalAudioMode invalid audioMode:" + mode + ", formats:" + formats);
            return;
        }
        Log.d(TAG, "setDigitalAudioMode audioMode:" + digitalModeToString(mode) + ", formats:" + formats);
        if (DroidLogicUtils.getAudioDebugEnable() && formats != null) {
            String tempFromats = formats.trim();
            if (!tempFromats.isEmpty()) {
                String[] formatArrStr = tempFromats.split(",");
                for (int i = 0; i < formatArrStr.length; i++) {
                    int format = Integer.parseInt(formatArrStr[i]);
                    Log.d(TAG, "setDigitalAudioMode format[" + i + "]:" + "(" + format + ") " + audioFormatToName(format));
                }
            }
        }
        if (DIGITAL_AUDIO_MODE_MANUAL == mode && formats == null) {
            formats = "";
            Log.w(TAG, "setDigitalAudioMode manual mode, formats is null.");
        }
        setDigitalAudioModeToHal(mode, formats);
        setSurroundModeToAndroid(mode, formats);
    }
    public int getDigitalAudioMode() {
        return executeRemoteCall(() -> mDroidAudioService.AudioManager_getDigitalAudioMode(), "getDigitalAudioMode", 0);
    }

    public void setAudioManualFormats(int id, boolean enabled) {
        HashSet<Integer> fmts = new HashSet<>();
        String formats = getAudioManualFormats();
        if (!formats.isEmpty()) {
            try {
                Arrays.stream(formats.split(",")).mapToInt(Integer::parseInt)
                    .forEach(fmts::add);
            } catch (NumberFormatException e) {
                Log.w(TAG, "ENCODED_SURROUND_OUTPUT_ENABLED_FORMATS misformatted.", e);
            }
        }
        if (enabled) {
            fmts.add(id);
        } else {
            fmts.remove(id);
        }
        setDigitalAudioMode(DIGITAL_AUDIO_MODE_MANUAL, TextUtils.join(",", fmts));
    }
    public String getAudioManualFormats() {
        String formats = Settings.Global.getString(mResolver, ENCODED_SURROUND_OUTPUT_ENABLED_FORMATS);
        if (DroidLogicUtils.getAudioDebugEnable()) Log.i(TAG, "getAudioManualFormats formats:" + formats);
        return (formats == null) ? "" : formats;
    }

    public int setForceDDPEnabled(boolean enable) {
        return executeRemoteCall(() -> mDroidAudioService.AudioManager_setForceDDPEnabled(enable), "setForceDDPEnabled", 0);
    }
    public boolean isForceDDPEnabled() {
        return executeRemoteCall(() -> mDroidAudioService.AudioManager_isForceDDPEnabled(), "isForceDDPEnabled", false);
    }

    public static final int DOLBY_DRC_MODE_OFF                                                           = 0;
    public static final int DOLBY_DRC_MODE_LINE                                                          = 1;
    public static final int DOLBY_DRC_MODE_RF                                                            = 2;
    public static final int DOLBY_DRC_MODE_MIN                                                           = DOLBY_DRC_MODE_OFF;
    public static final int DOLBY_DRC_MODE_MAX                                                           = DOLBY_DRC_MODE_RF;
    public int setDolbyDrcMode(int mode) {
        return executeRemoteCall(() -> mDroidAudioService.AudioManager_setDolbyDrcMode(mode), "setDolbyDrcMode", 0);
    }
    public int getDolbyDrcMode() {
        return executeRemoteCall(() -> mDroidAudioService.AudioManager_getDolbyDrcMode(), "getDolbyDrcMode", 0);
    }

    public int setDolbyDrcLineLevel(int level) {
        return executeRemoteCall(() -> mDroidAudioService.AudioManager_setDolbyDrcLineLevel(level), "setDolbyDrcLineLevel", 0);
    }
    public int getDolbyDrcLineLevel() {
        return executeRemoteCall(() -> mDroidAudioService.AudioManager_getDolbyDrcLineLevel(), "getDolbyDrcLineLevel", 0);
    }

    public boolean isDtsXEnabled() {
        return executeRemoteCall(() -> mDroidAudioService.AudioManager_isDtsXEnabled(), "isDtsXEnabled", false);
    }

    public int setDtsXDrcEnabled(boolean enable) {
        return executeRemoteCall(() -> mDroidAudioService.AudioManager_setDtsXDrcEnabled(enable), "setDtsXDrcEnabled", 0);
    }
    public boolean isDtsXDrcEnabled() {
        return executeRemoteCall(() -> mDroidAudioService.AudioManager_isDtsXDrcEnabled(), "isDtsXDrcEnabled", false);
    }

    public static final int DIALOGUE_ENHANCEMENT_LEVEL_OFF                                                  = 0;
    public static final int DIALOGUE_ENHANCEMENT_LEVEL_LOW                                                  = 1;
    public static final int DIALOGUE_ENHANCEMENT_LEVEL_MEDIUM                                               = 2;
    public static final int DIALOGUE_ENHANCEMENT_LEVEL_HIGH                                                 = 3;
    public static final int DIALOGUE_ENHANCEMENT_LEVEL_MIN                                                  = DIALOGUE_ENHANCEMENT_LEVEL_OFF;
    public static final int DIALOGUE_ENHANCEMENT_LEVEL_MAX                                                  = DIALOGUE_ENHANCEMENT_LEVEL_HIGH;
    public int setDialogEnhancerLevel(int level) {
        return executeRemoteCall(() -> mDroidAudioService.AudioManager_setDialogEnhancerLevel(level), "setDialogEnhancerLevel", 0);
    }
    public int getDialogEnhancerLevel() {
        return executeRemoteCall(() -> mDroidAudioService.AudioManager_getDialogEnhancerLevel(), "getDialogEnhancerLevel", 0);
    }

    public static final int DOLBY_SOUND_DMX_MODE_SURROUND                                                   = 0;
    public static final int DOLBY_SOUND_DMX_MODE_STEREO                                                     = 1;
    public static final int DOLBY_SOUND_DMX_MODE_MIN                                                        = DOLBY_SOUND_DMX_MODE_SURROUND;
    public static final int DOLBY_SOUND_DMX_MODE_MAX                                                        = DOLBY_SOUND_DMX_MODE_STEREO;
    public int setSoundDmxMode(int mode) {
        return executeRemoteCall(() -> mDroidAudioService.AudioManager_setSoundDmxMode(mode), "setSoundDmxMode", 0);
    }
    public int getSoundDmxMode() {
        return executeRemoteCall(() -> mDroidAudioService.AudioManager_getSoundDmxMode(), "getSoundDmxMode", 0);
    }

    public static final int DOLBY_SOUND_LEVELER_MODE_OFF = 0;
    public static final int DOLBY_SOUND_LEVELER_MODE_ON = 1;
    public static final int DOLBY_SOUND_LEVELER_MODE_AUTO = 2;
    public static final int DOLBY_SOUND_LEVELER_MODE_MIN = DOLBY_SOUND_LEVELER_MODE_OFF;
    public static final int DOLBY_SOUND_LEVELER_MODE_MAX = DOLBY_SOUND_LEVELER_MODE_AUTO;

    public int setSoundLevelerMode(int mode) {
        return executeRemoteCall(() -> mDroidAudioService.AudioManager_setSoundLevelerMode(mode), "setSoundLevelerMode", 0);
    }

    public int getSoundLevelerMode() {
        return executeRemoteCall(() -> mDroidAudioService.AudioManager_getSoundLevelerMode(), "getSoundLevelerMode", 0);
    }

    public int setSoundLevelerAmount(int value) {
        return executeRemoteCall(() -> mDroidAudioService.AudioManager_setSoundLevelerAmount(value), "setSoundLevelerAmount", 0);
    }

    public int getSoundLevelerAmount() {
        return executeRemoteCall(() -> mDroidAudioService.AudioManager_getSoundLevelerAmount(), "getSoundLevelerAmount", 0);
    }

    public int setVadEnabled(boolean enable) {
        return executeRemoteCall(() -> mDroidAudioService.AudioManager_setVadEnabled(enable), "setVadEnabled", 0);
    }
    public boolean isVadEnabled() {
        return executeRemoteCall(() -> mDroidAudioService.AudioManager_isVadEnabled(), "isVadEnabled", false);
    }

    public static final int SOURCE_TYPE_ATV                                                                 = 0;
    public static final int SOURCE_TYPE_DTV                                                                 = 1;
    public int openTvAudio(int source) {
        return executeRemoteCall(() -> mDroidAudioService.AudioManager_openTvAudio(source), "openTvAudio", 0);
    }

    public static final int DROID_AUDIO_CONFIG_ID_IS_DRIVER_BASE                                            = 0;
    public static final int DROID_AUDIO_CONFIG_ID_IS_SUPPORT_MS12                                           = 1;
    public int getDroidAudioConfig(int id) {
        return executeRemoteCall(() -> mDroidAudioService.AudioManager_getDroidAudioConfig(id), "getDroidAudioConfig", 0);
    }

    // sourceDevice: DEVICE_IN_TV_TUNER...  frameworks/base/media/java/android/media/AudioSystem.java
    // sinkDevice: DEVICE_OUT_SPEAKER...  frameworks/base/media/java/android/media/AudioSystem.java
    public int createAudioPatch(int sourceDevice, int sinkDevice) {
        return executeRemoteCall(() -> mDroidAudioService.AudioManager_createAudioPatch(sourceDevice, sinkDevice), "createAudioPatch", 0);

    }
    public int releaseAudioPatch(int handle) {
        return executeRemoteCall(() -> mDroidAudioService.AudioManager_releaseAudioPatch(handle), "releaseAudioPatch", 0);
    }

    public int setAiDeEnabled(boolean enable) {
        return executeRemoteCall(() -> mDroidAudioService.AudioManager_setAiDeEnabled(enable), "setAiDeEnabled", 0);
    }

    public boolean isAiDeEnabled() {
        return executeRemoteCall(() -> mDroidAudioService.AudioManager_isAiDeEnabled(), "isAiDeEnabled", false);
    }

    public int setAiDeGain(int gain) {
        return executeRemoteCall(() -> mDroidAudioService.AudioManager_setAiDeGain(gain), "setAiDeGain", 0);
    }

    public int getAiDeGain() {
        return executeRemoteCall(() -> mDroidAudioService.AudioManager_getAiDeGain(), "getAiDeGain", 0);
    }

    public static final int MIC_SOURCE_TYPE_BUILT_IN                                                        = 0;  //built in mic
    public static final int MIC_SOURCE_TYPE_LINE_IN                                                         = 1;  //line in mic
    public static final int MIC_SOURCE_TYPE_USB_IN                                                          = 2;  //line in mic
    public int setGlobalMicEnable(int source, boolean enable) {
        return executeRemoteCall(() -> mDroidAudioService.AudioManager_setGlobalMicEnable(source, enable), "setGlobalMicEnable", 0);
    }
    public boolean getGlobalMicStatus(int source) {
        return executeRemoteCall(() -> mDroidAudioService.AudioManager_getGlobalMicStatus(source), "getGlobalMicStatus", false);
    }
    public int setMicSource(int source) {
        return executeRemoteCall(() -> mDroidAudioService.AudioManager_setMicSource(source), "setMicSource", 0);
    }
    public int getMicSource() {
        return executeRemoteCall(() -> mDroidAudioService.AudioManager_getMicSource(), "getMicSource", 0);
    }
    public int setMicMute(int source, boolean mute) {
        return executeRemoteCall(() -> mDroidAudioService.AudioManager_setMicMute(source, mute), "setMicMute", 0);
    }
    public boolean isMicMute(int source) {
        return executeRemoteCall(() -> mDroidAudioService.AudioManager_isMicMute(source), "isMicMute", false);
    }

    /* mic gain rang:[0 100] db
       Value mapping rule: UI index - setting 0  mapping minum volume, such as -999999
        - UI            :    Setting
        - 0                  minum (-100)
        - Val [ 1 100]       Val - 10
    */
    public int setMicGain(int source, int gain) {
        return executeRemoteCall(() -> mDroidAudioService.AudioManager_setMicGain(source, gain), "setMicGain", 0);
    }
    public int getMicGain(int source) {
        return executeRemoteCall(() -> mDroidAudioService.AudioManager_getMicGain(source), "getMicGain", 0);
    }
    public int setMicReverb(int source, boolean enable) {
        return executeRemoteCall(() -> mDroidAudioService.AudioManager_setMicReverb(source, enable), "setMicReverb", 0);
    }
    public boolean isEnableMicReverb(int source) {
        return executeRemoteCall(() -> mDroidAudioService.AudioManager_isEnableMicReverb(source), "isEnableMicReverb", false);
    }

    //Reverb level rang int: [0, 5]
    public int setMicReverbLevel(int source, int level) {
        return executeRemoteCall(() -> mDroidAudioService.AudioManager_setMicReverbLevel(source, level), "setMicReverbLevel", 0);
    }
    public int getMicReverbLevel(int source) {
        return executeRemoteCall(() -> mDroidAudioService.AudioManager_getMicReverbLevel(source), "getMicReverbLevel", 0);
    }
}
