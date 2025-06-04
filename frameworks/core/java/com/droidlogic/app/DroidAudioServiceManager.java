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

import android.content.Context;
import android.content.Intent;
import android.content.ServiceConnection;
import android.os.Handler;
import android.os.IBinder;
import android.os.Looper;
import android.os.RemoteException;
import android.util.Log;

import vendor.amlogic.hardware.droidaudio.IDroidAudio;
import vendor.amlogic.hardware.droidaudio.IDroidAudioClient;

public class DroidAudioServiceManager {
    private static String TAG = "DroidAudioServiceManager";
    private static IDroidAudio mDroidAudioService;
    private static IDroidAudioClient mDroidAudioServiceClient = new DroidAudioServiceClient();
    private Context mContext;

    private static DroidAudioServiceManager mInstance;
    public static DroidAudioServiceManager getInstance(Context context) {
        if (mInstance == null) {
            synchronized (DroidAudioServiceManager.class) {
                if (mInstance == null) {
                    mInstance = new DroidAudioServiceManager(context);
                }
            }
        }
        return mInstance;
    }

    private DroidAudioServiceManager(Context context) {
        Log.i(TAG, "construction DroidAudioServiceManager");
        mContext = context;
    }

    private static class DroidAudioServiceClient extends IDroidAudioClient.Stub {
        @Override
        public void onDroidAudioEvent(int event, int[] data) {
            // TODO:
            Log.i(TAG, "onDroidAudioEvent event:" + event + ", data:" + data);
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
                Log.e(TAG, "get IDroidAudio Service fail:" + ex);
                Log.w(TAG, Log.getStackTraceString(new Throwable()));
                return null;
            }
            if (mDroidAudioService == null) {
                Log.w(TAG, "getService get IDroidAudio service failed.");
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
}
