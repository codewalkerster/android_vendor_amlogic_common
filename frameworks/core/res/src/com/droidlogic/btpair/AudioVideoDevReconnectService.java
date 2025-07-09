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
import android.os.PowerManager;

public class AudioVideoDevReconnectService extends Service {
    private static final String TAG = "AudioVideoDevReconnectService";
    private static final boolean DEBUG = false;

        private Context mContext =  null;
        private boolean mIsDisconnectting = false;
        private boolean mScreenOn = true; //when system bootup,no screen on msg is sent
        private static final Object sLock = new Object();
        private BluetoothAdapter mBtAdapter;

        private ArrayList<String> mWaitToConnectSpeakers = new ArrayList<>();
        private ArrayList<String> mBondRemoteDevices = new ArrayList<>();

        private ArrayList<CachedBluetoothDevice> mReconnectFailedRemoteDevList = new ArrayList<>();
        private final int MSG_RECONNECT_SPEAKER_DEVICE = 0;
        private final int MSG_CHECK_ALL_REMOTE_RECONNECTED = 1;
        private final int MSG_RECONNECT_REMOTE_DEVICE = 2;
        private final int MSG_RETRY_RECONNECT_SPEAKER = 3;

        private static int DEV_UNKNOWN = 0;
        private static int DEV_SPEAKER = 1;
        private static int DEV_REMOTE = 2;

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
            filter.addAction(BluetoothDevice.ACTION_ACL_CONNECTED);
            filter.addAction(BluetoothDevice.ACTION_ACL_DISCONNECTED);
            registerReceiver(receiver, filter, mContext.RECEIVER_EXPORTED);

            mBtAdapter = BluetoothAdapter.getDefaultAdapter();

            myThread = new AudioVideoDevPairThread();
            myThread.start();

        }

    private final BroadcastReceiver receiver = new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, Intent intent) {
            mContext = context;

            String action = intent.getAction();
            Log.i(TAG, "onReceive:" + action + ",mIsDisconnectting:" + mIsDisconnectting);
            synchronized (sLock) {
                if (Intent.ACTION_SCREEN_ON.equals(action)) {
                    mScreenOn = true;

                    if (mWaitToConnectSpeakers.size() > 0) {
                        mMyHandler.removeMessages(MSG_RECONNECT_SPEAKER_DEVICE);
                        mMyHandler.sendEmptyMessageDelayed(MSG_RECONNECT_SPEAKER_DEVICE, mIsDisconnectting ? 3000 : 0);

                    }

                    mMyHandler.removeMessages(MSG_CHECK_ALL_REMOTE_RECONNECTED);
                    mMyHandler.sendEmptyMessageDelayed(MSG_CHECK_ALL_REMOTE_RECONNECTED, 1000);
                } else if (Intent.ACTION_SCREEN_OFF.equals(action)) {
                    mMyHandler.removeMessages(MSG_RECONNECT_SPEAKER_DEVICE);
                    mMyHandler.removeMessages(MSG_CHECK_ALL_REMOTE_RECONNECTED);
                    mMyHandler.removeMessages(MSG_RECONNECT_REMOTE_DEVICE);
                    mScreenOn = false;

                    updateBondSpeakers();
                    updateBondRemotes();
                } else if (BluetoothDevice.ACTION_ACL_CONNECTED.equals(action) ) {
                    final BluetoothDevice device = intent.getParcelableExtra(BluetoothDevice.EXTRA_DEVICE);
                    String aclConnectDev = device.getAddress();
                    Log.d(TAG, "ACTION_ACL_CONNECTED:" + aclConnectDev + ",name:" + device.getName());
                    int devType = getDevType(device);

                    if (mScreenOn) {
                        if (mWaitToConnectSpeakers.size() > 0 && devType == DEV_SPEAKER) {
                            Log.d(TAG, "    device is connected:" + aclConnectDev);
                            if (mWaitToConnectSpeakers.contains(aclConnectDev)) {
                                mWaitToConnectSpeakers.remove(aclConnectDev);
                                Log.d(TAG, "    ** mWaitToConnectSpeakers ** rm:" + aclConnectDev);
                                if (mWaitToConnectSpeakers.size() > 0) {
                                    mMyHandler.removeMessages(MSG_RECONNECT_SPEAKER_DEVICE);
                                    mMyHandler.sendEmptyMessage(MSG_RECONNECT_SPEAKER_DEVICE);
                                } else {
                                    Log.i(TAG, "    all speaker are reconnected.");
                                }
                            }
                        }
                    } else {
                        if (devType == DEV_SPEAKER) {
                            disconnectAudioDev(device);
                            if (!mWaitToConnectSpeakers.contains(aclConnectDev)) {
                                mWaitToConnectSpeakers.add(aclConnectDev);
                                Log.i(TAG, "  ** mWaitToConnectSpeakers ** add:" + aclConnectDev);
                            }
                        } else if (devType == DEV_REMOTE) {
                            updateBondRemotes();
                        }
                    }

               } else if (BluetoothDevice.ACTION_ACL_DISCONNECTED.equals(action) ) {
                   if (mScreenOn) {
                       return;
                   }
                   final BluetoothDevice device = intent.getParcelableExtra(BluetoothDevice.EXTRA_DEVICE);
                   String aclDisConnectDev = device.getAddress();
                   Log.d(TAG, "ACTION_ACL_DISCONNECTED:" + aclDisConnectDev + ",name:" + device.getName());
                   int devType = getDevType(device);
                   if (devType == DEV_REMOTE) {
                       updateBondRemotes();
                   }
               }
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


    private void updateBondSpeakers() {
         //mWaitToConnectSpeakers.clear();
         Log.i(TAG, "updateBondSpeakers,now have "+ mWaitToConnectSpeakers.size());

         if (mBtAdapter == null) {
             Log.w(TAG, "  Can't get BT adapter, return");
             return ;
         }
         if (!mBtAdapter.isEnabled()) {
            return;
         }
        final Set<BluetoothDevice> bondedDevices = mBtAdapter.getBondedDevices();
        if (bondedDevices == null) {
            Log.i(TAG, "  No bondedDevices");
            return;
        }

        for (final BluetoothDevice device : bondedDevices) {
            final String deviceAddress = device.getAddress();
            String deviceName = device.getName();
            if (TextUtils.isEmpty(deviceAddress)) {
                Log.i(TAG, "  device:" + deviceName + " addr is :" + deviceAddress + ",return");
                continue;
            }

            if (getDevType(device) == DEV_SPEAKER) {
                if (!isConnected(device)) {
                    Log.i(TAG, "  skip disconnected speaker:" + deviceName);
                    return;
                }
                mWaitToConnectSpeakers.add(device.getAddress());
                mIsDisconnectting = true;
                Log.i(TAG, "  ** mWaitToConnectSpeakers ** add: " + device.getAddress() + ",name:" + deviceName);
                disconnectAudioDev(device);
            }
        }
     }

    private void updateBondRemotes() {
         mBondRemoteDevices.clear();
         Log.d(TAG, "** mBondRemoteDevices ** clear ");
         if (mBtAdapter == null) {
             Log.w(TAG, "  Can't get BT adapter, return");
             return ;
         }
         if (!mBtAdapter.isEnabled()) {
            return;
         }
        final Set<BluetoothDevice> bondedDevices = mBtAdapter.getBondedDevices();
        if (bondedDevices == null) {
            Log.i(TAG, "  No bondedDevices");
            return;
        }

        for (final BluetoothDevice device : bondedDevices) {
            final String deviceAddress = device.getAddress();
            String deviceName = device.getName();
            if (TextUtils.isEmpty(deviceAddress)) {
                continue;
            }

            if (getDevType(device) == DEV_REMOTE) {
                if (!isConnected(device)) {
                    Log.i(TAG, "    skip store remote:" + deviceName);
                    return;
                }
                Log.i(TAG, "  ** mBondRemoteDevices ** add: " + device.getAddress() + ",name:" + deviceName);
                mBondRemoteDevices.add(device.getAddress());
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
            boolean ret;
            Log.d(TAG, "handleMessage:" + msg.what);
            synchronized (sLock) {
                switch (msg.what) {
                    case MSG_RECONNECT_SPEAKER_DEVICE:
                        if (mWaitToConnectSpeakers.size() == 0) {
                            Log.i(TAG, "no device need to be reconnect now");
                            break;
                        }
                        String deviceAddress = mWaitToConnectSpeakers.get(0);
                        ret = connectDevice(deviceAddress);
                        if (!ret) {
                            mMyHandler.sendEmptyMessageDelayed(MSG_RETRY_RECONNECT_SPEAKER, 500);
                        }
                        break;
                    case MSG_CHECK_ALL_REMOTE_RECONNECTED:
                        if (mBondRemoteDevices != null && mBondRemoteDevices.size() > 0) {
                            updateReconnectFailedRemotes();
                            if (mReconnectFailedRemoteDevList.size() > 0)
                                mMyHandler.sendEmptyMessageDelayed(MSG_RECONNECT_REMOTE_DEVICE, 500);
                        }
                        break;
                    case MSG_RECONNECT_REMOTE_DEVICE:
                        if (mReconnectFailedRemoteDevList.size() > 0) {
                            CachedBluetoothDevice cachedDev = mReconnectFailedRemoteDevList.get(0);
                            Log.i(TAG, "reconnect remote:" + cachedDev.getName());
                            cachedDev.connect();
                            mReconnectFailedRemoteDevList.remove(0);
                            if (mReconnectFailedRemoteDevList.size() > 0)
                                mMyHandler.sendEmptyMessageDelayed(MSG_RECONNECT_REMOTE_DEVICE, 500);
                        }
                        break;
                    case MSG_RETRY_RECONNECT_SPEAKER:
                            if (mWaitToConnectSpeakers.size() == 0) {
                            Log.i(TAG, "no need to retry to connect speaker now");
                            break;
                        }
                        String retryDevAddr = mWaitToConnectSpeakers.get(0);
                        ret = connectDevice(retryDevAddr);
                        if (!ret) {
                            mWaitToConnectSpeakers.remove(0);
                            Log.i(TAG, "stop retry,skip this device");
                            if (mWaitToConnectSpeakers.size() > 0) {
                                mMyHandler.sendEmptyMessage(MSG_RECONNECT_SPEAKER_DEVICE);
                            }
                        }
                        break;
                    default:
                       Log.d(TAG, "No handler case available for message: " + msg.what);
                }
            }
        }
    }

    private boolean connectDevice(String devAddress) {
        Log.i(TAG, "connectDevice:" + devAddress);
        BluetoothDevice device = getDeviceByAddress(devAddress);
        if (device != null) {
            if (mLocalBluetoothManager != null) {
                CachedBluetoothDevice cachedDevice = mLocalBluetoothManager.getCachedDeviceManager().findDevice(device);
                if (DEBUG) {
                    String deviceName = device.getName();
                    String deviceName_a = device.getAlias();
                    boolean isCachedDevConnect = cachedDevice.isConnected();
                    boolean isBusy = cachedDevice.isBusy();
                    boolean isConnect = isConnected(device);
                    Log.d(TAG, "      deviceName:" + deviceName + ",isConnect:" + isConnect + ",isConnected: " + device.isConnected());
                    Log.d(TAG, "      cachedDeviceName:" + deviceName_a + "isCachedDevConnect:" + isCachedDevConnect + ",isBusy: " + isBusy);
                    Log.d(TAG, "      mIsDisconnectting:" + mIsDisconnectting);
                }
                if (cachedDevice != null) {
                    cachedDevice.connect();
                    return true;
                } else {
                    Log.e(TAG, "failed to find:" + devAddress + " in cached list");
                }
            }
        }else {
            Log.e(TAG, "failed to find:" + devAddress + " in bonded dev list");
        }
        return false;
    }


    private BluetoothDevice getDeviceByAddress(String address) {

        List<BluetoothDevice> devices = getDevices();
        BluetoothDevice curDevice = null;

        for (BluetoothDevice device: devices) {
            Log.i(TAG, "getDeviceByAddress:" + device.getAddress());
            if (address.equals(device.getAddress())) {
                curDevice = device;
                break;
            }
        }
        return curDevice;
    }

    private List<BluetoothDevice> getDevices() {
        if (mBtAdapter != null) {
            return new ArrayList<>(mBtAdapter.getBondedDevices());
        } else {
            Log.e(TAG, "BtAdpater is null.getDevices fail");
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
                    Log.d(TAG, "disconnectAudioDev : " + cachedDevice.getAddress());
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
        int bondState = device.getBondState();
        boolean isConnected = (bondState == BluetoothDevice.BOND_BONDED) && device.isConnected();
        Log.d(TAG, "isConnected,dev:" + device.getAddress() + ",bondState:  " + bondState
                + ",connected:" + device.isConnected());
        return isConnected;
    }

    private void updateReconnectFailedRemotes() {
        mReconnectFailedRemoteDevList.clear();

        final Collection<CachedBluetoothDevice> cachedDevices =
                 mLocalBluetoothManager.getCachedDeviceManager().getCachedDevicesCopy();
        Log.d(TAG, "updateReconnectFailedRemotes");

        for (CachedBluetoothDevice cachedBluetoothDevice : cachedDevices) {

            BluetoothClass btClass = cachedBluetoothDevice.getBtClass();
            if (btClass != null) {
                int bt_class = btClass.getMajorDeviceClass();
                if (bt_class == BluetoothClass.Device.Major.PERIPHERAL) {
                    if (cachedBluetoothDevice.isConnected() || cachedBluetoothDevice.isBusy()) {
                        Log.d(TAG, "    skip to add remote:" + cachedBluetoothDevice.getName());
                        continue;
                    }
                    if (mBondRemoteDevices.contains(cachedBluetoothDevice.getAddress())) {
                        mReconnectFailedRemoteDevList.add(cachedBluetoothDevice);
                        Log.d(TAG, "    ** mReconnectFailedRemoteDevList ** add: " + cachedBluetoothDevice.getName()
                                   + ",addr:" + cachedBluetoothDevice.getAddress());
                    }
                }
            } else {
                Log.e(TAG, "    updateReconnectFailedRemotes error, btClass is null");
            }
        }

     }


    private int getDevType(BluetoothDevice device) {
        BluetoothClass btClass = device.getBluetoothClass();
        if ( btClass != null) {
            int bt_class = btClass.getMajorDeviceClass();

            if (bt_class == BluetoothClass.Device.Major.AUDIO_VIDEO) {
                Log.d(TAG, "    getDevType,bt_class:" + bt_class + ",type:DEV_SPEAKER");
                return DEV_SPEAKER;
            } else if (bt_class == BluetoothClass.Device.Major.PERIPHERAL) {
                Log.d(TAG, "    getDevType,bt_class:" + bt_class + ",type:DEV_REMOTE");
                return DEV_REMOTE;
            }
            Log.d(TAG, "    getDevType,bt_class:" + bt_class + ",type:DEV_UNKNOWN");
        }

        return DEV_UNKNOWN;
    }

}
