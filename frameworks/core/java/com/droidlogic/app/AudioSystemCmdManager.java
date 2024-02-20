/*
 * Copyright (C) 2021 Amlogic Corporation.
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

import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.content.ServiceConnection;
import android.os.IBinder;
import android.os.RemoteException;
import android.util.Log;
import android.media.AudioManager;


public class AudioSystemCmdManager {
    private static final String TAG = "AudioSystemCmdManager";
    private Context mContext;
    private AudioManager mAudioManager;

    // These codes for compatibility with older Android versions of TvInput.
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
    private static AudioSystemCmdManager mInstance;
    public static AudioSystemCmdManager getInstance(Context context) {
        if (mInstance == null) {
            synchronized (AudioSystemCmdManager.class) {
                if (mInstance == null) {
                    mInstance = new AudioSystemCmdManager(context);
                }
            }
        }
        return mInstance;
    }

    private AudioSystemCmdManager(Context context) {
        mContext = context;
        mAudioManager = (AudioManager) mContext.getSystemService(mContext.AUDIO_SERVICE);
        Log.i(TAG, "construction AudioSystemCmdManager");
    }

    /**
     * These codes for compatibility with older Android versions of TvInput.
     */
    @Deprecated
    public void setParameters(String arg) {
        mAudioManager.setParameters(arg);
    }

    /**
     * These codes for compatibility with older Android versions of TvInput.
     */
    @Deprecated
    public String getParameters(String arg) {
        return mAudioManager.getParameters(arg);
    }

    /**
     * These codes for compatibility with older Android versions of TvInput.
     */
    @Deprecated
    public void handleAdtvAudioEvent(int cmd, int param1, int param2) {
        Log.d(TAG, "[Deprecated API] handleAdtvAudioEvent cmd:" + cmd + ", param1:" + param1 + ", param2:" + param2);
    }

    /**
     * These codes for compatibility with older Android versions of TvInput.
     */
    @Deprecated
    public void updateAudioPortGain(int sourceType) {
        Log.d(TAG, "[Deprecated API] updateAudioPortGain sourceType:" + sourceType);
    }

    /**
     * These codes for compatibility with older Android versions of TvInput.
     */
    @Deprecated
    public void openTvAudio(int sourceType) {
        Log.i(TAG, "[Deprecated API] openTvAudio set source type:" + sourceType);
        //  Same as the contents of the DroidLogicTvUtils.java
        if (sourceType == 0 /* SOURCE_TYPE_ATV */) {
            mAudioManager.setParameters("hal_param_tuner_in=atv");
        } else if (sourceType == 1 /* SOURCE_TYPE_DTV */) {
            mAudioManager.setParameters("hal_param_tuner_in=dtv");
        } else {
            Log.w(TAG, "openTvAudio unsupported source type:" + sourceType);
        }
    }

    /**
     * These codes for compatibility with older Android versions of TvInput.
     */
    @Deprecated
    public void closeTvAudio() {
        Log.d(TAG, "[Deprecated API] closeTvAudio ");
    }
}
