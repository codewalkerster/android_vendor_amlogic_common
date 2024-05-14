/*
 * Copyright (C) 2015 The Android Open Source Project
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
 * limitations under the License
 */

package com.droidlogic;

import android.app.Service;
import android.content.Intent;
import android.database.ContentObserver;
import android.os.Binder;
import android.os.HandlerThread;
import android.os.IBinder;
import android.os.Looper;
import android.os.Handler;
import android.os.Message;
import android.provider.Settings;
import android.util.Log;
import android.app.Notification;
import android.app.PendingIntent;

import com.droidlogic.app.SystemControlManager;

public class FrameRateService extends Service {
    public static final String TAG = "FrameRateService";
    private final IBinder mBinder = new FrameRateBinder();
    private static final int EVENT_UPDATE_UI = 1;
    private static final int R_EVENT_READ = 2;
    private static final int TIMEDELAY = 300;
    private static boolean mEnabled;
    private static FrameRateRemoteView mRemoteView;
    private Handler mHandler;
    private SettingsValueChangeContentObserver mContentOb;
    private SystemControlManager mSystemControlManager;
    private static final String SAVE_FRAME_RATE = "FRAME_RATE";
    private static final int FRAME_RATE_ENABLE = 1;
    private static final int FRAME_RATE_DISABLE = 0;
    private static final String SCENE_FS = "/sys/class/display/frame_rate";
    private static HandlerThread workThread = new HandlerThread("FrameRateThread");
    private static WorkHandler workHandler;
    private final static String NOTIFICATION_CHANNEL_NAME = "CHANNEL_NAME";
    private final static int FOREGROUND_ID = 0;

    private static final String PROP_FRAME_RATE_ENABLE = "persist.vendor.sys.framerate.enable";
    private static final String FRAME_RATE_PROP = "persist.vendor.sys.framerate.feature";

    private String mLastValue;

    public FrameRateService() {
        mSystemControlManager = SystemControlManager.getInstance();
        if (workHandler == null || workHandler.getLooper() == null) {
            workThread.start();
            workHandler = new WorkHandler(workThread.getLooper());
            mRemoteView = FrameRateRemoteView.getInstance();
            mHandler = new Handler() {
                @Override
                public void handleMessage(Message msg) {
                    switch (msg.what) {
                        case EVENT_UPDATE_UI:
                            String text = (String) msg.obj;
                            updateRemoveView(text);
                            break;
                    }
                    super.handleMessage(msg);
                }
            };
        }
    }

    public class SettingsValueChangeContentObserver extends ContentObserver {
        public SettingsValueChangeContentObserver() {
            super( new Handler());
        }
        @Override
        public void onChange(boolean selfChange) {
            Log.d(TAG,"[SettingsValueChangeContentObserver] onchange = "
                    + Settings.Global.getInt(getContentResolver(), SAVE_FRAME_RATE, 0));
            super.onChange(selfChange);
            switch (Settings.Global.getInt(getContentResolver(), SAVE_FRAME_RATE, 0)) {
                case FRAME_RATE_ENABLE:
                    setFrameRateEnabled(true);
                    break;
                case FRAME_RATE_DISABLE:
                    setFrameRateEnabled(false);
                    break;
                default:
                    break;
            }
        }
    }

    @Override
    public IBinder onBind(Intent intent) {
        return mBinder;
    }

    @Override
    public void onCreate() {
        super.onCreate();
        mContentOb = new SettingsValueChangeContentObserver();
        getContentResolver().registerContentObserver(
                Settings.Global.getUriFor(SAVE_FRAME_RATE),
                false,
                mContentOb);
        int sys_value = Settings.Global.getInt(getContentResolver(), SAVE_FRAME_RATE, 0);
        Log.d(TAG, "[AIPQservice] onCreate sys_value= " + sys_value);
        if ((mSystemControlManager.getPropertyBoolean(PROP_FRAME_RATE_ENABLE, false) == true)
                && (sys_value == FRAME_RATE_DISABLE)) {
            Settings.Global.putInt(getContentResolver(), SAVE_FRAME_RATE, FRAME_RATE_ENABLE);
        }
        if ((mSystemControlManager.getPropertyBoolean(PROP_FRAME_RATE_ENABLE, false) == false)
                && (sys_value == 1)) {
            Settings.Global.putInt(getContentResolver(), SAVE_FRAME_RATE, FRAME_RATE_DISABLE);
        }
    }

    @Override
    public int onStartCommand(Intent intent, int flags, int startId) {
        mSystemControlManager = SystemControlManager.getInstance();
        if (getFrameRateEnabled()) {
            enableFrameRate();
        }

        Log.d(TAG, "[FrameRateService] onStartCommand");
        return START_REDELIVER_INTENT;
    }

    @Override
    public void onDestroy() {
        stopForeground(true);
        super.onDestroy();
    }

    class WorkHandler extends Handler {
        public WorkHandler(Looper looper) {
            super(looper);
        }

        @Override
        public void handleMessage(Message msg) {
            switch (msg.what) {
                case R_EVENT_READ:
                    String scenseVal = mSystemControlManager.readSysFs(SCENE_FS);
                    updateUI(scenseVal);

                    if (mEnabled) {
                        sendEmptyMessageDelayed(R_EVENT_READ, TIMEDELAY);
                    }
                    break;
            }
        }
    }

    public class FrameRateBinder extends Binder {
        FrameRateService getService() {
            return FrameRateService.this;
        }
    }

    public boolean isShowing() {
        return mRemoteView != null && mRemoteView.isShow();
    }

    public boolean getFrameRateEnabled() {
        return mSystemControlManager.getPropertyBoolean(PROP_FRAME_RATE_ENABLE, false)
                && mSystemControlManager.getPropertyBoolean(FRAME_RATE_PROP, false);
    }

    public void setFrameRateEnabled(final boolean enable) {
        if (enable) {
            enableFrameRate();
            mSystemControlManager.setProperty(PROP_FRAME_RATE_ENABLE, "true");
        } else {
            disableFrameRate();
            mSystemControlManager.setProperty(PROP_FRAME_RATE_ENABLE, "false");
        }
    }

    private void updateRemoveView(String value) {
        mRemoteView.updateUI(value);
    }

    private void enableFrameRate() {
        if (!mEnabled) {
            mEnabled = true;
            showFrameRateTopView(true);
            workHandler.sendEmptyMessageDelayed(R_EVENT_READ, TIMEDELAY);
        }
    }

    private void disableFrameRate() {
        if (mEnabled) {
            mEnabled = false;
            workHandler.removeMessages(R_EVENT_READ);
            showFrameRateTopView(false);
        } else {
            workHandler.removeMessages(R_EVENT_READ);
        }
    }

    public void updateUI(String ValStr) {
        String newVal = ValStr;
        if (newVal == null || newVal.length() <= 0) {
            newVal = getResources().getString(R.string.prepare);
        }
        if (newVal != null && newVal.length() > 0 && (!newVal.equals(mLastValue))) {
            mLastValue = newVal;
            mHandler.removeMessages(EVENT_UPDATE_UI);
            Message msg = mHandler.obtainMessage();
            msg.what = EVENT_UPDATE_UI;
            msg.obj = newVal;
            mHandler.sendMessage(msg);
        }
    }

    //display top view
    private void showFrameRateTopView(boolean show) {
        if (show) {
            if (!mRemoteView.isCreated()) {
                mRemoteView.createView(getApplicationContext());
            }
            mRemoteView.show();
        } else {
            mRemoteView.hide();
        }
    }

}
