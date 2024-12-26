// Copyright 2022 Google Inc. All Rights Reserved.

package com.droidlogic.android.tv.connecteddevices;

import android.bluetooth.BluetoothDevice;

import com.google.android.tv.btservices.BluetoothDeviceService;
import com.google.android.tv.btservices.remote.DfuProvider;
import com.google.android.tv.btservices.remote.DfuBinary;
import com.google.android.tv.btservices.remote.RemoteProxy;
import com.google.android.tv.btservices.remote.Version;
import com.google.android.tv.btservices.remote.DefaultProxy;

public class DefaultBluetoothDeviceService extends BluetoothDeviceService {

    // Device firmware update is provided by DfuService, so disabling Dfu here
    @Override
    protected DfuProvider getDfuProvider() {
        return null;
    }

    // Default implementation to provide battery level of remote control
    @Override
    protected RemoteProxy createRemoteProxy(BluetoothDevice device) {
        return new DefaultProxy(this, device);
    }
}
