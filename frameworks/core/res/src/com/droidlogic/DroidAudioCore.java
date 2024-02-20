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


import java.util.regex.*;

import android.app.Service;
import android.content.ComponentName;
import android.content.ContentResolver;
import android.content.Context;
import android.content.Intent;
import android.database.ContentObserver;
import android.media.AudioManager;
import android.media.AudioSystem;
import android.net.Uri;
import android.os.Handler;
import android.os.UserHandle;
import android.provider.Settings;
import android.text.TextUtils;
import android.util.Log;

import com.droidlogic.ArcVolumeController;
import com.droidlogic.app.DroidLogicUtils;
import com.droidlogic.app.OutputModeManager;
import com.droidlogic.app.SystemControlManager;
import com.droidlogic.app.AudioConfigManager;
import com.droidlogic.app.DroidAudioManager;
import com.droidlogic.UEventObserver;


public class DroidAudioCore {
    private static final String TAG = "DroidAudioCore";

    private static DroidAudioCore mInstance;

    private DroidAudioManager mDroidAudioManager = null;
    private AudioManager mAudioManager;
    private SystemControlManager mSystemControlManager;
    private OutputModeManager mOutputModeManager;
    private ArcVolumeController mArcVolumeController;
    private SettingsObserver mSettingsObserver;

    private Context mContext;
    private ContentResolver mResolver;


    public static DroidAudioCore getInstance(Context context) {
        if (mInstance == null) {
            synchronized (DroidAudioCore.class) {
                if (mInstance == null) {
                    mInstance = new DroidAudioCore(context);
                }
            }
        }
        return mInstance;
    }

    private DroidAudioCore(Context context) {
        mContext = context;
        mResolver = context.getContentResolver();
        Log.i(TAG, "construction DroidAudioCore");
        mAudioManager = (AudioManager)context.getSystemService(Context.AUDIO_SERVICE);
        mOutputModeManager = OutputModeManager.getInstance(mContext);
        mDroidAudioManager = DroidAudioManager.getInstance(mContext);

        if (!DroidLogicUtils.isTv()) {
            mDroidAudioManager.setSoundBarModeEnabled(mDroidAudioManager.isSoundBarModeEnabled());
        }

        checkDefaultMuteStreams();
        mDroidAudioManager.init();
        mArcVolumeController = new ArcVolumeController(mContext);
        mObserver.startObserving(PATH_AUDIOFORMAT_UEVENT);
        mObserver.startObserving(PATH_TXLX_AUDIOFORMAT_UEVENT);
        SystemControlManager mSystemControlManager = SystemControlManager.getInstance();
        final boolean isSupportDolby = mSystemControlManager.getPropertyBoolean("ro.vendor.platform.support.dolby", false);
        if (isSupportDolby) {
            mDroidAudioManager.setDrcMode(mDroidAudioManager.getDrcMode());
        }

        String[] settings = new String[] {
                DroidAudioManager.ENCODED_SURROUND_OUTPUT,
                DroidAudioManager.ENCODED_SURROUND_OUTPUT_ENABLED_FORMATS,
        };
        mSettingsObserver = new SettingsObserver(new Handler());
        for (String s : settings) {
            mResolver.registerContentObserver(Settings.Global.getUriFor(s), false, mSettingsObserver);
        }
        /*setThisValue for dts scale*/
        mOutputModeManager.setDtsDrcScaleSysfs();
        initDigitalAudioFormat();
        mDroidAudioManager.setARCLatency(mDroidAudioManager.getARCLatency());
        mDroidAudioManager.setSoundSpdifEnable(mDroidAudioManager.getSoundSpdifEnable());
        mDroidAudioManager.setAdSupportEnable(mDroidAudioManager.getAdSupportEnable());
        mDroidAudioManager.setAc4DialogEnhancer(mDroidAudioManager.getAc4DialogEnhancer());
        mDroidAudioManager.setForceDDPEnable(mDroidAudioManager.getForceDDPEnable());

        // refresh db delay of media to hal
        mDroidAudioManager.refreshAudioCfgBySrc(DroidAudioManager.AUDIO_OUTPUT_DELAY_SOURCE_MEDIA, true);
        mDroidAudioManager.setAudioOutputAllDelay(mDroidAudioManager.getAudioOutputAllDelay());
        // refresh db prescale of all source to hal (set one prescale, at the same time the others will be set)
        mDroidAudioManager.setAudioPrescale(DroidAudioManager.AUDIO_OUTPUT_DELAY_SOURCE_ATV,
                                        mDroidAudioManager.getAudioPrescale(DroidAudioManager.AUDIO_OUTPUT_DELAY_SOURCE_ATV));

        // init VAD
        String vadUbootEnable = mSystemControlManager.getBootenv(DroidAudioManager.AUDIO_VAD_UBOOTENV_FFV_WAKE, DroidAudioManager.AUDIO_VAD_STRING_VAD_OFF);
        String property = mSystemControlManager.getPropertyString(DroidAudioManager.AUDIO_VAD_PROPERTY_VADWAKE, DroidAudioManager.AUDIO_VAD_STRING_VAD_OFF);
        Log.i(TAG, "initVadStatus uboot status:" + vadUbootEnable + ", prop:" + property);
        if (vadUbootEnable.equals(DroidAudioManager.AUDIO_VAD_STRING_VAD_ON)) {
            mSystemControlManager.setProperty(DroidAudioManager.AUDIO_VAD_PROPERTY_VADWAKE, vadUbootEnable);
        }
    }

    private static final int DEFAULT_MUTE_STREAMS_AFFECTED =
                    (1 << AudioSystem.STREAM_VOICE_CALL) |
                    (1 << AudioSystem.STREAM_SYSTEM) |
                    (1 << AudioSystem.STREAM_RING) |
                    (1 << AudioSystem.STREAM_MUSIC) |
                    (1 << AudioSystem.STREAM_ALARM) |
                    (1 << AudioSystem.STREAM_NOTIFICATION) |
                    (1 << AudioSystem.STREAM_BLUETOOTH_SCO) |
                    (1 << AudioSystem.STREAM_DTMF) |
                    (1 << AudioSystem.STREAM_TTS) |
                    (1 << AudioSystem.STREAM_ACCESSIBILITY) |
                    (1 << AudioSystem.STREAM_ASSISTANT);
    // need mute all stream volume
    private void checkDefaultMuteStreams() {
        int muteStreamsMask = Settings.System.getInt(mResolver,
                android.provider.Settings.System.MUTE_STREAMS_AFFECTED, AudioSystem.DEFAULT_MUTE_STREAMS_AFFECTED);
        if (muteStreamsMask != DEFAULT_MUTE_STREAMS_AFFECTED) {
            Settings.System.putInt(mContext.getContentResolver(),
                    android.provider.Settings.System.MUTE_STREAMS_AFFECTED, DEFAULT_MUTE_STREAMS_AFFECTED);
            mAudioManager.reloadAudioSettings();
        }
    }

    private boolean needSyncDroidSetting(int surround) {
        int format = Settings.Global.getInt(mResolver, DroidAudioManager.DIGITAL_AUDIO_FORMAT, -1);
        Log.i(TAG, "needSyncDroidSetting internal audio format:" + format + ", android format:" + surround);
        switch (surround) {
            case DroidAudioManager.ENCODED_SURROUND_OUTPUT_AUTO:
                if (format == DroidAudioManager.DIGITAL_AUDIO_FORMAT_AUTO ||
                    format == DroidAudioManager.DIGITAL_AUDIO_FORMAT_PASSTHROUGH) {
                    return false;
                }
                break;
            case DroidAudioManager.ENCODED_SURROUND_OUTPUT_NEVER:
                if (format == DroidAudioManager.DIGITAL_AUDIO_FORMAT_PCM)
                    return false;
                break;
            case DroidAudioManager.ENCODED_SURROUND_OUTPUT_ALWAYS:
            case DroidAudioManager.ENCODED_SURROUND_OUTPUT_MANUAL:
                String subformat = mDroidAudioManager.getAudioManualFormats();
                String subsurround = getSurroundManualFormats();
                if (subsurround == null)
                    subsurround = "";
                if ((format == DroidAudioManager.DIGITAL_AUDIO_FORMAT_MANUAL)
                        && subsurround.equals(subformat)) {
                    return false;
                }
                break;
            default:
                Log.d(TAG, "error surround format");
                break;
        }
        return true;
    }

    private String getSurroundManualFormats() {
        return Settings.Global.getString(mResolver, DroidAudioManager.ENCODED_SURROUND_OUTPUT_ENABLED_FORMATS);
    }

    private class SettingsObserver extends ContentObserver {
        public SettingsObserver(Handler handler) {
            super(handler);
        }

        @Override
        public void onChange(boolean selfChange, Uri uri) {
            String option = uri.getLastPathSegment();
            final int esoValue = Settings.Global.getInt(mResolver,
                DroidAudioManager.ENCODED_SURROUND_OUTPUT,
                DroidAudioManager.ENCODED_SURROUND_OUTPUT_AUTO);

            if (!needSyncDroidSetting(esoValue)) {
                return;
            }
            if (DroidAudioManager.ENCODED_SURROUND_OUTPUT.equals(option)) {
                switch (esoValue) {
                    case DroidAudioManager.ENCODED_SURROUND_OUTPUT_AUTO:
                        mDroidAudioManager.saveDigitalAudioFormatToHal(
                            DroidAudioManager.DIGITAL_AUDIO_FORMAT_AUTO, "");
                        break;
                    case DroidAudioManager.ENCODED_SURROUND_OUTPUT_NEVER:
                        mDroidAudioManager.saveDigitalAudioFormatToHal(
                            DroidAudioManager.DIGITAL_AUDIO_FORMAT_PCM, "");
                        break;
                    case DroidAudioManager.ENCODED_SURROUND_OUTPUT_ALWAYS:
                    case DroidAudioManager.ENCODED_SURROUND_OUTPUT_MANUAL:
                        mDroidAudioManager.saveDigitalAudioFormatToHal(
                            DroidAudioManager.DIGITAL_AUDIO_FORMAT_MANUAL, getSurroundManualFormats());
                        break;
                    default:
                        break;
                }
            } else if (DroidAudioManager.ENCODED_SURROUND_OUTPUT_ENABLED_FORMATS.equals(option)) {
                mDroidAudioManager.saveDigitalAudioFormatToHal(
                    DroidAudioManager.DIGITAL_AUDIO_FORMAT_MANUAL, getSurroundManualFormats());
            }
        }
    }

    private void initDigitalAudioFormat () {
        int audioFormat = mDroidAudioManager.getDigitalAudioFormatOut();
        switch (audioFormat) {
            case DroidAudioManager.DIGITAL_AUDIO_FORMAT_MANUAL:
                String format = mDroidAudioManager.getAudioManualFormats();
                mDroidAudioManager.setDigitalAudioFormatOut(DroidAudioManager.DIGITAL_AUDIO_FORMAT_MANUAL, format);
                break;
            case DroidAudioManager.DIGITAL_AUDIO_FORMAT_PCM:
            case DroidAudioManager.DIGITAL_AUDIO_FORMAT_AUTO:
            default:
                mDroidAudioManager.setDigitalAudioFormatOut(audioFormat);
                break;
        }
    }

    private String covertAudioFormatIndextToString(int audioFormat) {
        String stringValue = " ";
        switch (audioFormat) {
            case 0:
            case 9:
                stringValue = "PCM";
                break;
            case 1:
                stringValue = "DTS Express";
                break;
            case 3:
                stringValue = "DTS";
                break;
            case 5:
                stringValue = "DTS-HD";
                break;
            case 8:
                stringValue = "DTS-HD Master Audio";
                break;
            case 6:
                stringValue = "Multi PCM";
                break;
            case 2:
            case 4:
            case 7:
            case 10:
            case 11:
            case 12:
            case 13:
            case 14:
            case 15:
                stringValue = "Dolby Audio";
                break;
            case 23:
                stringValue = "DTS:X";
                break;
            default:
                Log.w(TAG, "invalid audioFormat value:" + audioFormat);
                break;
        }
        if (DroidLogicUtils.getAudioDebugEnable()) {
            Log.d(TAG, "covertAudioFormatIndextToEnum: audioFormat:" + audioFormat + ", stringVal:" + stringValue);
        }
        return stringValue;
    }

    private static final String PATH_AUDIOFORMAT_UEVENT = "/devices/platform/auge_sound";
    private static final String PATH_NEW_AUDIOFORMAT_UEVENT_REGEX = "/devices/platform/auge_sound/sound/card\\d+/controlC\\d+";
    private static final String PATH_TXLX_AUDIOFORMAT_UEVENT = "/devices/platform/aml_snd_tv";
    private static final String ACTION_AUDIO_FORMAT_CHANGE = "droidlogic.audioservice.action.AUDIO_FORMAT";
    private static final String AUDIO_FORMAT_KEY = "audio_format";
    private static final String AUDIO_FORMAT_VALUE_KEY = "audio_format_value";
    private final UEventObserver mObserver = new UEventObserver() {
        @Override
        public void onUEvent(UEventObserver.UEvent event) {
            if (DroidLogicUtils.getAudioDebugEnable()) {
                Log.d(TAG, "UEVENT: " + event.toString());
                Log.d(TAG, "DEVPATH: " + event.get("DEVPATH"));
            }

            if (isDevicePathMatch(event.get("DEVPATH", null))) {
                String audioFormatStr = event.get("AUDIO_FORMAT", null);
                if (audioFormatStr == null) {
                    Log.e(TAG, "Error! got audio uevent from kernel, but no AUDIO_FORMAT value set!");
                    return;
                }
                if (DroidLogicUtils.getAudioDebugEnable()) {
                    Log.d(TAG, "AUDIO_FORMAT = " + audioFormatStr);
                }
                final int audioFormat = Integer.parseInt(audioFormatStr.substring(audioFormatStr.indexOf("=")+1));
                if (audioFormat < 0) {
                    Log.d(TAG, "ignoring incorrect audio event format:" + audioFormat);
                } else {
                    String extra = covertAudioFormatIndextToString(audioFormat);
                    Intent intent = new Intent(ACTION_AUDIO_FORMAT_CHANGE);
                    intent.putExtra(AUDIO_FORMAT_KEY, extra);
                    intent.putExtra(AUDIO_FORMAT_VALUE_KEY,audioFormat);
                    mContext.sendBroadcastAsUser(intent, UserHandle.ALL);
                }
            }
        }
    };

    private boolean isDevicePathMatch(String devicePath) {
        if (TextUtils.isEmpty(devicePath)) {
            return false;
        }
        boolean result = PATH_AUDIOFORMAT_UEVENT.equals(devicePath) || PATH_TXLX_AUDIOFORMAT_UEVENT.equals(devicePath);
        if (!result) {
            Pattern pattern = Pattern.compile(PATH_NEW_AUDIOFORMAT_UEVENT_REGEX);
            Matcher matcher = pattern.matcher(devicePath);
            result = matcher.matches();
        }
        return result;
    }
}
