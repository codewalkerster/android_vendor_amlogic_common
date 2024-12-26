// Copyright 2022 Google Inc. All Rights Reserved.

package com.droidlogic.android.tv.connecteddevices;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.util.Log;

public class BootReceiver extends BroadcastReceiver {

    private static final String TAG = "Atv.BootReceiver";
    private static final boolean DEBUG = false;

    @Override
    public void onReceive(Context context, Intent intent) {
        if (DEBUG) Log.i(TAG, "onReceive");
        Intent BtDeviceServiceIntent = new Intent(context, DefaultBluetoothDeviceService.class);
        context.startService(BtDeviceServiceIntent);
    }
}
