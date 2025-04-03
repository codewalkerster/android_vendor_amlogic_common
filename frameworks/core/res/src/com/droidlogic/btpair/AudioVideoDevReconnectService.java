/*
 * Copyright (c) 2014 Amlogic, Inc. All rights reserved.
 *
 * This source code is subject to the terms and conditions defined in the
 * file 'LICENSE' which is part of this source code package.
 *
 * Description:
 *     AMLOGIC AudioVideoDevReconnectReceiver
 */

package com.droidlogic.btpair;

import android.content.Context;
import android.content.Intent;
import android.util.Log;

import android.content.BroadcastReceiver;


import android.content.pm.PackageInfo;
import java.util.List;

import android.bluetooth.BluetoothAdapter;
import android.bluetooth.BluetoothDevice;
import java.util.Set;
import android.text.TextUtils;

import android.bluetooth.BluetoothClass;
import android.os.Handler;
import android.os.Message;
import android.os.Handler;
import android.os.IBinder;
import android.os.Message;
import java.util.ArrayList;
import java.util.Set;
import android.text.TextUtils;
import android.content.ComponentName;
import android.bluetooth.BluetoothClass;
import android.bluetooth.BluetoothDevice;

import android.content.IntentFilter;
import android.os.Looper;

import com.android.settingslib.bluetooth.LocalBluetoothManager;
import com.android.settingslib.bluetooth.CachedBluetoothDevice;
import android.app.Service;
import java.util.Collection;

public class AudioVideoDevReconnectService extends Service {
    private static final String TAG = "AudioVideoDevReconnectService";
    private static final boolean DEBUG = false;

        private Context mContext =  null;
        private boolean mWaitForDisconnct = false;

        private ArrayList<String> mBondAudioDevices = new ArrayList<>();
        private ArrayList<CachedBluetoothDevice> mDisconnectedRemoteDevices = new ArrayList<>();
        private final int MSG_RECONNECT_BOND_DEVICE = 0;
        private final int MSG_UPDATE_DISCONNECTED_REMOCE_DEVICE = 1;
        private final int MSG_RECONNECT_REMOCE_DEVICE = 2;

        private AudioVideoDevPairThread myThread = null;
        private MyHandler mMyHandler;
        private LocalBluetoothManager mLocalBluetoothManager;

        @Override
        public IBinder onBind(Intent intent) {
            return null;
        }

        @Override
        public void onCreate() {
            mContext = this;
            Log.w(TAG, "oncreate");

            mLocalBluetoothManager = getLocalBluetoothManager(mContext);
            IntentFilter filter = new IntentFilter();
            filter.addAction(Intent.ACTION_SCREEN_ON);
            filter.addAction(Intent.ACTION_SCREEN_OFF);
            registerReceiver(receiver, filter, mContext.RECEIVER_EXPORTED);

            myThread = new AudioVideoDevPairThread();
            myThread.start();

        }

        private final BroadcastReceiver receiver = new BroadcastReceiver() {
            @Override
            public void onReceive(Context context, Intent intent) {
                mContext = context;

                String action = intent.getAction();
                Log.i(TAG, "onReceive:" + action + ",mWaitForDisconnct:" + mWaitForDisconnct);
                if (Intent.ACTION_SCREEN_ON.equals(action)) {
                    if (mBondAudioDevices.size() > 0) {
                        mMyHandler.removeMessages(MSG_RECONNECT_BOND_DEVICE);
                        mMyHandler.sendEmptyMessageDelayed(MSG_RECONNECT_BOND_DEVICE, mWaitForDisconnct ? 3000 : 0);
                    }
                    mMyHandler.removeMessages(MSG_UPDATE_DISCONNECTED_REMOCE_DEVICE);
                    mMyHandler.sendEmptyMessageDelayed(MSG_UPDATE_DISCONNECTED_REMOCE_DEVICE, 1000);
                } else if (Intent.ACTION_SCREEN_OFF.equals(action)) {
                    mWaitForDisconnct = true;
                    mMyHandler.removeMessages(MSG_RECONNECT_BOND_DEVICE);
                    updateCachedAudioDev();
                }
           }
        };

    class AudioVideoDevPairThread extends Thread {
        AudioVideoDevPairThread() {
            super("AudioVideoDevPairThread");
        }
        @Override
        public void run() {
            Looper.prepare();

            mMyHandler = new MyHandler();
            Looper.loop();
        }
    }


    private void updateCachedAudioDev() {
         mBondAudioDevices.clear();

         final BluetoothAdapter btAdapter = BluetoothAdapter.getDefaultAdapter();
         if (btAdapter == null) {
             Log.w(TAG, "Can't get BT adapter, return");
             return ;
         }
         if (!btAdapter.isEnabled()) {
            return;
         }
        final Set<BluetoothDevice> bondedDevices = btAdapter.getBondedDevices();
        if (bondedDevices == null) {
            Log.i(TAG, "No bondedDevices");
            return;
        }

        for (final BluetoothDevice device : bondedDevices) {
            final String deviceAddress = device.getAddress();
            String deviceName = device.getName();
            if (TextUtils.isEmpty(deviceAddress)) {
                continue;
            }

            BluetoothClass btClass = device.getBluetoothClass();
            if ( btClass != null) {
                int bt_class = btClass.getMajorDeviceClass();
                if (bt_class == BluetoothClass.Device.Major.AUDIO_VIDEO) {
                    mBondAudioDevices.add(device.getAddress());
                    Log.d(TAG, "mBondAudioDevices add: " + device.getAddress());
                    disconnectAudioDev(device);
                }
            }
        }
     }

    class MyHandler extends Handler {
        MyHandler() {
            super();
        }

        MyHandler(Looper looper) {
            super(looper);
        }
        @Override
        public void handleMessage(Message msg) {
            switch (msg.what) {
                case MSG_RECONNECT_BOND_DEVICE:
                    mWaitForDisconnct = false;
                    String deviceAddress = mBondAudioDevices.get(0);
                    connectDevice(deviceAddress);

                    mBondAudioDevices.remove(0);
                    if (mBondAudioDevices.size() > 0)
                        mMyHandler.sendEmptyMessageDelayed(MSG_RECONNECT_BOND_DEVICE, 500);
                    break;
                case MSG_UPDATE_DISCONNECTED_REMOCE_DEVICE:
                    updateDisconnectedRemote();
                    if (mDisconnectedRemoteDevices.size() > 0)
                        mMyHandler.sendEmptyMessageDelayed(MSG_RECONNECT_REMOCE_DEVICE, 500);
                    break;
                case MSG_RECONNECT_REMOCE_DEVICE:
                    CachedBluetoothDevice cachedDev = mDisconnectedRemoteDevices.get(0);
                    Log.d(TAG, "reconnect:" + cachedDev.getName());
                    cachedDev.connect();
                    mDisconnectedRemoteDevices.remove(0);
                    if (mDisconnectedRemoteDevices.size() > 0)
                        mMyHandler.sendEmptyMessageDelayed(MSG_RECONNECT_REMOCE_DEVICE, 500);
                    break;
                default:
                   Log.d(TAG, "No handler case available for message: " + msg.what);
            }
        }
    }

    private void connectDevice(String devAddress) {
        BluetoothDevice device = findDevice(devAddress);
        if (device != null) {
            if (mLocalBluetoothManager != null) {
                CachedBluetoothDevice cachedDevice = mLocalBluetoothManager.getCachedDeviceManager().findDevice(device);
                boolean isConnect = isConnected(device);
                boolean isCachedDevConnect = cachedDevice.isConnected();
                boolean isBusy = cachedDevice.isBusy();
                if (DEBUG) {
                    String deviceName = device.getName();
                    String deviceName_a = device.getAlias();
                    Log.d(TAG, "      deviceName:" + deviceName + ",isConnect:" + isConnect + ",isConnected: " + device.isConnected());
                    Log.d(TAG, "cachedDeviceName:" + deviceName_a + "isCachedDevConnect:" + isCachedDevConnect + ",isBusy: " + isBusy);
                    Log.d(TAG, "mWaitForDisconnct:" + mWaitForDisconnct);
                }
                if (cachedDevice != null) {
                    cachedDevice.connect();
                } else {
                    Log.e(TAG, "failed to find:" + devAddress);
                }
            }
        }
    }


    private static BluetoothDevice findDevice(String address) {

        List<BluetoothDevice> devices = getDevices();
        BluetoothDevice curDevice = null;

        for (BluetoothDevice device: devices) {
            Log.d(TAG, "device:" + device.getAddress());
            if (address.equals(device.getAddress())) {
                curDevice = device;
                break;
            }
        }
        return curDevice;
    }

    private static List<BluetoothDevice> getDevices() {
        final BluetoothAdapter btAdapter = BluetoothAdapter.getDefaultAdapter();
        if (btAdapter != null) {
            return new ArrayList<>(btAdapter.getBondedDevices());
        }
        return new ArrayList<>(); // Empty list
    }

    public static LocalBluetoothManager getLocalBluetoothManager(Context context) {
        try {
            return LocalBluetoothManager.getInstance(context, (c, bluetoothManager) -> {});
        } catch (Exception e) {
            Log.w(TAG, "Error getting LocalBluetoothManager.", e);
            return null;
        }
    }

    private void disconnectAudioDev(BluetoothDevice device) {
        if (device != null) {
            if (mLocalBluetoothManager != null) {
                CachedBluetoothDevice cachedDevice = mLocalBluetoothManager.getCachedDeviceManager().findDevice(device);
                if (cachedDevice != null) {
                    cachedDevice.disconnect();
                } else {
                    Log.e(TAG, "disconnect failed because failed to find device");
                }
            }
        }
    }

    public static boolean isConnected(BluetoothDevice device) {
        if (device == null) {
            return false;
        }
        boolean isConnected = device.getBondState() == BluetoothDevice.BOND_BONDED && device.isConnected();
        Log.d(TAG, "BluetoothDevice.BondState: " + device.getBondState()
                + ";BluetoothDevice.isConnected: " + device.isConnected());
        return isConnected;
    }

    private void updateDisconnectedRemote() {
        mDisconnectedRemoteDevices.clear();

        final Collection<CachedBluetoothDevice> cachedDevices =
                mLocalBluetoothManager.getCachedDeviceManager().getCachedDevicesCopy();

        for (CachedBluetoothDevice cachedBluetoothDevice : cachedDevices) {
            if (cachedBluetoothDevice.isConnected() || cachedBluetoothDevice.isBusy()) {
                Log.d(TAG, "skip to reconnect:" + cachedBluetoothDevice.getName());
                continue;
            }

            BluetoothClass btClass = cachedBluetoothDevice.getBtClass();
            if (btClass != null) {
                int bt_class = btClass.getMajorDeviceClass();
                Log.e(TAG, "updateDisconnectedRemote, bt_class is " + bt_class);
                if (bt_class == BluetoothClass.Device.Major.PERIPHERAL) {
                    mDisconnectedRemoteDevices.add(cachedBluetoothDevice);
                    Log.d(TAG, "mDisconnectedRemoteDevices add: " + cachedBluetoothDevice.getName() + ",addr:" + cachedBluetoothDevice.getAddress());
                }
            } else {
                Log.e(TAG, "updateDisconnectedRemote error, btClass is null");
            }
        }
     }



}
