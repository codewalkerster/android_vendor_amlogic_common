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
import android.util.Log;

import android.content.Intent;
import android.content.Context;
import android.content.ContentResolver;
import android.provider.Settings;
import android.database.ContentObserver;
import android.os.IBinder;
import android.os.Binder;

import android.os.Handler;
import android.net.Uri;

import com.droidlogic.app.SystemControlManager;

public class DeviceControlService extends Service {
    public static final String TAG = "DeviceControlService";
    private static final String DEVICE_PATH = "/sys/devices/virtual/misc/usb3phy/sw_otg/otg_mode";
    private final SystemControlManager mSystemControlManager = SystemControlManager.getInstance();

    private SettingsValueChangeContentObserver mSettingsObserver;
    private ContentResolver mContentResolver;
    private final IBinder mBinder = new DeviceControlBinder();

    public class DeviceControlBinder extends Binder {
        DeviceControlService getService() {
            return DeviceControlService.this;
        }
    }

    @Override
    public IBinder onBind(Intent intent) {
        return mBinder;
    }

    @Override
    public void onCreate() {
        Log.i(TAG, "[DeviceControlService]  onCreate");
        mContentResolver = getContentResolver();

        boolean shouldEnable = (Settings.Global.getInt(mContentResolver,
          Settings.Global.ADB_ENABLED, 0) > 0);
        Log.d(TAG, "default: shouldEnable: " + shouldEnable + "  Settings.Global.ADB_ENABLED: " + Settings.Global.getInt(mContentResolver,
                    Settings.Global.ADB_ENABLED, 0));
        if (shouldEnable) {
            setDeviceMode("DEVICE");
        } else {
            setDeviceMode("HOST");
        }

        mSettingsObserver = new SettingsValueChangeContentObserver();
        mContentResolver.registerContentObserver(Settings.Global.getUriFor(Settings.Global.ADB_ENABLED),
                false, mSettingsObserver);
        super.onCreate();
    }

    @Override
    public void onDestroy() {
        getContentResolver().unregisterContentObserver(mSettingsObserver);
        super.onDestroy();
    }

    private class SettingsValueChangeContentObserver extends ContentObserver {
        private final Uri mAdbUsbUri = Settings.Global.getUriFor(Settings.Global.ADB_ENABLED);

        SettingsValueChangeContentObserver() {
            super(new Handler());
        }

        // onChange is set up to run in service thread.
        @Override
        public void onChange(boolean selfChange, Uri uri) {

            Log.i(TAG, "Uri: " + uri);
            if (mAdbUsbUri.equals(uri)) {
                boolean shouldEnable = (Settings.Global.getInt(mContentResolver,
                        Settings.Global.ADB_ENABLED, 0) > 0);
                Log.d(TAG, "shouldEnable: " + shouldEnable + "  Settings.Global.ADB_ENABLED: " + Settings.Global.getInt(mContentResolver,
                        Settings.Global.ADB_ENABLED, 0));
                if (shouldEnable) {
                    setDeviceMode("DEVICE");
                } else {
                    setDeviceMode("HOST");
                }
            }
            getDeviceControlValue();
        }

    }

    public void setDeviceMode(String userDeviceMode) {
        mSystemControlManager.writeSysFs(DEVICE_PATH, userDeviceMode);
    }

    public String getDeviceControlValue() {
        String deviceValue = mSystemControlManager.readSysFs(DEVICE_PATH);
        Log.d(TAG, "getDeviceControlValue: " + deviceValue);
        return deviceValue;
    }
}
