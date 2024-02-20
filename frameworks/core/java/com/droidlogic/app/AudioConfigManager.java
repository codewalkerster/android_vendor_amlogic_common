/*
 * Copyright (C) 2009 The Android Open Source Project
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

import android.content.Context;

import com.droidlogic.app.DroidAudioManager;

public class AudioConfigManager {
    private static final String TAG                         = "AudioConfigManager";
    private Context mContext;
    private static AudioConfigManager mAudioConfigManager = null;

    public static final int AUDIO_OUTPUT_DELAY_SOURCE_ATV               = DroidAudioManager.AUDIO_OUTPUT_DELAY_SOURCE_ATV;
    public static final int AUDIO_OUTPUT_DELAY_SOURCE_DTV               = DroidAudioManager.AUDIO_OUTPUT_DELAY_SOURCE_DTV;
    public static final int AUDIO_OUTPUT_DELAY_SOURCE_AV                = DroidAudioManager.AUDIO_OUTPUT_DELAY_SOURCE_AV;
    public static final int AUDIO_OUTPUT_DELAY_SOURCE_HDMI              = DroidAudioManager.AUDIO_OUTPUT_DELAY_SOURCE_HDMI;
    public static final int AUDIO_OUTPUT_DELAY_SOURCE_MEDIA             = DroidAudioManager.AUDIO_OUTPUT_DELAY_SOURCE_MEDIA;

    public static AudioConfigManager getInstance(Context context) {
        synchronized (AudioConfigManager.class) {
            if (mAudioConfigManager == null) {
                mAudioConfigManager = new AudioConfigManager(context);
            }
        }
        return mAudioConfigManager;
    }

    private AudioConfigManager(Context context) {
        mContext = context;
    }

    /**
     * These codes for compatibility with older Android versions of TvInput.
     */
    @Deprecated
    public void refreshAudioCfgBySrc(int source) {
        DroidAudioManager.getInstance(mContext).refreshAudioCfgBySrc(source, false);
    }
}

