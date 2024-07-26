/*
 * Copyright (c) 2014 Amlogic, Inc. All rights reserved.
 *
 * This source code is subject to the terms and conditions defined in the
 * file 'LICENSE' which is part of this source code package.
 *
 * Description:
 *     AMLOGIC GlobalKeyReceiver
 */

package com.droidlogic;

import android.os.PowerManager;
import android.content.Context;
import android.app.Service;
import com.droidlogic.app.SystemControlManager;
import android.content.BroadcastReceiver;
import android.content.IntentFilter;
import java.io.IOException;
import android.os.PowerManager.LowPowerStandbyPolicy;
import android.os.PowerManager;
import android.util.Log;
import android.content.Intent;
import android.os.IBinder;
import android.os.Binder;


public class LowPowerPolicyService extends Service {
    private static final String TAG = "LowPowerPolicyService";

    private BroadcastReceiver mLowPowerChangeReceiver = null;
    private PowerManager mPowerManager = null;
    private final IBinder mBinder = new LowPowerPolicyBinder();


    public static SystemControlManager getSystemControlManager() {
        return SystemControlManager.getInstance();
    }

    public class LowPowerPolicyBinder extends Binder {
        LowPowerPolicyService getService() {
            return LowPowerPolicyService.this;
        }
    }

    @Override
    public IBinder onBind(Intent intent) {
        return mBinder;
    }


    @Override
    public void onCreate() {
        Log.i(TAG, "onCreate");
        super.onCreate();
        if (mPowerManager == null) {
            mPowerManager = (PowerManager) getSystemService(Context.POWER_SERVICE);
        }
        setupLowPowerPolicyListener(getApplicationContext());
        updateLowPowerbehaviorAccordingPolicy();
    }

    private void setupLowPowerPolicyListener(Context context) {
        if (mLowPowerChangeReceiver == null) {
            mLowPowerChangeReceiver = new LowPowerStandbyPolicyReceiver();
            IntentFilter filter = new IntentFilter();
            filter.addAction(PowerManager.ACTION_LOW_POWER_STANDBY_POLICY_CHANGED);
            filter.addAction(PowerManager.ACTION_LOW_POWER_STANDBY_ENABLED_CHANGED);
            context.registerReceiver(mLowPowerChangeReceiver, filter, 0);
        }
    }
    private class LowPowerStandbyPolicyReceiver extends BroadcastReceiver {
        @Override
        public void onReceive(Context context, Intent intent) {
        if (mPowerManager == null) {
            mPowerManager = (PowerManager) context.getSystemService(Context.POWER_SERVICE);
        }

        String action = intent.getAction();
        Log.d(TAG, "onReceive:" + action);
        if (PowerManager.ACTION_LOW_POWER_STANDBY_POLICY_CHANGED.equals(action)) {
            updateLowPowerbehaviorAccordingPolicy();
        } else if (PowerManager.ACTION_LOW_POWER_STANDBY_ENABLED_CHANGED.equals(action)) {
            updateLowPowerbehaviorAccordingPolicy();
        } else {
            Log.e(TAG, "This should not be happen!!!");
        }

    }
    }

    private void updateLowPowerbehaviorAccordingPolicy() {
        boolean isSupported = false;
        boolean isEnabled = false;
        if (mPowerManager != null) {
            isSupported = mPowerManager.isLowPowerStandbySupported();
            isEnabled = mPowerManager.isLowPowerStandbyEnabled();
        }
        Log.d(TAG, "isSupported:" + isSupported);
        Log.d(TAG, "isEnabled:" + isEnabled);
        if (isSupported && isEnabled) {

            LowPowerStandbyPolicy currentPolicy = mPowerManager.getLowPowerStandbyPolicy();
            if (currentPolicy == null) {
                return ;
            }
            String identifier = currentPolicy.getIdentifier();
            Log.d(TAG, "identifier:" + identifier); //low_energy_use,moderate_energy_use,high_energy_use
            boolean ret = false;
            if (identifier.equals("low_energy_use")) {
                getSystemControlManager().setProperty("persist.vendor.sys.low_energy_mode", "enable");
                ret = getSystemControlManager().writeSysFs("/sys/class/ethernet/wol", "0"); //wol off
            } else {
                getSystemControlManager().setProperty("persist.vendor.sys.low_energy_mode", "disable");
                ret = getSystemControlManager().writeSysFs("/sys/class/ethernet/wol", "1"); //wol on
            }
        }
    }


}
