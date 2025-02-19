package com.droidlogic;

import android.app.ActivityManager;
import android.app.Service;
import android.content.BroadcastReceiver;
import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.pm.PackageManager;
import android.os.IBinder;
import android.os.SystemProperties;
import android.util.Log;
import java.util.List;

public class YouTubeService extends Service {
  private static final String TAG = "YouTubeService";

  private static String DEFAULT_PACKAGE_NAME = "dev.cobalt.coat";
  private static final String DEFAULT_ACTIVITY_NAME = "dev.cobalt.app.MainActivity";
  private final static String EXTRA_HDMI_PLUGGED_STATE = "state";
  private static final String ACTION_HDMI_PLUGGED = "android.intent.action.HDMI_PLUGGED";
  // update last HDMI plugged state
  private boolean lastHdmiPluggedState = false;
  private boolean isYoutubeStopped = false;
  private BroadcastReceiver mHdmiHotPlugReceiver = new BroadcastReceiver() {
    @Override
    public void onReceive(Context context, Intent intent) {
      String action = intent.getAction();
      Log.d(TAG, "action: " + action);
      if (action != null && action.equals(ACTION_HDMI_PLUGGED)) {
        boolean isHdmiPlugged = intent.getBooleanExtra("state", false);
        // lastHdmiPluggedState changed to avoid Duplicate message received
        if (isHdmiPlugged != lastHdmiPluggedState) {
          Log.d(TAG, "HDMI state changed: " + isHdmiPlugged);
          String packageNameFromSystem = SystemProperties.get("ro.product.youtube.package.name");
          if (!packageNameFromSystem.isEmpty()) {
            DEFAULT_PACKAGE_NAME = packageNameFromSystem;
          }
          Log.d(TAG, "DEFAULT_PACKAGE_NAME changed: " + DEFAULT_PACKAGE_NAME);
          if (isHdmiPlugged) {
            if (isYoutubeStopped) {
              resumeYouTube(context);
            }
          } else {
            stopYouTube(context);
          }
          // update last HDMI plugged state
          lastHdmiPluggedState = isHdmiPlugged;
        }
      }
    }
  };

  @Override
  public void onCreate() {
    super.onCreate();
    IntentFilter HDMIPlugFilter = new IntentFilter(ACTION_HDMI_PLUGGED);
    registerReceiver(mHdmiHotPlugReceiver, HDMIPlugFilter);
  }

  @Override
  public IBinder onBind(Intent intent) {
    return null;
  }

  @Override
  public void onDestroy() {
    unregisterReceiver(mHdmiHotPlugReceiver);
    super.onDestroy();
  }

  private void resumeYouTube(Context context) {
    if (isAppInstalled(context, DEFAULT_PACKAGE_NAME)) {
      Intent intent = new Intent();
      intent.setComponent(new ComponentName(DEFAULT_PACKAGE_NAME, DEFAULT_ACTIVITY_NAME));
      intent.addFlags(Intent.FLAG_ACTIVITY_REORDER_TO_FRONT);
      intent.addFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
      try {
        Log.d(TAG, "youtube stopped by YouTubeService, resumeYouTube");
        isYoutubeStopped = false;
        context.startActivity(intent);
      } catch (Exception e) {
        Log.e(TAG, "Failed to start YouTube activity: " + e.getMessage());
      }
    } else {
      Log.w(TAG, "YouTube app is not installed: " + DEFAULT_PACKAGE_NAME);
    }
  }

  private void stopYouTube(Context context) {
    ActivityManager activityManager =
        (ActivityManager) context.getSystemService(Context.ACTIVITY_SERVICE);
    if (activityManager != null) {
      List<ActivityManager.RunningTaskInfo> runningTasks = activityManager.getRunningTasks(1);
      if (!runningTasks.isEmpty()) {
        ActivityManager.RunningTaskInfo taskInfo = runningTasks.get(0);
        if (taskInfo.topActivity != null
            && DEFAULT_PACKAGE_NAME.equals(taskInfo.topActivity.getPackageName())
            && DEFAULT_ACTIVITY_NAME.equals(taskInfo.topActivity.getClassName())) {
          Log.d(TAG, "youtube on top, stopYouTube");
          // start Home to make cobalt activity stopped.
          Intent homeIntent = new Intent(Intent.ACTION_MAIN);
          homeIntent.addCategory(Intent.CATEGORY_HOME);
          homeIntent.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
          context.startActivity(homeIntent);
          isYoutubeStopped = true;
        }
      }
    }
  }

  private boolean isAppInstalled(Context context, String packageName) {
    try {
      context.getPackageManager().getApplicationInfo(packageName, 0);
      return true;
    } catch (PackageManager.NameNotFoundException e) {
      return false;
    }
  }
}
