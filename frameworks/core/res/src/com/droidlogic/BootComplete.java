/*
 * Copyright (c) 2014 Amlogic, Inc. All rights reserved.
 *
 * This source code is subject to the terms and conditions defined in the
 * file 'LICENSE' which is part of this source code package.
 *
 * Description:
 *     AMLOGIC BootComplete
 */

package com.droidlogic;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.pm.PackageManager;
import android.os.Handler;
import android.os.UserHandle;
import android.os.UserManager;
import android.provider.Settings;
import android.content.ContentResolver;
import android.util.Log;
import android.provider.Settings;
import android.database.ContentObserver;
import android.net.Uri;
import android.os.Handler;

import android.content.ComponentName;
import android.content.ServiceConnection;
import android.os.IBinder;
import android.os.RemoteException;
import java.lang.reflect.AccessibleObject;
import java.lang.reflect.Field;
import android.os.SystemProperties;

import com.droidlogic.app.DroidLogicUtils;
import com.droidlogic.app.PlayBackManager;
import com.droidlogic.app.SystemControlManager;
import com.droidlogic.app.UsbCameraManager;
import com.droidlogic.hdmi.HdmiCecService;
import com.droidlogic.app.SystemControlManager;
import android.content.DialogInterface;
import android.os.Handler;
import android.os.Message;

public class BootComplete extends BroadcastReceiver {
    private static final String TAG             = "BootComplete";
    private static final String DECRYPT_STATE = "encrypted";
    private static final String DECRYPT_TYPE = "file";
    private static final String DROID_SETTINGS_PACKAGE = "com.droidlogic.tv.settings";
    private static final String DROID_SETTINGS_ENCRYPTKEEPERFBE = "com.droidlogic.tv.settings.CryptKeeperFBE";

    private static final String SOUNDBAR_MODE = "soundbar_mode";
    private static final String PROPERTY_SOUNDBAR_MODE_SUPPORTED = "ro.vendor.platform.support.soundbar";
    private static final String KEY_POWER = "116";
    private static final String KEY_HOME = "102";
    private static final String NEED_START_NTF = "need_start_netflix_app";
    private static final String SAVE_WOL = "WOL";
    private static final String AMATI_FEATURE = "com.google.android.feature.AMATI_EXPERIENCE";

    private boolean mHasTvUiMode;

    private static final int MSG_SHOW_USB_POWER_STATUS = 0;
    private Context mContext;

    @Override
    public void onReceive(Context context, Intent intent) {
        String action = intent.getAction();
        Log.i(TAG, "action: " + action);
        if (SettingsPref.getSavedBootCompletedStatus(context)) {
            SettingsPref.setSavedBootCompletedStatus(context, false);
            return;
        }
        SettingsPref.setSavedBootCompletedStatus(context, true);

        mHasTvUiMode = DroidLogicUtils.isTv();
        final ContentResolver resolver = context.getContentResolver();

        if (SettingsPref.getFirstRun(context)) {
            Log.i(TAG, "first running: " + context.getPackageName());
            SettingsPref.setFirstRun(context, false);
        }

        boolean soundbarSupported = getBooleanProperty(PROPERTY_SOUNDBAR_MODE_SUPPORTED, false);
        Log.i(TAG, "soundbar supported:" + soundbarSupported);
        if (Settings.Global.getInt(resolver, SOUNDBAR_MODE, -1) == -1) {
            Settings.Global.putInt(resolver, SOUNDBAR_MODE, soundbarSupported ? 1 : 0);
        }

        //use to check whether disable camera or not
        new UsbCameraManager(context).bootReady();

        new PlayBackManager(context).initHdmiSelfadaption();

        if (getBooleanProperty("ro.vendor.subtitle.enable_fallback_display", false)) {
            context.startService(new Intent(context, SubtitleDisplayer.class));
        }

        SystemControlManager systemcontrolmanager = SystemControlManager.getInstance();
        String wakeup_key_event = systemcontrolmanager.readSysFs("/sys/class/remote0/amremote0/wakeup_key_event");
        Log.i(TAG, "wakeup_key_event:" + wakeup_key_event);

        if (context.getPackageManager().hasSystemFeature(NetflixService.FEATURE_SOFTWARE_NETFLIX)) {
            Intent netflix_intent = new Intent(context, NetflixService.class);

            if (wakeup_key_event != null && wakeup_key_event.length() > 0) {
                String key_map = systemcontrolmanager.readSysFs("/sys/class/remote0/amremote0/keymap");
                if (key_map != null && key_map.length() > 0) {
                    int index = key_map.indexOf(wakeup_key_event);
                    if (index >= 0) {
                        String wakeup_keycode = key_map.substring(index + wakeup_key_event.length());
                        Log.d(TAG, "wakeup key:" + wakeup_keycode);
                        if (!wakeup_keycode.contains(KEY_POWER) && !wakeup_keycode.contains(KEY_HOME)) {
                            netflix_intent.putExtra(NEED_START_NTF, true);
                            SystemProperties.set("persist.sys.customkey.wakeup", "true");
                        }
                    }
                }
            }
            context.startService(netflix_intent);
        }
        if (context.getPackageManager().hasSystemFeature(PackageManager.FEATURE_HDMI_CEC)) {
            context.startService(new Intent(context, HdmiCecService.class));
        }
        if (SystemProperties.get("sys.vendor.usb_otg.control").equals("enable")) {
            context.startService(new Intent(context, DeviceControlService.class));
        }

        Intent intent_t = new Intent();
        //Log.d(TAG, "start MdnsOffloadCmdService!!!");
        intent_t.setComponent(new ComponentName("com.android.tv.mdnsoffloadcmd", "com.android.tv.mdnsoffloadcmd.MdnsOffloadCmdService"));
        context.startForegroundService(intent_t);

        context.startService(new Intent(context,NtpService.class));
        context.startService(new Intent(context,ShutdownService.class));

        if (getBooleanProperty("ro.vendor.platform.need.bench.promote", false)) {
            context.startService(new Intent(context, DroidLogicBenchService.class));
        }

        if (mHasTvUiMode)
            context.startService(new Intent(context, DroidLogicPowerService.class));

        if (getBooleanProperty("ro.vendor.platform.support.network_led", false) == true)
            context.startService(new Intent(context, NetworkSwitchService.class));

        /*if (mHasTvUiMode)
            context.startService(new Intent(context, EsmService.class));*/

        if (getBooleanProperty("vendor.sys.bandwidth.enable", false))
            context.startService(new Intent(context, DDRBandwidthService.class));

        /*  AML default rotation config, cannot use with shipping_api_level=28
            String rotProp = mSystemControlManager.getPropertyString("persist.vendor.sys.app.rotation", "");
            ContentResolver res = context.getContentResolver();
            int acceRotation = Settings.System.getIntForUser(res,
                Settings.System.ACCELEROMETER_ROTATION,
                0,
                UserHandle.USER_CURRENT);
            if (rotProp != null && ("middle_port".equals(rotProp) || "force_land".equals(rotProp))) {
                    if (0 != acceRotation) {
                        Settings.System.putIntForUser(res,
                            Settings.System.ACCELEROMETER_ROTATION,
                            0,
                            UserHandle.USER_CURRENT);
                    }
            }
         */

        // enableCryptKeeperComponent(context);

        if (Intent.ACTION_BOOT_COMPLETED.equals(action)) {
            SettingsPref.setSavedBootCompletedStatus(context, false);
        }
        updateDeveloperOptionsWatcher(context);
        mContext = context;
        showUsbPowerDialog();

        // start FrameRateService
        context.startService(new Intent(context, FrameRateService.class));
    }

    private boolean getBooleanProperty(String property, boolean defVal) {
        try {
            return (boolean)Class.forName("android.os.SystemProperties")
                .getMethod("getBoolean", new Class[] { String.class, Boolean.TYPE })
                .invoke(null, new Object[] { property, defVal });
        } catch(Exception e) {
            e.printStackTrace();
        }
        return false;
    }

/*    private void enableCryptKeeperComponent(Context context) {
        String state = SystemProperties.get("ro.crypto.state");
        String type = SystemProperties.get("ro.crypto.type");
        boolean isMultiUser = UserManager.supportsMultipleUsers();
        if (("".equals(state) || !DECRYPT_STATE.equals(state) || !DECRYPT_TYPE.equals(type)) || !isMultiUser) {
            return;
        }

        PackageManager pm = context.getPackageManager();
        ComponentName name = new ComponentName(DROID_SETTINGS_PACKAGE, DROID_SETTINGS_ENCRYPTKEEPERFBE);
        Log.d(TAG, "enableCryptKeeperComponent " + name);
        try {
            pm.setComponentEnabledSetting(name, PackageManager.COMPONENT_ENABLED_STATE_ENABLED,
                    PackageManager.DONT_KILL_APP);
        } catch (Exception e) {
            Log.e(TAG, e.toString());
        }
    }
*/
    private Handler mHandler = new Handler() {
        public void handleMessage(Message msg) {
            switch (msg.what) {
            case MSG_SHOW_USB_POWER_STATUS:
                Log.i(TAG, "handleMessage:MSG_SHOW_USB_POWER_STATUS, POWER_LEVEL: " + msg.arg1);
                Intent intent1 = new Intent();
                        intent1.setComponent(new ComponentName("com.droidlogic","com.droidlogic.USBPowerActivity"));
                        intent1.addFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
                        intent1.putExtra("POWER_LEVEL", msg.arg1);
                        mContext.startActivity(intent1);

                        break;
                    default :
                        Log.d(TAG, "Not impossible!");
                        break;
                }
            }
    };

    private void showUsbPowerDialog() {
            SystemControlManager mSystemControlManager = SystemControlManager.getInstance();
            String usb_status_cc = mSystemControlManager.getPropertyString("ro.boot.cc.status", "1.5a@5v");
            String usb_cc_enable = mSystemControlManager.getBootenv("ubootenv.var.cc_enable", "0");
            Log.i(TAG,  "usb_status_cc:" + usb_status_cc + ", usb_cc_enable:" + usb_cc_enable);
            int power_level = -1;
            if (usb_status_cc.contains("0.5a")) {
                power_level = 0;
            }
            else if (usb_status_cc.contains("1.5a")) {
                power_level = 1;}
            else if (usb_status_cc.contains("3a")) {
                power_level = 2;}
            if (power_level >= 0 && "1".equals(usb_cc_enable)) {
                Log.i(TAG, "sendmsg:MSG_SHOW_USB_POWER_STATUS");
                Log.i(TAG,  "power_level:" + power_level);
                 //mHandler.sendEmptyMessageDelayed(MSG_SHOW_USB_POWER_STATUS,10000);
                 mHandler.sendMessageDelayed(mHandler.obtainMessage(MSG_SHOW_USB_POWER_STATUS, power_level, 0), 10000);
            }
    }

    private boolean needCecExtend(SystemControlManager sm, Context context) {
        //return sm.getPropertyInt("ro.hdmi.device_type", -1) == HdmiDeviceInfo.DEVICE_PLAYBACK;
        return true;
    }

    private static void updateDeveloperOptionsWatcher(final Context context) {
        Uri settingUri = Settings.Global.getUriFor(
                Settings.Global.DEVELOPMENT_SETTINGS_ENABLED);

        ContentObserver developerOptionsObserver =
                new ContentObserver(new Handler()) {
                    @Override
                    public void onChange(boolean selfChange) {
                        super.onChange(selfChange);

                        boolean developerOptionsEnabled = (1 ==
                                Settings.Global.getInt(context.getContentResolver(),
                                        Settings.Global.DEVELOPMENT_SETTINGS_ENABLED, 0));

                        Log.d(TAG, "onChange developerOptionsEnabled=" + developerOptionsEnabled);
                        if (developerOptionsEnabled) {
                            context.startService(new Intent(context, ThermalService.class));
                        } else {
                            context.stopService(new Intent(context, ThermalService.class));
                        }
                    }
                };

        context.getContentResolver().registerContentObserver(settingUri,
                false, developerOptionsObserver);
        developerOptionsObserver.onChange(true);
    }

}
