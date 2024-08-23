
package com.droidlogic.btpair;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.pm.PackageManager;
import android.os.SystemProperties;
import android.util.Log;
import com.droidlogic.app.SystemControlManager;

public class FastBootComplete extends BroadcastReceiver {
    private static final String TAG             = "FastBootComplete";

    @Override
    public void onReceive(Context context, Intent intent) {
        String action = intent.getAction();
        Log.i(TAG, "action:" + action);
        if (Intent.ACTION_BOOT_COMPLETED.equals(action)) {
            Intent gattServiceIntent = new Intent(context, DialogBluetoothService.class);
            context.startService(gattServiceIntent);

            SystemControlManager systemcontrolmanager = SystemControlManager.getInstance();
            String wakeup_key_event = systemcontrolmanager.readSysFs("/sys/class/remote0/amremote0/wakeup_key_event");
            Log.i(TAG, "wakeup_key_event:" + wakeup_key_event);
        }
    }
}
