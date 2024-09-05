package com.droidlogic;

import android.app.ActivityManager;
import android.app.IActivityManager;
import android.app.IProcessObserver;
import android.app.Service;
import android.content.Context;
import android.content.Intent;
import android.content.pm.PackageManager;
import android.os.IBinder;
import android.os.RemoteException;
import android.util.Log;

import com.droidlogic.app.SystemControlManager;

import java.util.ArrayList;
import java.util.List;
import java.util.Map;
import java.util.HashMap;
import java.util.Arrays;

public class DroidLogicBenchService extends Service {
    private static final String TAG = "DroidLogicBenchService";
    private static final int BENCH_TEST_APP_FLAG = 0;
    private static final int BENCH_TEST_APP_ENABLE = 1;
    private static final int BENCH_TEST_APP_DISABLE = 2;
    private Context mContext;
    private SystemControlManager mSCM;
    private IActivityManager mIActivityManager;
    private ProcessObserver mProcessObserver;
    private boolean bHasChangeToPerformance = false;
    private final Object mLock = new Object();
    private Map<String, String> CpusetmapDefault = new HashMap<>();
    private ArrayList<String> benchApps = new ArrayList<>(Arrays.asList(
        "com.google.android.inputmethod.latin",
        "com.google.android.katniss",
        "com.android.vending",
        "com.google.android.tvrecommendations",
        "com.google.android.tts"
        ));
    private ArrayList<String> benchmarkApps = new ArrayList<>(Arrays.asList(
        "com.primatelabs.geekbench",
        "net.kishonti.gfxbench",
        "com.futuremark.pcmark.android.benchmark",
        "com.antutu.ABenchMark",
        "com.antutu.benchmark.full",
        "com.antutu.benchmark.full:unity",
        "com.antutu.benchmark.full:refinery",
        "com.rightware.BasemarkOSIICN",
        "com.glbenchmark.glbenchmark27"
        ));

    private Map<String, String> CpusetmapPerformance = new HashMap<String, String>() {{
        put("/sys/class/thermal/thermal_zone0/mode", "disabled");
        put("/sys/devices/system/cpu/cpu0/cpufreq/scaling_governor", "performance");
        put("/sys/class/devfreq/fe400000.valhall/governor", "performance");
        put("/sys/class/mpgpu/scale_mode", "3");
        put("/dev/cpuset/background/cpus", "1");
        put("/dev/cpuset/system-background/cpus", "1");
        put("/dev/cpuset/restricted/cpus", "1");
        put("/sys/kernel/mm/lru_gen/enabled", "0");
        }};

    @Override
    public void onCreate() {
        super.onCreate();
        Log.d(TAG, "DroidLogicBenchService is oncreate");
        mContext = this;
        mSCM = SystemControlManager.getInstance();
        mProcessObserver = new ProcessObserver();
        mIActivityManager = ActivityManager.getService();
        try {
            mIActivityManager.registerProcessObserver(mProcessObserver);
        } catch (RemoteException e) {
            Log.e(TAG, "could not get IActivityManager");
        }
    }

    @Override
    public IBinder onBind(Intent intent) {
        return null;
    }

    private void getCpusets() {
        for (String key : CpusetmapPerformance.keySet()) {
            CpusetmapDefault.put(key, mSCM.readSysFsOri(key));
            Log.d(TAG, "Get " + key + " is " + CpusetmapDefault.get(key));
        }
    }

    private void enablePoorApp(int enableValue) {
        PackageManager packageManager = mContext.getPackageManager();
        for (String app : benchApps) {
            try {
                packageManager.getPackageInfo(app, PackageManager.GET_ACTIVITIES);
                packageManager.setApplicationEnabledSetting(app, enableValue, BENCH_TEST_APP_FLAG);
            } catch (Exception e) {
                Log.w(TAG, app + " is not found");
            }

        }
    }

    @Override
    public void onDestroy() {
        try {
            mIActivityManager.unregisterProcessObserver(mProcessObserver);
        } catch (RemoteException e) {
            Log.e(TAG, "Failed to unregister listeners", e);
        }
        super.onDestroy();
    }

   public boolean isBenchApp(int nPid) {
        ActivityManager activityManager = (ActivityManager) mContext.getSystemService(Context.ACTIVITY_SERVICE);
        List<ActivityManager.RunningAppProcessInfo> runningProcesses = activityManager.getRunningAppProcesses();
        for (ActivityManager.RunningAppProcessInfo processInfo : runningProcesses) {
            for (String app : benchmarkApps) {
                if (processInfo.processName.equals(app)) {
                    if (nPid == processInfo.pid) {
                        return true;
                    }
                }
            }
        }
        return false;
    }

    public boolean isBenchAppForeGround() {
        ActivityManager activityManager = (ActivityManager) mContext.getSystemService(Context.ACTIVITY_SERVICE);
        List<ActivityManager.RunningAppProcessInfo> runningProcesses = activityManager.getRunningAppProcesses();
        for (ActivityManager.RunningAppProcessInfo processInfo : runningProcesses) {
            for (String app : benchmarkApps) {
                if (processInfo.processName.equals(app)) {
                    if (processInfo.importance == ActivityManager.RunningAppProcessInfo.IMPORTANCE_FOREGROUND) {
                        return true;
                    } else {
                        return isTopActivity();
                    }
                }
            }
        }
        return false;
    }

    private boolean isTopActivity() {
        ActivityManager am = (ActivityManager) mContext.getSystemService(Context.ACTIVITY_SERVICE);
        List<ActivityManager.RunningTaskInfo> infos = am.getRunningTasks(1);
        for (ActivityManager.RunningTaskInfo componentInfo : infos) {
            for (String app : benchmarkApps) {
                if (componentInfo.topActivity.getPackageName().equals(app)) {
                    Log.d(TAG, app + " is top activity!");
                    return true;

                } else {
                    Log.d(TAG, app + "is not top activity.");
                    return false;

                }
            }
        }
        return false;
    }

    private class ProcessObserver extends IProcessObserver.Stub {
        @Override
        public void onForegroundActivitiesChanged(int pid, int uid, boolean foregroundActivities) {
            Log.d(TAG, "onForegroundActivitiesChanged pid:" + pid + ",uid:" + uid + ",fg:" + foregroundActivities);
            new Thread(() -> {
                try {
                    //wait 500ms, for android update process stack
                    Thread.sleep(500);
                } catch (InterruptedException e) {
                    e.printStackTrace();
                }
                fGStateUpdate(pid,foregroundActivities);
            }).start();
        }

        private void fGStateUpdate(int pid, boolean foreground) {
            synchronized (mLock) {
                if (!isBenchApp(pid)) {
                    Log.d(TAG, "not bench app, ignore it");
                    return;
                }

                if (isBenchAppForeGround()) {
                    Log.d(TAG, "bench app is onForeground");
                    if (!bHasChangeToPerformance) {
                        getCpusets();
                    }
                    enablePoorApp(BENCH_TEST_APP_DISABLE);
                    performanceOptimization(true);
                    bHasChangeToPerformance = true;
                } else {
                    Log.d(TAG, "bench app is onBackground");
                    enablePoorApp(BENCH_TEST_APP_ENABLE);
                    if (bHasChangeToPerformance) {
                        performanceOptimization(false);
                        bHasChangeToPerformance = false;
                    }
                }
            }
        }

        @Override
        public void onForegroundServicesChanged(int pid, int uid, int fgServiceTypes) {
            Log.d(TAG, "onForegroundServicesChanged pid:" + pid);
        }

        @Override
        public void onProcessDied(int pid, int uid) {
        }
    }

    private void performanceOptimization(boolean status) {
        if (mSCM != null) {
            if (status) {
                for (String key : CpusetmapPerformance.keySet()) {
                    mSCM.writeSysFs(key,  CpusetmapPerformance.get(key));
                    Log.d(TAG, "Set " + key + " is " + CpusetmapPerformance.get(key));
                }
            } else {
                for (String key : CpusetmapDefault.keySet()) {
                    mSCM.writeSysFs(key,  CpusetmapDefault.get(key));
                    Log.d(TAG, "Set " + key + " is " + CpusetmapDefault.get(key));
                }
            }
        }
    }

}
