/*
 * Copyright (c) 2014 Amlogic, Inc. All rights reserved.
 *
 * This source code is subject to the terms and conditions defined in the
 * file 'LICENSE' which is part of this source code package.
 *
 * Description:
 *     AMLOGIC NetflixService
 */

package com.droidlogic;

import android.app.ActivityManager;
import android.app.IActivityManager;
import android.app.IProcessObserver;
import android.app.TaskStackListener;
import android.app.ActivityTaskManager.RootTaskInfo;

import android.app.Service;
import android.hardware.hdmi.HdmiControlManager;
import android.content.pm.PackageManager;
import android.content.pm.ResolveInfo;
import android.content.ComponentName;
import android.content.Context;
import android.content.BroadcastReceiver;
import android.content.Intent;
import android.content.IntentFilter;
import android.media.AudioDeviceCallback;
import android.media.AudioDeviceInfo;
import android.media.AudioManager;
import android.media.AudioFormat;
import android.media.AudioSystem;
import android.net.Uri;
import android.os.IBinder;
import android.os.RemoteException;
import android.os.Handler;
import android.os.Message;
import android.os.PowerManager;
import android.os.PowerManager.WakeLock;




import android.provider.Settings;
import android.text.TextUtils;
import android.util.Log;
import android.database.ContentObserver;
import android.content.ContentResolver;
import android.provider.DeviceConfig;
import org.json.JSONObject;
import android.hardware.display.DisplayManager;
import android.hardware.display.HdrConversionMode;

import android.view.Display;
import android.os.Handler;

import java.io.File;
import java.util.Arrays;

import java.util.List;
import java.util.Scanner;
import android.os.SystemProperties;
import android.os.HandlerExecutor;

import com.droidlogic.app.DroidAudioEffect;
import com.droidlogic.app.DroidAudioManager;
import com.droidlogic.app.DroidLogicUtils;
import com.droidlogic.app.SystemControlManager;
import com.droidlogic.app.OutputModeManager;
import android.net.wifi.WifiManager;

public class NetflixService extends Service {
    private static final String TAG = "NetflixService";

    public static final String FEATURE_SOFTWARE_NETFLIX = "droidlogic.software.netflix";

    private static final String NETFLIX_PKG_NAME = "com.netflix.ninja";
    private static final String YOUTUBE_PKG_NAME = "com.google.android.youtube.tv";
    private static final String LAUNCHER_PKG_NAME = "com.google.android.apps.tv.launcherx";
    private static final String SYS_AUDIO_CAP = "/sys/class/amhdmitx/amhdmitx0/aud_cap";
    private static final String WAKEUP_REASON_DEVICE = "/sys/class/meson_pm/suspend_reason";
    private static final String WAKEUP_REASON_DEVICE_OTHER = "/sys/devices/platform/aml_pm/suspend_reason";
    private static final String NRDP_PLATFORM_CAP = "nrdp_platform_capabilities";
    private static final String NRDP_AUDIO_PLATFORM_CAP = "nrdp_audio_platform_capabilities";
    private static final String NRDP_AUDIO_PLATFORM_CAP_MS12 = "nrdp_audio_platform_capabilities_ms12";
    private static final String NRDP_PLATFORM_CONFIG_DIR = "/vendor/etc/";
    private static final String DOLBY_LIB = "dolby_lib";
    private static final String NETFLIX_KEY_POWER_MODE = "power_on";
    private static final String ACTION_LAUNCH_APP = "com.google.global_button.ACTION_LAUNCH_APP";
    private static final String ACTION_LAUNCH_BENCH_APP = "com.amlogic.ACTION_LAUNCH_BENCH_APP";
    private static final String EXTRA_PACKAGE_NAME = "launchPackageName";
    private static final String EXTRA_LAUNCH_INTENT = "launchIntent";
    private static final String NETFLIX_INTENT = "com.netflix.action.NETFLIX_KEY_START";
     // Power State Change on Active Source Lost Settings values
    private static final String LOST_NONE = "none";
    private static final String LOST_STANDBY_NOW = "standby_now";
    private static final String TEMP_HDR = "temp_hdr";
    private final String NDRP_CEC_STATUS = "nrdp_video_platform_capabilities";
    private final String HDR_COVERSION_MODE = "hdr_conversion_mode";

    private static final String STR_ALWAYS = "0";
    private static final String STR_ADAPTIVE = "1";
    private static final int WAKEUP_REASON_CUSTOM = 9;
    private static final int MSG_UPDATA = 1;
    private static final int MSG_UPDATA_DISPLAY = 2;
    private static final int UI_AUDIO_DELAY_OFFSET_TV_NON_DOLBY = 60;
    private static final int UI_AUDIO_DELAY_OFFSET_TV_MS12 = 120;
    private static final int UI_AUDIO_DELAY_OFFSET_OTT_DOLBY = 70;
    private static final int UI_AUDIO_DELAY_OFFSET_OTT_PCM = 75;
    private static final int DEVICE_CLEANUP_TIMEOUT=8000;
    private static boolean atmosSupportedByConfig = false;
    private static boolean ddpSupportedByConfig = false;
    private boolean mIsNetflixFg = false;
    private boolean mIsYoutubeFg = false;
    private boolean hasMS12 = false;
    private boolean tempHDR = false;
    private Context mContext;
    private SystemControlManager mSCM;
    private AudioManager mAudioManager;
    private HdmiControlManager mHdmiControlManager;
    private DisplayManager mDisplayManager;
    private SettingsObserver mSettingsObserver;
    private CecStatusObserver mCecStatusObserver;
    private HdrStatusObserver mHdrStatusObserver;
    private OutputModeManager mOutputModeManager = null;
    private DroidAudioManager mDroidAudioManager = null;
    private final Object mLock = new Object();
    private IActivityManager mIActivityManager;
    private ProcessObserver mProcessObserver;
    private DeviceConfigListener mDeviceConfigListener = null;
    private  Handler mMsgHandler;
    private String mOriginalPowerStateChangeValue;
    private HdrConversionMode mHdrConversionMode;
    private AudioManagerAudioDeviceCallback mAudioManagerAudioDeviceCallback;
    private ActivityManager mActivityManager;

    private static final String NEED_START_NTF = "need_start_netflix_app";
    private WifiManager mWifiManager;

    private class SettingsObserver extends ContentObserver {
        public SettingsObserver(Handler handler) {
            super(handler);
        }

        @Override
        public void onChange(boolean selfChange, Uri uri) {
            int surround = mDroidAudioManager.getDigitalAudioMode();
            Log.i(TAG, "onChange surround: " + DroidAudioManager.digitalModeToString(surround));
            switch (surround) {
                case DroidAudioManager.DIGITAL_AUDIO_MODE_AUTO:
                case DroidAudioManager.DIGITAL_AUDIO_MODE_PASSTHROUGH:
                    Log.i(TAG, "onChange auto/passthrough");
                    setNrdpCapabilitiesIfNeed(NRDP_AUDIO_PLATFORM_CAP, true);
                case DroidAudioManager.DIGITAL_AUDIO_MODE_MANUAL:
                case DroidAudioManager.DIGITAL_AUDIO_MODE_PCM:
                    break;
                default:
                    Log.d(TAG, "error surround format");
                    break;
            }

            refreshAudioCapabilities(false);

        }
    }

    private class CecStatusObserver extends ContentObserver {
        public CecStatusObserver(Handler handler) {
            super(handler);
        }

        @Override
        public void onChange(boolean selfChange, Uri uri, int flags) {
            notifyChange("nrdp_video_platform_capabilities/activeCecState");
        }

        private void notifyChange(String settingsNote) {
            ContentResolver cr = mContext.getContentResolver();
            cr.notifyChange(Settings.Global.getUriFor(settingsNote), null,
                ContentResolver.NOTIFY_NO_DELAY);
            Log.i(TAG,"notify activeness changes without delay");
        }
    }

    private class HdrStatusObserver extends ContentObserver {
        public HdrStatusObserver(Handler handler) {
            super(handler);
        }

        @Override
        public void onChange(boolean selfChange, Uri uri, int flags) {
            updateHdrSettings();
        }
    }

    private final class DeviceConfigListener implements DeviceConfig.OnPropertiesChangedListener {
        private static final String KEY_LIGHT_IDLE_AFTER_INACTIVE_TIMEOUT = "light_after_inactive_to";
        private static final String LIGHT_IDLE_AFTER_INACTIVE_TIMEOUT_VALUE = "3600000";

        public DeviceConfigListener() {
            Log.d(TAG, "DeviceConfigListener");
            DeviceConfig.addOnPropertiesChangedListener(DeviceConfig.NAMESPACE_DEVICE_IDLE, new HandlerExecutor(new Handler()), this);
            setDefaultVal();
        }

        public void onPropertiesChanged(DeviceConfig.Properties properties) {
            for (String name : properties.getKeyset()) {
                if (name == null) {
                    continue;
                }

                switch (name) {
                case KEY_LIGHT_IDLE_AFTER_INACTIVE_TIMEOUT:

                    long light_after_inactive_to = properties.getLong(KEY_LIGHT_IDLE_AFTER_INACTIVE_TIMEOUT, 60000);

                    if (light_after_inactive_to != Long.parseLong(LIGHT_IDLE_AFTER_INACTIVE_TIMEOUT_VALUE)) {
                        Log.d(TAG, "DEVICE_IDLE changed light_after_inactive_to = " + light_after_inactive_to);
                        setDefaultVal();
                    }

                    break;
                }
            }
        }

        private void setDefaultVal() {
            boolean deviceConfigSetBoolean = DeviceConfig.setProperty(DeviceConfig.NAMESPACE_DEVICE_IDLE,
                    KEY_LIGHT_IDLE_AFTER_INACTIVE_TIMEOUT,
                    LIGHT_IDLE_AFTER_INACTIVE_TIMEOUT_VALUE, false);
            Log.d(TAG,
                "set DEVICE_IDLE light_after_inactive_to = " + LIGHT_IDLE_AFTER_INACTIVE_TIMEOUT_VALUE + ":" + deviceConfigSetBoolean);
        }
    }

    /** Notifications of audio device connection and disconnection events. */
    private class AudioManagerAudioDeviceCallback extends AudioDeviceCallback {
        private void updateNrdpProfile(AudioDeviceInfo[] devices, boolean state) {
            for (AudioDeviceInfo deviceInfo : devices) {
                Log.d(TAG, "isSink = " + deviceInfo.isSink() + ", " + (state ? "connect" : "disconnect") + " Audio device: " + deviceInfo.getType());
                if (deviceInfo.isSink() &&
                        (deviceInfo.getType() == AudioDeviceInfo.TYPE_HDMI ||
                                deviceInfo.getType() == AudioDeviceInfo.TYPE_HDMI_ARC ||
                                deviceInfo.getType() == AudioDeviceInfo.TYPE_HDMI_EARC ||
                                deviceInfo.getType() == AudioDeviceInfo.TYPE_WIRED_HEADPHONES)) {
                    Log.d(TAG, (state ? "connect" : "disconnect") + " Audio device: " + deviceInfo.getType());
                    refreshAudioCapabilities(false);
                    return;
                }
            }
        }
        @Override
        public void onAudioDevicesAdded(AudioDeviceInfo[] addedDevices) {
            updateNrdpProfile(addedDevices, true);
        }

        @Override
        public void onAudioDevicesRemoved(AudioDeviceInfo[] removedDevices) {
            updateNrdpProfile(removedDevices, false);
        }
    }

    private BroadcastReceiver ScreenOffReceiver = new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, Intent intent) {
            synchronized (mLock) {
                if (mIsNetflixFg) {
                    Log.d(TAG, "wake lock foreground" );
                    PowerManager powerManager = (PowerManager) getSystemService(Context.POWER_SERVICE);
                    WakeLock wakeLock = powerManager.newWakeLock(PowerManager.PARTIAL_WAKE_LOCK,TAG);
                    wakeLock.acquire(DEVICE_CLEANUP_TIMEOUT);
                }
            }
        }
    };

    @Override
    public void onCreate() {
        super.onCreate();
        mContext = this;
        mSCM = SystemControlManager.getInstance();
        mAudioManager = (AudioManager) getSystemService(Context.AUDIO_SERVICE);
        mOutputModeManager = OutputModeManager.getInstance(mContext);
        mDroidAudioManager = DroidAudioManager.getInstance(mContext);
        mHdmiControlManager = (HdmiControlManager)mContext.getSystemService(Context.HDMI_CONTROL_SERVICE);
        mDisplayManager = (DisplayManager)getSystemService(DisplayManager.class);
        mActivityManager = (ActivityManager)getSystemService(ActivityManager.class);
        mWifiManager = (WifiManager) getSystemService(WifiManager.class);

        int dolbyMs12Config = DroidAudioManager.getInstance(mContext).getDroidAudioConfig(DroidAudioManager.DROID_AUDIO_CONFIG_ID_IS_SUPPORT_MS12);
        hasMS12 = (dolbyMs12Config == 1);
        Log.d(TAG, "ms12Supported = " + hasMS12);
        initNrdpCapabilities();
        atmosSupportedByConfig = isAtmosConfiged();
        Log.d(TAG, "atmosSupportedByConfig = " + atmosSupportedByConfig);
        ddpSupportedByConfig = isDdpConfiged();
        Log.d(TAG, "ddpSupportedByConfig = " + ddpSupportedByConfig);

        mAudioManagerAudioDeviceCallback = new AudioManagerAudioDeviceCallback();
        mAudioManager.registerAudioDeviceCallback(mAudioManagerAudioDeviceCallback, null);
        refreshAudioCapabilities(true);

        updateHdrSettings();
        mSettingsObserver = new SettingsObserver(new Handler());
        getContentResolver().registerContentObserver(Settings.Global.getUriFor(DroidAudioManager.ENCODED_SURROUND_OUTPUT),
                false, mSettingsObserver);
        getContentResolver().registerContentObserver(Settings.Global.getUriFor(DroidAudioManager.ENCODED_SURROUND_OUTPUT_ENABLED_FORMATS),
                false, mSettingsObserver);
        getContentResolver().registerContentObserver(Settings.Global.getUriFor(DroidAudioManager.DB_ID_DROIDLOGIC_AUDIO_OUTPUT_DEVICE),
                false, mSettingsObserver);

        mCecStatusObserver = new CecStatusObserver(new Handler());
        getContentResolver().registerContentObserver(Settings.Global.getUriFor(NDRP_CEC_STATUS),
                false, mCecStatusObserver);

        mHdrStatusObserver = new HdrStatusObserver(new Handler());
        getContentResolver().registerContentObserver(Settings.Global.getUriFor(HDR_COVERSION_MODE),
                false, mHdrStatusObserver);

        startNetflixIfNeed();

        mDeviceConfigListener = new DeviceConfigListener();

        mProcessObserver = new ProcessObserver();
        mIActivityManager = ActivityManager.getService();
        try {
            mIActivityManager.registerProcessObserver(mProcessObserver);
            mIActivityManager.registerTaskStackListener(mTaskStackListener);
        } catch (RemoteException e) {
            Log.e(TAG, "could not get IActivityManager");
        }
        if (SystemProperties.get("sys.vendor.ethernet.wol", "enable").equals("enable")) {
            if (mSCM != null)
                mSCM.writeSysFs("/sys/class/ethernet/wol" , "1");
        }

        final String[] macAddresses = mWifiManager.getFactoryMacAddresses();
	 if (macAddresses != null && macAddresses.length > 0) {
            Log.d(TAG, "  wlan0 mac getfactory mac-macAddresses[0]=" + macAddresses[0]);
            mSCM.setProperty("ro.vendor.nrdp.wifi_mac", macAddresses[0]);
        }
        mMsgHandler = new Handler() {
            @Override
            public void handleMessage(Message msg) {
                switch (msg.what) {
                    case MSG_UPDATA:
                        // Log.d(TAG, "handleMessage");
                        netflixFGStateUpdate();
                        break;
                    default:
                        Log.d(TAG, "No handler case available for message: " + msg.what);
                }
            }
        };
        resetHdrPolicy();

        IntentFilter filter = new IntentFilter(Intent.ACTION_SCREEN_OFF);
        registerReceiver(ScreenOffReceiver, filter);
    }

    @Override
    public int onStartCommand(Intent intent, int flags, int startId) {
        if (intent != null) {
            boolean isNeedStartApp = intent.getBooleanExtra(NEED_START_NTF, false);
            if (isNeedStartApp) {
                launchNetflix();
            }
        }
        return super.onStartCommand(intent, flags, startId);
    }

    @Override
    public void onDestroy() {
        try {
            mIActivityManager.unregisterProcessObserver(mProcessObserver);
        } catch (RemoteException e) {
            Log.e(TAG, "Failed to unregister listeners", e);
        }
        mAudioManager.unregisterAudioDeviceCallback(mAudioManagerAudioDeviceCallback);
        mDeviceConfigListener = null;

        super.onDestroy();
    }

    @Override
    public IBinder onBind(Intent intent) {
        return null;
    }
    private void updateHdrSettings() {
        if (!DroidLogicUtils.isTv()) {
            if (mDisplayManager.getHdrConversionMode().getConversionMode() == HdrConversionMode.HDR_CONVERSION_PASSTHROUGH) {
                setHDRSettingspolicy("playback");
            } else {
                setHDRSettingspolicy("always");
            }
        }
    }

    private void resetDisplayConversionMode(){
         if (mDisplayManager.getHdrConversionMode().getConversionMode() != HdrConversionMode.HDR_CONVERSION_SYSTEM)
             return;

        int preferredHdrFormat = mDisplayManager.getHdrConversionMode().getPreferredHdrOutputType();
        Log.d(TAG, "now preferredHdrFormat = " + preferredHdrFormat);
        if (preferredHdrFormat != -1
                && !isHdrFormatSupported(mDisplayManager.getDisplay(Display.DEFAULT_DISPLAY).getMode(), preferredHdrFormat)) {
            HdrConversionMode systemHdrConversionMode = new HdrConversionMode(
                    HdrConversionMode.HDR_CONVERSION_SYSTEM);
            mDisplayManager.setHdrConversionMode(systemHdrConversionMode);
            Log.d(TAG, "reset HDR_CONVERSION_SYSTEM to right preferredHdrFormat");
        }
    }

    private boolean isHdrFormatSupported(Display.Mode mode, int hdrFormat) {
        return Arrays.stream(mode.getSupportedHdrTypes()).anyMatch(
        hdr -> hdr == hdrFormat);
   }

    private void initNrdpCapabilities() {
        String buildDate = PlatformAPI.getStringProperty("ro.build.version.incremental", "");
        boolean needUpdate = !buildDate.equals(SettingsPref.getSavedBuildDate(mContext));
        boolean audioNeedUpdate = false;
        if (needUpdate && hasMS12) {
            Settings.Global.putInt(mContext.getContentResolver(), DOLBY_LIB, 2);
        }
        int dolbyint = Settings.Global.getInt(mContext.getContentResolver(), DOLBY_LIB, 0);
        if ( hasMS12 && (dolbyint != 2)) {
            audioNeedUpdate =true;
            Settings.Global.putInt(mContext.getContentResolver(), DOLBY_LIB, 2);
        }
        if ( !hasMS12 && (dolbyint != 0)) {
            audioNeedUpdate =true;
            Settings.Global.putInt(mContext.getContentResolver(), DOLBY_LIB, 0);
        }
        setNrdpCapabilitiesIfNeed(NRDP_PLATFORM_CAP, needUpdate);
        setNrdpCapabilitiesIfNeed(NRDP_AUDIO_PLATFORM_CAP, needUpdate || audioNeedUpdate);
        if (needUpdate) {
            SettingsPref.setSavedBuildDate(mContext, buildDate);
        }
    }

    private void startNetflixIfNeed() {
        Scanner scanner = null;
        int reason = -1;
        boolean isSysExists = true;
        String wakeupSys = WAKEUP_REASON_DEVICE;

        if (new File(WAKEUP_REASON_DEVICE).exists()) {
            wakeupSys = WAKEUP_REASON_DEVICE;
        } else if (new File(WAKEUP_REASON_DEVICE_OTHER).exists()) {
            wakeupSys = WAKEUP_REASON_DEVICE_OTHER;
        } else {
            isSysExists = false;
        }

        if (isSysExists) {
            try {
                scanner = new Scanner(new File(wakeupSys));
                reason = scanner.nextInt();
                scanner.close();
            } catch (Exception e) {
                if (scanner != null)
                    scanner.close();
                e.printStackTrace();
                return;
            }

        }

        if (reason == WAKEUP_REASON_CUSTOM) {
            Log.i(TAG, "launchNetflix");
            launchNetflix();
        }
    }

    private void launchNetflixAtv() {
        Log.i(TAG, "launchNetflix atv from power on");
        Intent netflixIntent = new Intent();
        netflixIntent.setAction(NETFLIX_INTENT);
        netflixIntent.setPackage(NETFLIX_PKG_NAME);
        netflixIntent.addFlags(Intent.FLAG_ACTIVITY_NEW_TASK | Intent.FLAG_ACTIVITY_REORDER_TO_FRONT);
        netflixIntent.putExtra(NETFLIX_KEY_POWER_MODE, true); //true for powerOnFromNetflixButton
        mContext.startActivity(netflixIntent);
    }

    private void launchNetflix() {
        PackageManager packageManager = mContext.getPackageManager();
        if (packageManager.getLaunchIntentForPackage(NETFLIX_PKG_NAME) == null) {
            Log.e(TAG, "Cannot find intent for Netlix package: " + NETFLIX_PKG_NAME);
            return;
        }
        String globalButtonLaunch = mContext.getString(R.string.config_globalButtonLaunch);
        Log.d(TAG, " globalButtonLaunch component: " + globalButtonLaunch);

        Intent intent = new Intent(ACTION_LAUNCH_APP);
        intent.setComponent(ComponentName.unflattenFromString(globalButtonLaunch));
        intent.putExtra(EXTRA_PACKAGE_NAME, NETFLIX_PKG_NAME);
        intent.addFlags(Intent.FLAG_INCLUDE_STOPPED_PACKAGES | Intent.FLAG_RECEIVER_FOREGROUND);

        Intent launchIntent  = new Intent(NETFLIX_INTENT);
        launchIntent.setPackage(NETFLIX_PKG_NAME);
        launchIntent.putExtra(NETFLIX_KEY_POWER_MODE, true);
        launchIntent.addFlags(Intent.FLAG_ACTIVITY_NEW_TASK | Intent.FLAG_INCLUDE_STOPPED_PACKAGES
            | Intent.FLAG_RECEIVER_FOREGROUND | Intent.FLAG_ACTIVITY_REORDER_TO_FRONT);
        intent.putExtra(EXTRA_LAUNCH_INTENT, launchIntent);

        if (canIntentBeHandled(mContext, intent)) {
            Log.d(TAG, "launchNetflix gtv from power on ");
            mContext.sendBroadcast(intent);
        } else {
            launchNetflixAtv();
        }
    }

    private  boolean canIntentBeHandled(Context context, Intent intent) {
        List<ResolveInfo> receivers = context.getPackageManager().queryBroadcastReceivers(
            intent, PackageManager.MATCH_ALL);
        Log.d(TAG, "receivers " + receivers);
        if (receivers != null && receivers.size() > 0) {
            return true;
        }
        return false;
    }

    private void setNrdpCapabilitiesIfNeed(String capName, boolean needUpdate) {
        String cap = Settings.Global.getString(getContentResolver(), capName);
        String capName_File = capName;
        Log.i(TAG, capName + ":\n" + cap);
        if (!needUpdate && !TextUtils.isEmpty(cap)) {
            return;
        }

        if (capName.startsWith(NRDP_AUDIO_PLATFORM_CAP) && hasMS12 &&
            mDroidAudioManager.getDigitalAudioMode() == DroidAudioManager.DIGITAL_AUDIO_MODE_AUTO) {
            capName_File = NRDP_AUDIO_PLATFORM_CAP_MS12;
        }

        try {
            Scanner scanner = new Scanner(new File(NRDP_PLATFORM_CONFIG_DIR + capName_File + ".json"));
            StringBuilder sb = new StringBuilder();

            while (scanner.hasNextLine()) {
                sb.append(scanner.nextLine());
                sb.append('\n');
            }

            Settings.Global.putString(getContentResolver(), capName, sb.toString());
            scanner.close();
        } catch (java.io.FileNotFoundException e) {
            Log.d(TAG, e.getMessage());
        } catch (Exception e) {
            e.printStackTrace();
        }
    }

    public boolean isVisibleApp(String pkgName) {
        ActivityManager am = (ActivityManager) mContext.getSystemService(Context.ACTIVITY_SERVICE);
        List<ActivityManager.RunningAppProcessInfo> infos = am.getRunningAppProcesses();

        for (int i = 0; i < infos.size(); i++) {
            ActivityManager.RunningAppProcessInfo info = infos.get(i);
            if (info.processName.contains(pkgName)) {
                Log.d(TAG, "processName:" + info.processName + ",importance:" + info.importance);
                if (info.importance == ActivityManager.RunningAppProcessInfo.IMPORTANCE_FOREGROUND) return true;
                else {
                    return isTopActivity(pkgName);
                }
            }
        }

        return isTopActivity(pkgName);
    }

    private boolean isTopActivity(String pkgName){
        ActivityManager am = (ActivityManager) mContext.getSystemService(Context.ACTIVITY_SERVICE);
        List<ActivityManager.RunningTaskInfo> infos = null;

        try {
            infos = am.getRunningTasks(1);
        } catch (SecurityException e) {
            Log.d(TAG, "Failed to get running tasks. " + e.getMessage());
            return false;
        }

        if (infos == null || infos.isEmpty()) {
            Log.d(TAG,"No running tasks found.");
            return false;
        }

        String topActivityPackageName = infos.get(0).topActivity.getPackageName();

        if (pkgName.equals(topActivityPackageName)) {
            // Log.d(TAG, pkgName + " is top activity!");
            return true;
        }

        // Log.d(TAG, pkgName + " is not top activity.");
        return false;
    }

    private boolean isAudioDeviceConnected(int type) {
        AudioDeviceInfo[] outputDevices = mAudioManager.getDevices(AudioManager.GET_DEVICES_OUTPUTS);
        for (AudioDeviceInfo info : outputDevices) {
            if (info.isSink() && info.getType() == type) {
                Log.i(TAG, "AudioDeviceConnected: " + type + " true");
                return true;
            }
        }

        Log.i(TAG, "AudioDeviceConnected: " + type + " false");
        return false;
	}


    private boolean isTopTask(String pkgName){
        try {
             // return if the activity monitor is no longer used
            if (mIActivityManager == null) {
                 return false;
            }
            List<RootTaskInfo> infos = mIActivityManager.getAllRootTaskInfos();
            for (RootTaskInfo info : infos) {
                if (info == null) {
                    continue;
                }
                if (!info.visible) {
                    continue;
                }
                ComponentName componentInfo = info.topActivity;
                if (componentInfo == null) {
                    continue;
                }
                if (componentInfo.getPackageName().equals(pkgName)) {
                    Log.d(TAG, componentInfo.getPackageName() + " is top activity!");
                    return true;
                }else{
                    // Log.d(TAG,componentInfo.getPackageName() + " is visible.");
                    continue;
                }
            }
            // Log.d(TAG,pkgName + " is not top activity.");
        }catch (RemoteException e) {
            Log.e(TAG, "Cannot getTasks", e);
        }
        return false;
    }

    private void refreshAudioCapabilities(boolean init) {
        boolean isTv = DroidLogicUtils.isTv();
        boolean state;
        String hdmiEncodings;
        int surround = mDroidAudioManager.getDigitalAudioMode();
        Log.i(TAG, "refreshAudioCapabilities: " + ", isTv:" + isTv + ", surround:" +
                DroidAudioManager.digitalModeToString(surround) +
                "isSoundbar: " + DroidLogicUtils.isSoundbar());

        if (isTv) {
            int[] outputDevices = mDroidAudioManager.getOutputDevices();
            int outputDevice = DroidAudioManager.DROID_AUDIO_FORCE_USE_NONE;
            if (outputDevices != null && outputDevices.length > 0) {
                outputDevice = outputDevices[0];
                Log.i(TAG, "outputDevice " + outputDevice);
            }

            if (DroidAudioManager.DIGITAL_AUDIO_MODE_MANUAL == surround) {
                String subformat = Settings.Global.getString(mContext.getContentResolver(), DroidAudioManager.ENCODED_SURROUND_OUTPUT_ENABLED_FORMATS);
                Log.i(TAG, "onChange manual subformat: " + subformat);
                setAtmosEnabled(subformat.contains(AudioFormat.ENCODING_E_AC3_JOC + ""));
                setDdpEnabled(subformat.contains(AudioFormat.ENCODING_E_AC3 + ""));
            } else if (DroidAudioManager.DIGITAL_AUDIO_MODE_PCM == surround) {
                Log.i(TAG, "PCM Mode");
                if (outputDevice == DroidAudioManager.DROID_AUDIO_FORCE_USE_HDMI) {
                    // Disable DDP & ATOMS in PCM mode.
                    Log.i(TAG, "Arc/eArc ");
                    setDdpEnabled(false);
                    setAtmosEnabled(false);
                } else if (outputDevice == DroidAudioManager.DROID_AUDIO_FORCE_USE_SPEAKER) {
                    Log.i(TAG, "Speaker ");
                    setDdpEnabled(ddpSupportedByConfig);
                    setAtmosEnabled(atmosSupportedByConfig);
                } else {
                    Log.i(TAG, "not Arc/eArc/Speaker ");
                    setDdpEnabled(ddpSupportedByConfig);
                    setAtmosEnabled(false);
                }

            } else {
                if (outputDevice == DroidAudioManager.DROID_AUDIO_FORCE_USE_HDMI) {
                    // For arc/earc, After disconnecting arc, it need to be configured as the default value in the json file.
                    Log.i(TAG, "Arc/eArc ");
                    hdmiEncodings = mAudioManager.getParameters("hdmi_encodings");
                    setDdpEnabled(hdmiEncodings.contains("eac3"));
                    setAtmosEnabled(hdmiEncodings.contains("atmos"));
                } else if (outputDevice == DroidAudioManager.DROID_AUDIO_FORCE_USE_SPEAKER) {
                    Log.i(TAG, "Speaker ");
                    setDdpEnabled(ddpSupportedByConfig);
                    setAtmosEnabled(atmosSupportedByConfig);
                } else {
                    Log.i(TAG, "not Arc/eArc/Speaker ");
                    setDdpEnabled(ddpSupportedByConfig);
                    setAtmosEnabled(false);
                }
            }

            setUiAudioBufferDelayOffsetTv();
        } else if (DroidLogicUtils.isSoundbar()) {
            if (init) {
                setAtmosEnabled(true);
                Log.i(TAG,"Soundbar mode, set amtos enable");
            }
        } else {
            state = AudioSystem.DEVICE_STATE_AVAILABLE == AudioSystem.getDeviceConnectionState(AudioSystem.DEVICE_OUT_HDMI, "");

            Log.i(TAG, "DEVICE_OUT_HDMI state: " + state);

            hdmiEncodings = mAudioManager.getParameters("hdmi_encodings");

            if ((init || state) && (DroidAudioManager.DIGITAL_AUDIO_MODE_AUTO == surround
                || DroidAudioManager.DIGITAL_AUDIO_MODE_PASSTHROUGH == surround
                || DroidAudioManager.DIGITAL_AUDIO_MODE_MANUAL == surround)) {

                if (hasMS12 || SystemProperties.get("sys.vendor.atmos.passthrough").equals("enable")) {
                    if (DroidAudioManager.DIGITAL_AUDIO_MODE_MANUAL == surround) {
                        String subformat = mDroidAudioManager.getAudioManualFormats();
                        Log.i(TAG, "onChange manual subformat: " + subformat);
                        setAtmosEnabled(subformat.contains(AudioFormat.ENCODING_E_AC3_JOC + ""));
                    } else {
                        setAtmosEnabled(hdmiEncodings.contains("atmos"));
                    }
                } else {
                    setAtmosEnabled(false);
                }

                if (hasMS12) {
                    setUiAudioBufferDelayOffset(hdmiEncodings.contains("ac3"));
                }
            }
        }
    }


    private void setDdpEnabled(boolean enabled) {
        updateSettingsJsonObjectBoolean(NRDP_AUDIO_PLATFORM_CAP, "ddplus", "enabled", enabled);
    }


    private void setAtmosEnabled(boolean enabled) {
        updateSettingsJsonObjectBoolean(NRDP_AUDIO_PLATFORM_CAP, "atmos", "enabled", enabled);
    }

    private void updateSettingsJsonObjectBoolean(String nrdp_name, String objName, String attr, boolean enabled) {
        // Refer to /vendor/etc/nrdp_audio_platform_capabilities.json
        String audioCap = Settings.Global.getString(getContentResolver(), nrdp_name);
        if (audioCap == null)
            return;

        Log.i(TAG, "set "+ objName + " support " + enabled);

        try {
            JSONObject rootObject = new JSONObject(audioCap);
            JSONObject audioCapsObject = rootObject.getJSONObject("audiocaps");
            JSONObject object = audioCapsObject.getJSONObject(objName);

            boolean isEnabled = object.getBoolean(attr);
            if (isEnabled ^ enabled) {
                Log.i(TAG, "set " + objName + " support " + isEnabled + " -> " + enabled);
                object.put(attr, enabled);
                Settings.Global.putString(getContentResolver(), NRDP_AUDIO_PLATFORM_CAP, rootObject.toString());
            }
        } catch (org.json.JSONException e) {
            e.printStackTrace();
        }
    }

    private void setHDRSettingspolicy(String hdrOutputType) {
        // Refer to /vendor/etc/nrdp_platform_capabilities.json
        String platformCap = Settings.Global.getString(getContentResolver(), NRDP_PLATFORM_CAP);
        if (platformCap == null)
            return;

        try {
            JSONObject rootObject = new JSONObject(platformCap);
            Log.i(TAG, "set hdrOutputType to "+ hdrOutputType);
            rootObject.put("hdrOutputType", hdrOutputType);
            Settings.Global.putString(getContentResolver(), NRDP_PLATFORM_CAP, rootObject.toString());

        } catch (org.json.JSONException e) {
            e.printStackTrace();
        }
    }

    private void setUiAudioBufferDelayOffset(boolean isDolbySupported) {
        if (DroidLogicUtils.isTv()) {
            setUiAudioBufferDelayOffsetTv();
        }else{
            setUiAudioBufferDelayOffset(isDolbySupported ? UI_AUDIO_DELAY_OFFSET_OTT_DOLBY : UI_AUDIO_DELAY_OFFSET_OTT_PCM);
        }
    }


    private void setUiAudioBufferDelayOffset(int setOffset) {
        // Refer to /vendor/etc/nrdp_audio_platform_capabilities.json
        String audioCap = Settings.Global.getString(getContentResolver(), NRDP_AUDIO_PLATFORM_CAP);
        if (audioCap == null)
            return;

        try {
            JSONObject rootObject = new JSONObject(audioCap);
            JSONObject audioCapsObject = rootObject.getJSONObject("audiocaps");
            int uiOffset = audioCapsObject.getInt("uiAudioBufferDelayOffset");
            if (uiOffset != setOffset) {
                Log.i(TAG, "uiOffset from  " + uiOffset + "to " + setOffset);
                audioCapsObject.put("uiAudioBufferDelayOffset", setOffset);
                Settings.Global.putString(getContentResolver(), NRDP_AUDIO_PLATFORM_CAP, rootObject.toString());
            }
        } catch (org.json.JSONException e) {
            e.printStackTrace();
        }
    }

    private void setUiAudioBufferDelayOffsetTv() {
        setUiAudioBufferDelayOffset(hasMS12
		    && (isAudioDeviceConnected(AudioDeviceInfo.TYPE_HDMI_EARC) || isAudioDeviceConnected(AudioDeviceInfo.TYPE_HDMI_ARC))
			&& !isAudioDeviceConnected(AudioDeviceInfo.TYPE_WIRED_HEADPHONES) ?
            UI_AUDIO_DELAY_OFFSET_TV_MS12 : UI_AUDIO_DELAY_OFFSET_TV_NON_DOLBY);
	}

    private boolean isDdpConfiged() {
        String capName_File = NRDP_AUDIO_PLATFORM_CAP;
        if (hasMS12) {
            capName_File = NRDP_AUDIO_PLATFORM_CAP_MS12;
        }

        return getSettingsJsonObjectBoolean(capName_File, "ddplus", "enabled");
    }

    private boolean isAtmosConfiged() {
        String capName_File = NRDP_AUDIO_PLATFORM_CAP;
        if (hasMS12) {
            capName_File = NRDP_AUDIO_PLATFORM_CAP_MS12;
        }

        return getSettingsJsonObjectBoolean(capName_File, "atmos", "enabled");

   }

    private boolean getSettingsJsonObjectBoolean(String nrdp_name, String objName, String attr) {
        try {
            Log.i(TAG, "capName_File = " + nrdp_name);
            StringBuilder sb = new StringBuilder();
            Scanner scanner = new Scanner(new File(NRDP_PLATFORM_CONFIG_DIR + nrdp_name + ".json"));
            while (scanner.hasNextLine()) {
                sb.append(scanner.nextLine());
                sb.append('\n');
            }
            scanner.close();

            JSONObject rootObject = new JSONObject(sb.toString());
            JSONObject audioCapsObject = rootObject.getJSONObject("audiocaps");
            JSONObject object = audioCapsObject.getJSONObject(objName);

            return object.getBoolean(attr);
        } catch(java.io.FileNotFoundException e) {
            Log.d(TAG, e.getMessage());
        } catch(Exception e) {
            e.printStackTrace();
        }

        return false;
    }


    private void setAlwaysHDR(boolean NetflixIsForeground) {
        //when netflix is fg, enable always HDR whatever.
        if (tempHDR) {
           Log.i(TAG, "setHdrStrategy adaptive default");
           mSCM.setHdrStrategy(STR_ADAPTIVE);
           tempHDR = false;
        }
        if (NetflixIsForeground && mOutputModeManager.getHdrStrategy().startsWith(STR_ADAPTIVE) &&
                                mDisplayManager.getDisplay(Display.DEFAULT_DISPLAY).isHdr()) {
            Log.i(TAG, "setHdrStrategy  always");
            mSCM.setHdrStrategy(STR_ALWAYS);
            tempHDR = true;
        }
         Log.d(TAG,"NetflixIsForeground,startsWith,isHdr: "+NetflixIsForeground
                +mOutputModeManager.getHdrStrategy().startsWith(STR_ADAPTIVE)+mDisplayManager.getDisplay(Display.DEFAULT_DISPLAY).isHdr());
    }


    private void setHDRConversionMode(boolean NetflixIsForeground) {
        boolean isPassThroughHdr = mDisplayManager.getHdrConversionModeSetting().equals(new HdrConversionMode(
                            HdrConversionMode.HDR_CONVERSION_PASSTHROUGH));
        Log.d(TAG,"NetflixIsForeground = " + NetflixIsForeground
                +" ,isPassThroughHdr = " + isPassThroughHdr
                + ", is display support hdr = " + mDisplayManager.getDisplay(Display.DEFAULT_DISPLAY).isHdr());
        if (tempHDR && (!NetflixIsForeground)) {
            Log.i(TAG, "setHdrStrategy adaptive default");
            mHdrConversionMode =new HdrConversionMode(HdrConversionMode.HDR_CONVERSION_PASSTHROUGH);
            mDisplayManager.setHdrConversionMode(mHdrConversionMode);
            tempHDR = false;
            Settings.Global.putInt(mContext.getContentResolver(), TEMP_HDR, 0);
            return;
        }
        if (NetflixIsForeground && isPassThroughHdr &&
                                mDisplayManager.getDisplay(Display.DEFAULT_DISPLAY).isHdr()) {
            Log.i(TAG, "setHdrStrategy  always");
            mHdrConversionMode = new HdrConversionMode(HdrConversionMode.HDR_CONVERSION_SYSTEM);
            mDisplayManager.setHdrConversionMode(mHdrConversionMode);
            tempHDR = true;
            Settings.Global.putInt(mContext.getContentResolver(), TEMP_HDR, 1);
        }
    }

    private void resetHdrPolicy() {
        int hdrpolicy = Settings.Global.getInt(mContext.getContentResolver(), TEMP_HDR, 0);
        if (hdrpolicy == 1) {
            Log.i(TAG, "reset HdrStrategy adaptive default");
            mHdrConversionMode =new HdrConversionMode(HdrConversionMode.HDR_CONVERSION_PASSTHROUGH);
            mDisplayManager.setHdrConversionMode(mHdrConversionMode);
            Settings.Global.putInt(mContext.getContentResolver(), TEMP_HDR, 0);
        }
    }

    private void netflixFGStateUpdate() {
        synchronized (mLock) {
            boolean fg = isTopTask(NETFLIX_PKG_NAME);
            Log.i(TAG,"fg: "+fg + "  mIsNetflixFg: "+ mIsNetflixFg);
            if (fg ^ mIsNetflixFg) {
                Log.i(TAG, "Netflix status changed from " + (mIsNetflixFg ? "fg" : "bg") + " -> " + (fg ? "fg" : "bg"));
                mIsNetflixFg = fg;

                mAudioManager.setParameters("continuous_audio_mode=" + (fg ? "1" : "0"));
                mSCM.setProperty("vendor.netflix.state", fg ? "fg" : "bg");
                if (fg) {
                    mOriginalPowerStateChangeValue = mHdmiControlManager.getPowerStateChangeOnActiveSourceLost();
                    mHdmiControlManager.setPowerStateChangeOnActiveSourceLost(LOST_NONE);
                } else {
                    mHdmiControlManager.setPowerStateChangeOnActiveSourceLost(mOriginalPowerStateChangeValue);
                }
            }

            boolean fgYoutube = isTopTask(YOUTUBE_PKG_NAME);
            if (fgYoutube ^ mIsYoutubeFg) {
                Log.i(TAG, "Youtube status changed from " + (mIsYoutubeFg ? "fg" : "bg") + " -> " + (fgYoutube ? "fg" : "bg"));
                mIsYoutubeFg = fgYoutube;
                mAudioManager.setParameters("compensate_video_enable=" + (fgYoutube ? "1" : "0"));
            }

        }
    }

    private final TaskStackListener mTaskStackListener = new TaskStackListener() {
        @Override
        public void onTaskStackChanged() {
            Log.i(TAG, "onTaskStackChanged");
            mMsgHandler.sendEmptyMessageDelayed(MSG_UPDATA,700);
        }
    };


    private class ProcessObserver extends IProcessObserver.Stub {
        @Override
        public void onForegroundActivitiesChanged(int pid, int uid, boolean foregroundActivities) {
            Log.d(TAG, "onForegroundActivitiesChanged pid:" + pid + ",uid:" + uid + ",fg:" + foregroundActivities);
            mMsgHandler.sendEmptyMessageDelayed(MSG_UPDATA,700);
        }

        @Override
        public void onForegroundServicesChanged(int pid, int uid, int fgServiceTypes) {
            // Log.d(TAG, "onForegroundServicesChanged pid:" + pid);
        }

        @Override
        public void onProcessDied(int pid, int uid) {
        }
    }

}

