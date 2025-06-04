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
import android.os.SystemProperties;
import android.os.UserHandle;
import android.provider.Settings;
import android.text.TextUtils;
import android.util.Log;

import com.droidlogic.app.DroidLogicUtils;
import com.droidlogic.app.OutputModeManager;
import com.droidlogic.app.SystemControlManager;
import com.droidlogic.app.DroidAudioManager;
import com.droidlogic.UEventObserver;


public class DroidAudioCore {
    private static final String TAG = "DroidAudioCore";

    private static DroidAudioCore mInstance;

    private DroidAudioManager mDroidAudioManager = null;
    private AudioManager mAudioManager;
    private SystemControlManager mSystemControlManager;
    private OutputModeManager mOutputModeManager;
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
        mDroidAudioManager = DroidAudioManager.getInstance(mContext);
        if (mDroidAudioManager.getDroidAudioConfig(DroidAudioManager.DROID_AUDIO_CONFIG_ID_IS_DRIVER_BASE) != 0) {
            Log.i(TAG, "driver base project, do not init");
            return;
        }
        mAudioManager = (AudioManager)context.getSystemService(Context.AUDIO_SERVICE);
        mOutputModeManager = OutputModeManager.getInstance(mContext);
        mObserver.startObserving(PATH_AUDIOFORMAT_UEVENT);
        mObserver.startObserving(PATH_TXLX_AUDIOFORMAT_UEVENT);

        String[] settings = new String[] {
                DroidAudioManager.ENCODED_SURROUND_OUTPUT,
                DroidAudioManager.ENCODED_SURROUND_OUTPUT_ENABLED_FORMATS,
                Settings.Global.USER_PREFERRED_RESOLUTION_HEIGHT,
        };
        mSettingsObserver = new SettingsObserver(new Handler());
        for (String s : settings) {
            mResolver.registerContentObserver(Settings.Global.getUriFor(s), false, mSettingsObserver);
        }
        mDroidAudioManager.init();
        /*setThisValue for dts scale*/
        mOutputModeManager.setDtsDrcScaleSysfs();
    }

    private class SettingsObserver extends ContentObserver {
        public SettingsObserver(Handler handler) {
            super(handler);
        }

        private int surroundModeToDigitalMode(int surroundMode) {
            switch (surroundMode) {
                case Settings.Global.ENCODED_SURROUND_OUTPUT_AUTO:
                    return DroidAudioManager.DIGITAL_AUDIO_MODE_AUTO;
                case Settings.Global.ENCODED_SURROUND_OUTPUT_NEVER:
                    return DroidAudioManager.DIGITAL_AUDIO_MODE_PCM;
                case Settings.Global.ENCODED_SURROUND_OUTPUT_ALWAYS:
                    return DroidAudioManager.DIGITAL_AUDIO_MODE_ALWAYS;
                case Settings.Global.ENCODED_SURROUND_OUTPUT_MANUAL:
                    return DroidAudioManager.DIGITAL_AUDIO_MODE_MANUAL;
                default:
                    Log.w(TAG, "surroundModeToDigitalMode invalid surroundMode:" + surroundMode + ", return AUTO");
                    return DroidAudioManager.DIGITAL_AUDIO_MODE_AUTO;
            }
        }

        private boolean needSyncDroidSetting(int surroundMode) {
            int format = mDroidAudioManager.getDigitalAudioMode();
            int digitalMode = surroundModeToDigitalMode(surroundMode);
            boolean needSync = false;
            if (surroundMode == -1) {
                Log.w(TAG, "needSyncDroidSetting invalid surround mode:" + surroundMode);
                return true;
            }
            if (digitalMode == format) {
                // do nothing
            } else if (digitalMode == DroidAudioManager.DIGITAL_AUDIO_MODE_AUTO &&
                        format == DroidAudioManager.DIGITAL_AUDIO_MODE_PASSTHROUGH) {
                // for passthrough -> auto
            } else {
                needSync = true;
            }
            Log.i(TAG, "needSyncDroidSetting needSync:" + needSync + ", audioMode:" +
                DroidAudioManager.digitalModeToString(format) + ", surround:" +
                DroidAudioManager.surroundModeToString(surroundMode));
            return needSync;
        }

        @Override
        public void onChange(boolean selfChange, Uri uri) {
            String option = uri.getLastPathSegment();
            if (Settings.Global.ENCODED_SURROUND_OUTPUT.equals(option)) {
                final int surroundMode = Settings.Global.getInt(mResolver,
                    Settings.Global.ENCODED_SURROUND_OUTPUT, Settings.Global.ENCODED_SURROUND_OUTPUT_AUTO);
                if (!needSyncDroidSetting(surroundMode)) {
                    return;
                }
                int digitalMode = surroundModeToDigitalMode(surroundMode);
                if (digitalMode == -1) {
                    Log.w(TAG, "onChange invalid surround mode:" + surroundMode);
                    return;
                }
                mDroidAudioManager.setDigitalAudioModeToHal(digitalMode,
                            digitalMode == DroidAudioManager.DIGITAL_AUDIO_MODE_MANUAL ? mDroidAudioManager.getAudioManualFormats() : "");
            } else if (Settings.Global.ENCODED_SURROUND_OUTPUT_ENABLED_FORMATS.equals(option)) {
                mDroidAudioManager.setDigitalAudioModeToHal(
                        DroidAudioManager.DIGITAL_AUDIO_MODE_MANUAL, mDroidAudioManager.getAudioManualFormats());
            } else if (Settings.Global.USER_PREFERRED_RESOLUTION_HEIGHT.equals(option)) {
                int resolutionHeight = Settings.Global.getInt(mResolver, Settings.Global.USER_PREFERRED_RESOLUTION_HEIGHT, 0);
                boolean preDdpEnable = mDroidAudioManager.isForceDDPEnabled();
                boolean needEnable = (resolutionHeight == 576 || resolutionHeight == 480) ? true : false;
                if (needEnable != preDdpEnable) {
                    Log.i(TAG, "onChange resolutionHeight:" + resolutionHeight + ", set ddp enable:" + needEnable);
                    mDroidAudioManager.setForceDDPEnabled(needEnable);
                }
            }
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
