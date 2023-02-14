package com.droidlogic;

import android.app.Service;
import android.content.Intent;
import android.os.IBinder;
import android.util.Log;

import com.droidlogic.app.SystemControlManager;

public class DroidLogicCpuService extends Service {
    private static final String TAG = "DroidLogicCpuService";
    private SystemControlManager mSystemControlManager = null;

    private void setCpusetsDefault() {
        new Thread() {
            @Override
            public void run() {
                try {
                    Thread.sleep(1000*60*3);
                    mSystemControlManager.writeSysFs("/dev/cpuset/top-app/cpus", "0-3");
                    mSystemControlManager.writeSysFs("/dev/cpuset/foreground/cpus", "0-3");
                    mSystemControlManager.writeSysFs("/dev/cpuset/background/cpus", "0-3");
                    mSystemControlManager.writeSysFs("/dev/cpuset/system-background/cpus", "0-3");
                    mSystemControlManager.writeSysFs("/dev/cpuset/restricted/cpus", "0-3");
                    Log.d(TAG, "end setCpusetsDefault: 0-3");
                } catch (InterruptedException e) {
                    e.printStackTrace();
                }
            }
        }.start();

    }

    @Override
    public IBinder onBind(Intent intent) {
        return null;
    }

    @Override
    public void onCreate() {
        super.onCreate();
        Log.d(TAG, "DroidLogicCpuService is oncreate");
        mSystemControlManager = SystemControlManager.getInstance();
        Log.d(TAG, "start setCpusetsDefault: 0-3");
        setCpusetsDefault();
    }

    @Override
    public void onDestroy() {
        super.onDestroy();
    }
}
