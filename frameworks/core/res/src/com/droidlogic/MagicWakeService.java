/*
 * Copyright (c) 2014 Amlogic, Inc. All rights reserved.
 *
 * This source code is subject to the terms and conditions defined in the
 * file 'LICENSE' which is part of this source code package.
 *
 * Description:
 *     AMLOGIC MagicWakeService
 */

package com.droidlogic;

import android.app.Service;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.os.Handler;
import android.os.IBinder;
import android.os.Looper;
import android.os.PowerManager;
import android.os.SystemProperties;
import android.util.Log;

import java.io.File;
import java.io.FileInputStream;
import java.io.IOException;
import java.util.concurrent.TimeUnit;
import com.droidlogic.app.SystemControlManager;

public class MagicWakeService extends Service {
    private static final String TAG = "MagicWakeService";
    private static final String MAGIC_WAKE_PROPERTY = "vendor.sys.magic_wake.triggered";
    private static final String MAGIC_WAKE_CONTROL_PROPERTY = "vendor.sys.magic_wake.control";
    private static final String CONTROL_ENABLE = "1";
    private static final String CONTROL_DISABLE = "0";
    private static final long WAKE_LOCK_TIMEOUT = 3 * 60 * 1000L; // 3 minutes timeout
    private static final long MONITOR_INTERVAL = 1000; // Check interval in milliseconds

    private PowerManager.WakeLock wakeLock;
    private PowerManager.WakeLock screenLock;
    private Handler handler;
    private boolean monitoringWake = false;
    private boolean startMagicWake = false;
    private Runnable monitorRunnable;

    private final BroadcastReceiver screenReceiver = new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, Intent intent) {
            String action = intent.getAction();
            Log.i(TAG, "onReceive: action=" + action);

            if (Intent.ACTION_SCREEN_OFF.equals(action)) {
                Log.i(TAG, "Screen turned off, starting Magic Packet monitoring");
                if (startMagicWake == false) {
                    startMagicWake = true;
                    enableMagicWake();
                    startMonitoring();
                }
            } else if (Intent.ACTION_SCREEN_ON.equals(action)) {
                Log.i(TAG, "Screen turned on, immediately stopping magic packet listening");
                // Immediately stop magic packet listening regardless of the reason for screen turning on
                startMagicWake = false;
                disableMagicWake();
                stopMonitoring();
                releaseWakeLocks();
            }
        }
    };

    @Override
    public void onCreate() {
        Log.i(TAG, "onCreate");
        super.onCreate();

        handler = new Handler(Looper.getMainLooper());

        IntentFilter filter = new IntentFilter();
        filter.addAction(Intent.ACTION_SCREEN_OFF);
        filter.addAction(Intent.ACTION_SCREEN_ON);
        registerReceiver(screenReceiver, filter);

        // Initialize monitoring task
        monitorRunnable = new Runnable() {
            @Override
            public void run() {
                if (monitoringWake) {
                    checkWakeTriggers();
                    handler.postDelayed(this, MONITOR_INTERVAL);
                }
            }
        };
    }

    private void checkIfWakenByMagicPacket() {
        Log.i(TAG, "checkIfWakenByMagicPacket Checking if system was woken by Magic Packet");
        // Check property value
        String value = SystemControlManager.getInstance().getPropertyString(MAGIC_WAKE_PROPERTY, "0");
        if ("1".equals(value)) {
            Log.i(TAG, "System was woken up by Magic Packet (from property)");
            acquireWakeLocks();
            SystemControlManager.getInstance().setProperty(MAGIC_WAKE_PROPERTY, "0");
        }
    }

    private void checkWakeTriggers() {
        // Check property
        String value = SystemProperties.get(MAGIC_WAKE_PROPERTY, "0");
        if ("1".equals(value)) {
            Log.i(TAG, "Magic packet wake trigger detected from property");
            acquireWakeLocks();
            SystemControlManager.getInstance().setProperty(MAGIC_WAKE_PROPERTY, "0");
        }
    }

    private void enableMagicWake() {
        Log.i(TAG, "Enabling magic wake");
        SystemControlManager.getInstance().setProperty(MAGIC_WAKE_CONTROL_PROPERTY, CONTROL_ENABLE);
    }

    private void disableMagicWake() {
        Log.i(TAG, "Disabling magic wake");
        SystemControlManager.getInstance().setProperty(MAGIC_WAKE_CONTROL_PROPERTY, CONTROL_DISABLE);
    }

    @Override
    public int onStartCommand(Intent intent, int flags, int startId) {
        Log.i(TAG, "onStartCommand: intent=" + intent + ", flags=" + flags + ", startId=" + startId);
        // Check if the service is being restarted by the daemon
        if (intent == null && (flags & START_FLAG_RETRY) != 0) {
            checkIfWakenByMagicPacket();
        }
        return START_STICKY;
    }

    private void startMonitoring() {
        Log.i(TAG, "startMonitoring");
        if (monitoringWake) return;

        monitoringWake = true;
        handler.post(monitorRunnable);
    }

    private void stopMonitoring() {
        Log.i(TAG, "stopMonitoring");
        monitoringWake = false;
        handler.removeCallbacks(monitorRunnable);
    }

    private void acquireWakeLocks() {
        Log.i(TAG, "acquireWakeLocks");
        PowerManager pm = (PowerManager) getSystemService(POWER_SERVICE);

        if (wakeLock == null) {
            wakeLock = pm.newWakeLock(PowerManager.PARTIAL_WAKE_LOCK, TAG + ":WakeLock");
            wakeLock.setReferenceCounted(false);
        }
        if (!wakeLock.isHeld()) {
            wakeLock.acquire(WAKE_LOCK_TIMEOUT);
        }

        if (screenLock == null) {
            screenLock = pm.newWakeLock(
                PowerManager.SCREEN_BRIGHT_WAKE_LOCK | PowerManager.ACQUIRE_CAUSES_WAKEUP,
                TAG + ":ScreenLock");
            screenLock.setReferenceCounted(false);
        }
        if (!screenLock.isHeld()) {
            screenLock.acquire(WAKE_LOCK_TIMEOUT);
        }
    }

    private void releaseWakeLocks() {
        Log.i(TAG, "releaseWakeLocks");
        if (wakeLock != null && wakeLock.isHeld()) {
            wakeLock.release();
        }
        if (screenLock != null && screenLock.isHeld()) {
            screenLock.release();
        }
    }

    @Override
    public void onDestroy() {
        Log.i(TAG, "onDestroy");
        stopMonitoring();
        releaseWakeLocks();
        unregisterReceiver(screenReceiver);
        super.onDestroy();
    }

    @Override
    public IBinder onBind(Intent intent) {
        return null;
    }
}