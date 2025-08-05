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

import android.os.RemoteException;
import android.util.Log;
import com.droidlogic.app.DroidAudioServiceManager;
import com.droidlogic.app.SystemControlManager;

import vendor.amlogic.hardware.droidaudio.IDroidAudio;
import vendor.amlogic.hardware.droidaudio.IDroidAudioClient;

public class MpeghNativeManager extends IDroidAudioClient.Stub {

    public static final int SYSTEM_CMD_RESET = 0;                          // reset
    public static final int SYSTEM_CMD_DRC_SELECTED = 10;                  // mpegh_drc_effect
    public static final int SYSTEM_CMD_DRC_BOOST = 11;                     // mpegh_drc_boost
    public static final int SYSTEM_CMD_DRC_COMPRESS = 12;                  // mpegh_drc_att
    public static final int SYSTEM_CMD_TARGET_LOUDNESS = 20;               // mpegh_tl
    public static final int SYSTEM_CMD_ALBUM_MODE = 21;                    // mpegh_drc_album
    public static final int SYSTEM_CMD_ACCESSIBILITY_PREFERENCE = 31;      // mpegh_accessibility
    public static final int SYSTEM_CMD_AUDIO_LANGUAGE_SELECTED = 70;       // mpegh_audio_lang
    public static final int SYSTEM_CMD_INTERFACE_LANGUAGE_SELECTED = 71;   // mpegh_label_lang

    public interface AudioSceneUpdateListener {
        void onAudioSceneUpdated(String xml);
    }

    private static final String TAG = "MpeghNativeManager.J";
    private static MpeghNativeManager mInstance;
    private static IDroidAudio mDroidAudioService;
    private AudioSceneUpdateListener mListener;
    public static synchronized MpeghNativeManager getInstance() {
        if (mInstance == null) {
            mInstance = new MpeghNativeManager();
            mInstance.registerClient(mInstance);
        }
        return mInstance;
    }

    private MpeghNativeManager() {
        Log.i(TAG, "construction MpeghNativeManager");
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

    @Override
    public void onDroidAudioEvent(int event, int[] data) {
        //Log.i(TAG, "onDroidAudioEvent event:" + event + ", data:" + data);
        return;
    }
    @Override
    public void onMpeghAsiEvent(int event, int[] data) {
        Log.i(TAG, "onMpeghAsiEvent event:" + event + ", data:" + data);
        String xml = getXmlSceneInfo();
        if (mListener != null) {
            mListener.onAudioSceneUpdated(xml);
        }
        return;
    }
    @Override
    public String getInterfaceHash() {
        return IDroidAudioClient.HASH;
    }
    @Override
    public int getInterfaceVersion() {
        return IDroidAudioClient.VERSION;
    }

    private int registerClient(IDroidAudioClient client) {
        return executeRemoteCall(() -> mDroidAudioService.registerClient(client), "registerClient", 0);
    }

    public int setActionEvent(String eventXml) {
        return executeRemoteCall(() -> mDroidAudioService.MpeghManager_setActionEvent(eventXml), "setActionEvent", 0);
    }

    public int setSystemConfig(int id, String value) {
        return executeRemoteCall(() -> mDroidAudioService.MpeghManager_setSystemConfig(id, value), "setSystemConfig", 0);
    }

    public String getSystemConfig(int id) {
        return executeRemoteCall(() -> mDroidAudioService.MpeghManager_getSystemConfig(id), "getSystemConfig", "0");
    }

    public String getXmlSceneInfo() {
        return executeRemoteCall(() -> mDroidAudioService.MpeghManager_getXmlSceneInfo(), "getXmlSceneInfo", "");
    }

    public void setAudioSceneUpdateListener(AudioSceneUpdateListener listener) {
        this.mListener = listener;
    }

}
