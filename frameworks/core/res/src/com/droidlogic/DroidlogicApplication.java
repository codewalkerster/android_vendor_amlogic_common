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
import android.hardware.display.DisplayManager;
import android.media.tv.TvContract;
import android.os.Build;
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

import java.lang.reflect.Field;
import java.lang.reflect.InvocationTargetException;
import java.lang.reflect.Method;

public class DroidlogicApplication extends Application {
    private static final String TAG = "DroidlogicApplication";
    private SystemControlEvent mSystemControlEvent;
    private SystemControlManager mSystemControlManager;
    private PowerManager.WakeLock mWakeLock;
    DroidAudioCore mDroidAudioCore;
    private String mTop;
    private String mForeground;
    private String mBackground;
    private String mSystem;
    private String mRestricted;
    private Context mContext;

    @Override
    public void onCreate() {
        super.onCreate();
        Log.d(TAG, "onCreate");
        mDroidAudioCore = DroidAudioCore.getInstance(this);
        // Should not do in java
        //register system control callback
        mSystemControlEvent   = SystemControlEvent.getInstance(this);
        mSystemControlManager = SystemControlManager.getInstance();
        mSystemControlManager.setListener(mSystemControlEvent);
        mSystemControlManager.setProperty("vendor.sys.display.boot_complete","1");
        checkAndResetOverride();
        // GTVS version default use earlysuspend wakelock
        if (isGtvsVersion() && SystemProperties.getBoolean("ro.vendor.platform.earlysuspend", true)) {
            PowerManager powerManager = (PowerManager) getSystemService(POWER_SERVICE);
            mWakeLock = powerManager.newWakeLock(PowerManager.PARTIAL_WAKE_LOCK,
                "EarlysuspendTag[ro.vendor.platform.earlysuspend]:"+this);
            mWakeLock.acquire();
            Log.d(TAG, "wakelocked");
        }
        DisableBtPairInstrumentation(this);
        initCpusets();
        setCpusetsDefault();
        mContext = this;
    }

    private void initCpusets() {
        mTop = mSystemControlManager.readSysFsOri("/dev/cpuset/top-app/cpus");
        Log.d(TAG, "mTop cpus is  " + mTop);
        mForeground = mSystemControlManager.readSysFsOri("/dev/cpuset/foreground/cpus");
        Log.d(TAG, "mForeground cpus is  " + mForeground);
        mBackground = mSystemControlManager.readSysFsOri("/dev/cpuset/background/cpus");
        Log.d(TAG, "background cpus is  " + mBackground);
        mSystem = mSystemControlManager.readSysFsOri("/dev/cpuset/system-background/cpus");
        Log.d(TAG, "system background cpus is  " + mSystem);
        mRestricted = mSystemControlManager.readSysFsOri("/dev/cpuset/restricted/cpus");
        Log.d(TAG, "restricted cpus is  " + mRestricted);
    }

    private void setCpusetsDefault() {
        new Thread() {
            @Override
            public void run() {
                try {
                    Thread.sleep(1000*60*1);
                    mSystemControlManager.writeSysFs("/dev/cpuset/top-app/cpus", mTop);
                    mSystemControlManager.writeSysFs("/dev/cpuset/foreground/cpus", mForeground);
                    mSystemControlManager.writeSysFs("/dev/cpuset/background/cpus", mBackground);
                    mSystemControlManager.writeSysFs("/dev/cpuset/system-background/cpus", mSystem);
                    mSystemControlManager.writeSysFs("/dev/cpuset/restricted/cpus", mRestricted);
                    Log.d(TAG, "end setCpusetsDefault");
                } catch (InterruptedException e) {
                    e.printStackTrace();
                }
            }
        }.start();

    }

    private void checkAndResetOverride() {
        Log.d(TAG,"Build version "+Build.VERSION.SDK_INT+"///"+SystemControlManager.getInstance().getPropertyInt("persist.vendor.sys.sdk_int", 0));
        if (Build.VERSION.SDK_INT == SystemControlManager.getInstance().getPropertyInt("persist.vendor.sys.sdk_int", 0)) {
            return;
        }else {
            Context context = getApplicationContext();
            DisplayManager displaymanager = context.getSystemService(DisplayManager.class);
            Log.d(TAG,"DisplayManager "+displaymanager.getGlobalUserPreferredDisplayMode());
            int density = -1;
            String sizeForced = "";
            try {
                density = Settings.Secure.getInt(context.getContentResolver(),"display_density_forced");
                sizeForced = Settings.Global.getString(context.getContentResolver(),"display_size_forced");
                Log.d(TAG,"density "+density+" sizeForced--"+sizeForced);
            }catch (Settings.SettingNotFoundException e) {
            }finally {
                SystemControlManager.getInstance().setProperty("persist.vendor.sys.sdk_int",""+Build.VERSION.SDK_INT);
                try {
                    Log.d(TAG,"density "+density+" sizeForced"+sizeForced+"sizeForced.isEmpty()"+(sizeForced == null || sizeForced.isEmpty()));
                    if (density != -1 && (sizeForced == null || sizeForced.isEmpty())) {
                        Class globalclass = Class.forName("android.view.WindowManagerGlobal");
                        Method getWmServiceMethod = globalclass.getDeclaredMethod("getWindowManagerService");
                        getWmServiceMethod.setAccessible(true);
                        Log.d(TAG,"clear density ");
                        Object iWindowManager = getWmServiceMethod.invoke(null);
                        Method clearForcedDisplayDensityForUser = iWindowManager.getClass().getMethod("clearForcedDisplayDensityForUser", int.class,  int.class);
                        clearForcedDisplayDensityForUser.invoke(iWindowManager, 0, 0);
                    }
                }catch(Exception  ex){
                    ex.printStackTrace();
                }
            }
        }
    }

    private boolean isGtvsVersion() {
        return !TextUtils.isEmpty(SystemProperties.get("ro.com.google.gmsversion", ""));
    }

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

