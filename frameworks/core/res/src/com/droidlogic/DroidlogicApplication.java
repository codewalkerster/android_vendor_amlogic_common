/*
 * Copyright (c) 2014 Amlogic, Inc. All rights reserved.
 *
 * This source code is subject to the terms and conditions defined in the
 * file 'LICENSE' which is part of this source code package.
 *
 * Description:
 *     AMLOGIC UsbCameraReceiver
 */

package com.droidlogic;

import android.app.Application;
import android.content.ComponentName;
import android.content.Intent;
import android.content.ContentProviderClient;
import android.content.Context;

import android.media.tv.TvContract;
import android.os.Handler;
import android.os.Message;
import android.os.SystemProperties;
import android.os.PowerManager;
import android.text.TextUtils;
import android.util.Log;
import android.provider.Settings;
import android.text.TextUtils;
import com.droidlogic.app.DroidAudioCore;
import com.droidlogic.app.DroidLogicUtils;
import com.droidlogic.app.OutputModeManager;
import com.droidlogic.app.SystemControlEvent;
import com.droidlogic.app.SystemControlManager;
import android.content.pm.PackageManager;
import com.droidlogic.btpair.BluetoothAutoPairReceiver;
import android.os.Handler;
import android.os.IBinder;
import android.os.Looper;
import android.os.RemoteException;
import android.os.ServiceManager;


public class DroidlogicApplication extends Application {
    private static final String TAG = "DroidlogicApplication";
    private SystemControlEvent mSystemControlEvent;
    private SystemControlManager mSystemControlManager;
    private PowerManager.WakeLock mWakeLock;
    DroidAudioCore mDroidAudioCore;

    @Override
    public void onCreate() {
        super.onCreate();
        Log.d(TAG, "onCreate");
        mDroidAudioCore = DroidAudioCore.getInstance(this);
        mHandler.sendEmptyMessage(MSG_CHECK_BOOTVIDEO_FINISHED);
        // Should not do in java
        //register system control callback
        mSystemControlEvent   = SystemControlEvent.getInstance(this);
        mSystemControlManager = SystemControlManager.getInstance();
        mSystemControlManager.setListener(mSystemControlEvent);
        mSystemControlManager.setProperty("vendor.sys.display.boot_complete","1");
        // GTVS version default use earlysuspend wakelock
        if (isGtvsVersion() && SystemProperties.getBoolean("ro.vendor.platform.earlysuspend", true)) {
            PowerManager powerManager = (PowerManager) getSystemService(POWER_SERVICE);
            mWakeLock = powerManager.newWakeLock(PowerManager.PARTIAL_WAKE_LOCK,
                "EarlysuspendTag[ro.vendor.platform.earlysuspend]:"+this);
            mWakeLock.acquire();
            Log.d(TAG, "wakelocked");
        }
        DisableBtPairInstrumentation(this);
    }

    private boolean isGtvsVersion() {
        return !TextUtils.isEmpty(SystemProperties.get("ro.com.google.gmsversion", ""));
    }

    private boolean isBootvideoStopped() {
        ContentProviderClient tvProvider = null;
        return (((SystemProperties.getInt("persist.vendor.media.bootvideo", 50)  > 100)
                        && TextUtils.equals(SystemProperties.get("service.bootvideo.exit", "1"), "0"))
                || ((SystemProperties.getInt("persist.vendor.media.bootvideo", 50)  <= 100)));
    }

    private static final int MSG_CHECK_BOOTVIDEO_FINISHED = 0;
    private Handler mHandler = new Handler() {
        public void handleMessage(Message msg) {
            switch (msg.what) {
                case MSG_CHECK_BOOTVIDEO_FINISHED:
                    if (isBootvideoStopped()) {
                        Log.d(TAG, "bootvideo stopped, start initializing AudioEffect");
                        Intent intent = new Intent();
                        intent.setComponent(new ComponentName("com.droidlogic", "com.droidlogic.audioservice.services.AudioEffectsService"));
                        intent.setAction("com.droidlogic.audioservice.services.AudioEffectsService.STARTUP");
                        startService(intent);
                    } else {
                        if (DroidLogicUtils.getAudioDebugEnable()) {
                            Log.d(TAG, "handleMessage sendEmptyMessageDelayed MSG_CHECK_BOOTVIDEO_FINISHED");
                        }
                        mHandler.sendEmptyMessageDelayed(MSG_CHECK_BOOTVIDEO_FINISHED, 10);
                    }
                    break;
                default:
                    break;
            }
        }
    };

    private void DisableBtPairInstrumentation(Context context) {
        if (SystemProperties.get("sys.vendor.remote.type", "IR_NONE").contains("BT"))
            return;
        PackageManager pm = context.getPackageManager();
        ComponentName name = new ComponentName(context, BluetoothAutoPairReceiver.class);
        Log.i(TAG, "set BluetoothAutoPairReceiver disabled");
        pm.setComponentEnabledSetting(name, PackageManager.COMPONENT_ENABLED_STATE_DISABLED,
        PackageManager.DONT_KILL_APP);
    }
}

