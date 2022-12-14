/*
 * Copyright (c) 2014 Amlogic, Inc. All rights reserved.
 *
 * This source code is subject to the terms and conditions defined in the
 * file 'LICENSE' which is part of this source code package.
 *
 * Description:
 *     AMLOGIC DroidLogicPowerService
 */

package com.droidlogic;

import android.app.Service;
import android.content.Context;
import android.content.BroadcastReceiver;
import android.content.Intent;
import android.content.IntentFilter;
import android.net.wifi.WifiManager;
import android.os.IBinder;
import android.util.Log;
import android.content.pm.PackageManager;
import android.os.SystemProperties;
import android.net.Uri;
import java.util.ArrayList;
import com.droidlogic.app.DroidLogicUtils;
import com.droidlogic.app.SystemControlManager;

import static android.content.Intent.ACTION_PACKAGE_ADDED;
import static android.content.Intent.ACTION_PACKAGE_REMOVED;

public class DroidLogicPowerService extends Service {
    private static final String TAG = "DroidLogicPowerService";
    private SystemControlManager mSystemControlManager = null;
    private boolean mWifiDisableWhenSuspend = false;
    private static final int POWER_SUSPEND_OFF = 0;
    private static final int POWER_SUSPEND_ON = 1;
    private static final int POWER_SUSPEND_SHUTDOWN = 2;

    private static final String TVTS_PKG_MEDIA_TEST = "com.google.android.medialaunch.tvts";
    private static final String TVTS_PKG_PLAYER_TEST = "com.google.android.tvts.testmediaplayer";
    private static final String TVTS_PKG_WARM_TEST = "com.google.android.leanbackjank";
    private static final int TVTS_TEST_APP_FLAG = 0;
    private static final int TVTS_TEST_APP_ENABLE = 1;
    private static final int TVTS_TEST_APP_DISABLE = 2;
    private PackageManager mPackageManager;
    private ArrayList<String> poorApps;
    private ArrayList<String> mediaApps;

    private BroadcastReceiver mReceiver = new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, Intent intent) {
            String action = intent.getAction();
            Log.d(TAG, "action: " + action);
            if (Intent.ACTION_SCREEN_ON.equals(action)) {
                setSuspendState(POWER_SUSPEND_OFF);
                setWifiState(context, true);
            } else if (Intent.ACTION_SCREEN_OFF.equals(action)) {
                setSuspendState(POWER_SUSPEND_ON);
                setWifiState(context, false);
            } else if (Intent.ACTION_SHUTDOWN.equals(action)) {
                setSuspendState(POWER_SUSPEND_SHUTDOWN);
            }
        }
    };
    private void initPoorApp() {
        poorApps = new ArrayList();
        poorApps.add("com.android.bluetooth");
        poorApps.add("com.google.android.inputmethod.latin");
	    poorApps.add("com.android.vending");
        //poorApps.add("com.google.android.gms");
    }

    private void initMediaApp() {
        mediaApps = new ArrayList();
        mediaApps.add("com.google.android.katniss");
        mediaApps.add("com.android.vending");
        mediaApps.add("com.google.android.permissioncontroller");
        mediaApps.add("com.google.android.tvrecommendations");
        mediaApps.add("com.netflix.ninja");
    }

    private void hidePoorApp() {
        for (String app : poorApps) {
            mPackageManager.setApplicationEnabledSetting(app, TVTS_TEST_APP_DISABLE, TVTS_TEST_APP_FLAG);
        }
    }

    private void unHidePoorApp() {
        for (String app : poorApps) {
            mPackageManager.setApplicationEnabledSetting(app, TVTS_TEST_APP_ENABLE, TVTS_TEST_APP_FLAG);
        }
    }

    private void hideMediaApp() {
        for (String app : mediaApps) {
            mPackageManager.setApplicationEnabledSetting(app, TVTS_TEST_APP_DISABLE, TVTS_TEST_APP_FLAG);
        }
    }

    private void unHideMediaApp() {
        for (String app : mediaApps) {
            mPackageManager.setApplicationEnabledSetting(app, TVTS_TEST_APP_ENABLE, TVTS_TEST_APP_FLAG);
        }
    }
    @Override
    public void onCreate() {
        super.onCreate();
        mSystemControlManager = SystemControlManager.getInstance();
        IntentFilter filter = new IntentFilter();
        filter.addAction(Intent.ACTION_SCREEN_OFF);
        filter.addAction(Intent.ACTION_SCREEN_ON);
        filter.addAction(Intent.ACTION_SHUTDOWN);
        registerReceiver (mReceiver, filter);
        Context mContext = this.getApplicationContext();
        mSystemControlManager = SystemControlManager.getInstance();
        final IntentFilter packageFilter = new IntentFilter();
        packageFilter.addAction(ACTION_PACKAGE_ADDED);
        packageFilter.addAction(ACTION_PACKAGE_REMOVED);
        packageFilter.addDataScheme("package");
        mContext.registerReceiver(new PackageReceiver(), packageFilter);
        mPackageManager = mContext.getPackageManager();
        initPoorApp();
        initMediaApp();
    }

    @Override
    public int onStartCommand(Intent intent, int flags, int startId) {
        return super.onStartCommand(intent, flags, startId);
    }

    @Override
    public void onDestroy() {
        super.onDestroy();
        unregisterReceiver(mReceiver);
    }

    @Override
    public IBinder onBind(Intent intent) {
        return null;
    }

    private void setWifiState(Context context, boolean state) {
        if (mSystemControlManager.getPropertyBoolean("ro.vendor.platform.wifi.suspend", false) == false) {
            return;
        }

        WifiManager wm = (WifiManager) context.getSystemService(Context.WIFI_SERVICE);

        if (state) {
            if (mWifiDisableWhenSuspend == true) {
                try {
                    wm.setWifiEnabled(true);
                    mWifiDisableWhenSuspend = false;
                } catch (Exception e) {
                    /* ignore - local call */
                }
            }
        } else {
            int wifiState = wm.getWifiState();
            if (wifiState == WifiManager.WIFI_STATE_ENABLING
                    || wifiState == WifiManager.WIFI_STATE_ENABLED) {
                try {
                    wm.setWifiEnabled(false);
                    mWifiDisableWhenSuspend = true;
                } catch (Exception e) {
                    /* ignore - local call */
                }
                try {
                    Thread.sleep(2300);
                } catch (InterruptedException ignore) {
                }
            }
        }

        Log.d(TAG, "setWifiState: " + state);
    }

    private static final String KILL_ESM_PATH = "/sys/module/tvin_hdmirx/parameters/hdcp22_kill_esm";
    private static final String KILL_ESM_PATH_54 = "sys/module/aml_media/parameters/hdcp22_kill_esm";
    private static final String VIDEO_GLOBAL_OUTPUT_PATH = "/sys/class/video/video_global_output";

    private void setSuspendState(int state) {
        if (!DroidLogicUtils.isTv()) {
            return;
        }

        if (state == POWER_SUSPEND_SHUTDOWN) {
            mSystemControlManager.writeSysFs(VIDEO_GLOBAL_OUTPUT_PATH, "0");
            mSystemControlManager.writeSysFs(KILL_ESM_PATH, "1");
            mSystemControlManager.writeSysFs(KILL_ESM_PATH_54, "1");
        }

        if (state == POWER_SUSPEND_ON) {
            mSystemControlManager.setBootenv("ubootenv.var.suspend", "on");
        } else if (state == POWER_SUSPEND_OFF) {
            mSystemControlManager.setBootenv("ubootenv.var.suspend", "off");
        } else if (state == POWER_SUSPEND_SHUTDOWN) {
            mSystemControlManager.setBootenv("ubootenv.var.suspend", "shutdown");
        }

        Log.d(TAG, "setSuspendState: " + state);
    }
	private class PackageReceiver extends BroadcastReceiver {

        @Override
        public void onReceive(Context context, Intent intent) {

            final Uri data = intent.getData();
            if (data == null) {
                Log.e(TAG, "Cannot handle package broadcast with null data");
                return;
            }
            //Mode mCurrentMode = mDisplayManager.getDisplay(0).getMode();
            //Log.d(TAG, "mCurrentMode: " + mCurrentMode);
            final String packageName = data.getSchemeSpecificPart();
            switch (intent.getAction()) {
                case ACTION_PACKAGE_ADDED:
                    if (packageName.equals(TVTS_PKG_PLAYER_TEST)) {
                        Log.d(TAG, "ACTION_PACKAGE_ADDED packageName:" + packageName);
                        performanceOptimization(true);
                        setPropTvts(true);
                    }
                    break;
                case ACTION_PACKAGE_REMOVED:
                    if (packageName.equals(TVTS_PKG_PLAYER_TEST)) {
                        Log.d(TAG, "ACTION_PACKAGE_REMOVED packageName:" + packageName);
                        performanceOptimization(false);
                        setPropTvts(false);
                    }
                    break;
                default:
                    // do nothing
                    break;
            }
        }

        private void setPropTvts(boolean status) {
            if (mSystemControlManager != null) {
                if (status) {
                    mSystemControlManager.setProperty("vendor.media.omx.dec.dmc.level", "4");
                    mSystemControlManager.setProperty("vendor.media.omx.dw", "0");
                    SystemProperties.set("sys.tvts.running", "2");
                    mPackageManager.setApplicationEnabledSetting("com.google.android.youtube.tv", TVTS_TEST_APP_DISABLE, TVTS_TEST_APP_FLAG);
                    hidePoorApp();
                } else {
                    mSystemControlManager.setProperty("vendor.media.omx.dec.dmc.level", "");
                    mSystemControlManager.setProperty("vendor.media.omx.dw", "");
                    SystemProperties.set("sys.tvts.running", "0");
                    mPackageManager.setApplicationEnabledSetting("com.google.android.youtube.tv", TVTS_TEST_APP_ENABLE, TVTS_TEST_APP_FLAG);
                    unHidePoorApp();
                }
            }
        }

        private void setPropMediaTvts(boolean status) {
            if (mSystemControlManager != null) {
                if (status) {
                    mSystemControlManager.setProperty("vendor.media.omx.dec.dmc.level", "4");
                    mSystemControlManager.setProperty("vendor.media.omx.dw", "0");
                    SystemProperties.set("sys.tvts.running", "1");
                } else {
                    mSystemControlManager.setProperty("vendor.media.omx.dec.dmc.level", "");
                    mSystemControlManager.setProperty("vendor.media.omx.dw", "");
                    SystemProperties.set("sys.tvts.running", "0");
                }
            }
        }

        private void performanceOptimization(boolean status) {
            if (mSystemControlManager != null) {
                if (status) {
                    mSystemControlManager.writeSysFs("/sys/class/thermal/thermal_zone0/mode", "disabled");
                    mSystemControlManager.writeSysFs("/sys/devices/system/cpu/cpu0/cpufreq/scaling_governor", "performance");
                    mSystemControlManager.writeSysFs("/proc/sys/kernel/printk", "0");
                    mSystemControlManager.writeSysFs("/sys/class/mpgpu/scale_mode", "3");
                   // mSystemControlManager.writeSysFs("/sys/module/amvideo/parameters/force_vskip_cnt", "1000");
                    mSystemControlManager.writeSysFs("/sys/module/di/parameters/bypass_all", "1");
                } else {
                    mSystemControlManager.writeSysFs("/sys/class/thermal/thermal_zone0/mode", "enable");
                    mSystemControlManager.writeSysFs("/sys/devices/system/cpu/cpu0/cpufreq/scaling_governor", "interactive");
                    mSystemControlManager.writeSysFs("/proc/sys/kernel/printk", "4");
                    mSystemControlManager.writeSysFs("/sys/class/mpgpu/scale_mode", "1");
                    //mSystemControlManager.writeSysFs("/sys/module/amvideo/parameters/force_vskip_cnt", "0");
                    mSystemControlManager.writeSysFs("/sys/module/di/parameters/bypass_all", "0");

                }
            }
        }
    }
}
