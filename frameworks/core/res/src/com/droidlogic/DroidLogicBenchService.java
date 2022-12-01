package com.droidlogic;

import android.app.Service;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.pm.PackageManager;
import android.net.Uri;
import android.os.IBinder;
import android.util.Log;
import com.droidlogic.app.SystemControlManager;
import java.util.ArrayList;

public class DroidLogicBenchService extends Service {
    private static final String TAG = "DroidLogicBenchService";
    private static final int BENCH_TEST_APP_FLAG = 0;
    private static final int BENCH_TEST_APP_ENABLE = 1;
    private static final int BENCH_TEST_APP_DISABLE = 2;
    private static final String ACTION_LAUNCH_BENCH_APP = "com.amlogic.ACTION_LAUNCH_BENCH_APP";
    private Context mContext;
    private SystemControlManager mSCM;
    private ArrayList<String> benchApps;
    private void initPoorApp() {
        benchApps = new ArrayList();
        benchApps.add("com.android.bluetooth");
        benchApps.add("com.google.android.inputmethod.latin");
        benchApps.add("com.google.android.tvrecommendations");
        benchApps.add("com.google.android.katniss");
        benchApps.add("com.android.vending");
    }

    @Override
    public void onCreate() {
        super.onCreate();
        Log.d(TAG, "DroidLogicBenchService is oncreate");
        mContext = this;
        mSCM = SystemControlManager.getInstance();
        PackageManager mPackageManager = mContext.getPackageManager();
        final IntentFilter packageFilter = new IntentFilter();
        packageFilter.addAction(ACTION_LAUNCH_BENCH_APP);
        mContext.registerReceiver(new PackageReceiver(), packageFilter);
        initPoorApp();
    }

    @Override
    public IBinder onBind(Intent intent) {
        return null;
    }

    private void hidePoorApp() {
        PackageManager packageManager = mContext.getPackageManager();
        for (String app : benchApps) {
            packageManager.setApplicationEnabledSetting(app, BENCH_TEST_APP_DISABLE, BENCH_TEST_APP_FLAG);
        }
    }

    private void unHidePoorApp() {
        PackageManager packageManager = mContext.getPackageManager();
        for (String app : benchApps) {
            packageManager.setApplicationEnabledSetting(app, BENCH_TEST_APP_ENABLE, BENCH_TEST_APP_FLAG);
        }
    }

    private class PackageReceiver extends BroadcastReceiver {

        @Override
        public void onReceive(Context context, Intent intent) {
            if (ACTION_LAUNCH_BENCH_APP.equals(intent.getAction())) {
                Log.d(TAG, "action is com.amlogic.ACTION_LAUNCH_BENCH_APP");
                if (intent.getStringExtra(ACTION_LAUNCH_BENCH_APP).equals("true")) {
                    Log.d(TAG, "bench app is onForeground");
                    hidePoorApp();
                    performanceOptimization(true);
                } else if (intent.getStringExtra(ACTION_LAUNCH_BENCH_APP).equals("false")) {
                    Log.d(TAG, "bench app is not onForeground and set default param");
                    unHidePoorApp();
                    performanceOptimization(false);
                }
            }
        }

        private void performanceOptimization(boolean status) {
            if (mSCM != null) {
                if (status) {
                    mSCM.writeSysFs("/sys/class/thermal/thermal_zone0/mode", "disabled");
                    mSCM.writeSysFs("/sys/devices/system/cpu/cpu0/cpufreq/scaling_governor", "performance");
                    mSCM.writeSysFs("/proc/sys/kernel/printk", "0");
                    mSCM.writeSysFs("/sys/class/mpgpu/scale_mode", "3");
                } else {
                    mSCM.writeSysFs("/sys/class/thermal/thermal_zone0/mode", "enable");
                    mSCM.writeSysFs("/sys/devices/system/cpu/cpu0/cpufreq/scaling_governor", "interactive");
                    mSCM.writeSysFs("/proc/sys/kernel/printk", "4");
                    mSCM.writeSysFs("/sys/class/mpgpu/scale_mode", "1");
                }
            }
        }
    }
}
