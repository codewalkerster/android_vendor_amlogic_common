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
import java.util.List;
import android.os.PowerManager;
import android.os.PowerManager.LowPowerStandbyPortDescription;
import vendor.amlogic.hardware.droidmdnsoffload.IDroidMdnsOffload;
import java.util.function.Supplier;
import com.android.internal.annotations.GuardedBy;
import androidx.annotation.Nullable;
import android.os.ServiceManager;
import android.os.RemoteException;

public class LowPowerPolicyService extends Service {
    private static final String TAG = "LowPowerPolicyService";

    private BroadcastReceiver mLowPowerChangeReceiver = null;
    private PowerManager mPowerManager = null;
    private final IBinder mBinder = new LowPowerPolicyBinder();
    private Context mContext = null;

    private static int PROTOCOL_TCP = 0;
    private static int PROTOCOL_UDP = 1;

    private static int MATCHER_LOCAL = 0;
    private static int MATCHER_REMOTE = 1;

    @Nullable
    private Supplier<IDroidMdnsOffload> mService_mdnsoffload;

    private void initMdnsService(Supplier<IDroidMdnsOffload> service) {
        mService_mdnsoffload = service.get() != null ? service : null;
    }

    private static class VintfHalCache implements Supplier<IDroidMdnsOffload>, IBinder.DeathRecipient {
        @GuardedBy("this")
        private IDroidMdnsOffload mInstance = null;

        @Override
        public synchronized IDroidMdnsOffload get() {
            if (mInstance == null) {
                IBinder binder = Binder.allowBlocking(
                        ServiceManager.waitForDeclaredService(IDroidMdnsOffload.DESCRIPTOR + "/default"));
                if (binder != null) {
                    Log.d(TAG, "binder is not null");
                    mInstance = IDroidMdnsOffload.Stub.asInterface(binder);
                    try {
                        binder.linkToDeath(this, 0);
                    } catch (RemoteException e) {
                        Log.e(TAG, "Unable to register DeathRecipient for " + mInstance);
                    }
                } else
                    Log.d(TAG, "binder is null");
            }
            return mInstance;
        }

        @Override
        public synchronized void binderDied() {
            Log.d(TAG, "binderDied");
            mInstance = null;
        }
    }

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
        mContext = this;
        initMdnsService(new VintfHalCache());
        setupLowPowerPolicyListener(getApplicationContext());
        updateLowPowerbehaviorAccordingPolicy();
        updateWakePorts();
    }

    private void setupLowPowerPolicyListener(Context context) {
        if (mLowPowerChangeReceiver == null) {
            mLowPowerChangeReceiver = new LowPowerStandbyPolicyReceiver();
            IntentFilter filter = new IntentFilter();
            filter.addAction(PowerManager.ACTION_LOW_POWER_STANDBY_POLICY_CHANGED);
            filter.addAction(PowerManager.ACTION_LOW_POWER_STANDBY_ENABLED_CHANGED);
            filter.addAction(PowerManager.ACTION_LOW_POWER_STANDBY_PORTS_CHANGED);
            context.registerReceiver(mLowPowerChangeReceiver, filter, Context.RECEIVER_EXPORTED);
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
        } else if (PowerManager.ACTION_LOW_POWER_STANDBY_PORTS_CHANGED.equals(action)) {
            updateWakePorts();
        } else {
            Log.e(TAG, "This should not be happen!!!");
        }

    }
    }

    private void updateWakePorts() {
        try {
            List<LowPowerStandbyPortDescription> list = mPowerManager.getActiveLowPowerStandbyPorts();
            if (list != null && list.size() > 0) {
                Log.i(TAG, "list size: " + list.size());
                int num = list.size();
                int[] protocolList = new int[num];
                int[] matcherList = new int[num];
                int[] portList = new int[num];

                int  ret_num = 0;
                for (int i = 0;i < list.size();i++) {
                    LowPowerStandbyPortDescription description = list.get(i);
                    Log.d(TAG, "description:" + description.toString());
                    int protocol = description.getProtocol();
                    int matcher = description.getPortMatcher();
                    int portNum = description.getPortNumber();

                    if (protocol != LowPowerStandbyPortDescription.PROTOCOL_TCP && protocol != LowPowerStandbyPortDescription.PROTOCOL_UDP) {
                        Log.d(TAG, "wrong protocol ,return");
                        return;
                    }
                    if (matcher != LowPowerStandbyPortDescription.MATCH_PORT_LOCAL && protocol != LowPowerStandbyPortDescription.MATCH_PORT_REMOTE) {
                        Log.d(TAG, "wrong match port ,return");
                        return;
                    }

                    if (description.getProtocol() == LowPowerStandbyPortDescription.PROTOCOL_TCP) {
                        protocolList[i] = PROTOCOL_TCP;
                    } else if (description.getProtocol() == LowPowerStandbyPortDescription.PROTOCOL_UDP) {
                        protocolList[i] = PROTOCOL_UDP;
                    }

                    if (description.getProtocol() == LowPowerStandbyPortDescription.MATCH_PORT_LOCAL) {
                        matcherList[i] = MATCHER_LOCAL;
                    } else if (description.getProtocol() == LowPowerStandbyPortDescription.MATCH_PORT_REMOTE) {
                        matcherList[i] = MATCHER_REMOTE;
                    }

                    portList[i] = portNum;
                    ret_num++;
                    Log.d(TAG, "protocol:" + protocolList[i] + " matcher:" + matcherList[i] +" port:" + portList[i]);
                }

                if (ret_num > 0 ) {
                    if (mService_mdnsoffload != null && mService_mdnsoffload.get() != null)
                        mService_mdnsoffload.get().setWakePorts(ret_num,protocolList,matcherList,portList);
                }
            }
        } catch (RemoteException ex) {
            Log.e(TAG, "Failed updateWakePorts", ex);
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
