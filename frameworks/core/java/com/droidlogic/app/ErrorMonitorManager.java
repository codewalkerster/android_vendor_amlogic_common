/*
**
** Copyright 2008, The Android Open Source Project
**
** Licensed under the Apache License, Version 2.0 (the "License");
** you may not use this file except in compliance with the License.
** You may obtain a copy of the License at
**
**     http://www.apache.org/licenses/LICENSE-2.0
**
** Unless required by applicable law or agreed to in writing, software
** distributed under the License is distributed on an "AS IS" BASIS,
** WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
** See the License for the specific language governing permissions and
** limitations under the License.
*/

package com.droidlogic.app;

import org.json.JSONArray;
import org.json.JSONObject;
import org.json.JSONException;
import java.util.NoSuchElementException;
import java.util.Arrays;
import java.util.List;
import java.util.ArrayList;
import java.util.List;
import java.util.HashSet;
import java.util.Map;
import java.util.HashMap;
import java.util.Set;
import java.util.Date;
import java.io.File;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.BufferedReader;
import java.io.InputStreamReader;
import java.io.InputStream;
import java.lang.reflect.Method;
import java.lang.ref.WeakReference;
import android.util.Log;
import android.content.Context;
import android.os.HwBinder;
import android.os.RemoteException;
import android.os.Build;
import android.os.DropBoxManager;
import android.os.Process;
import android.content.Intent;
import android.content.BroadcastReceiver;
import android.content.IntentFilter;
import android.content.SharedPreferences;

public class ErrorMonitorManager{
    private static final String TAG = "ErrorMonitorManager";
    private final static String SHARED_PREFERENCES_NAME = "ErrorMonitorManager";
    private final static String SHARED_PREFERENCES_ERROR_INFO_TAG = "ErrorInfo";
    private final static String SHARED_PREFERENCES_TIME_STAMP_TAG = "TimeStamp";
    private final static String SHARED_PREFERENCES_PSTORE_TIME_STAMP_TAG = "PstoreTimeStamp";
    private static final String KERNEL_PSTORE_TAG = "SYSTEM_LAST_KMSG";
    private static final String ANDROID_TOMBSTONE_TAG = "SYSTEM_TOMBSTONE";
    private static final String ANDROID_SYSTEM_APP_ANR_TAG = "system_app_anr";
    private static final String ANDROID_SYSTEM_SERVER_ANR_TAG = "system_server_anr";
    private static final String ANDROID_DATA_APP_ANR_TAG = "data_app_anr";
    private static final boolean DEBUG = false;
    private static ErrorMonitorManager mInstance = null;

    private static final int HANDLE_SUCCESS = 0;
    private static final int HANDLE_STATE_ERROR = -1;

    private static final int MSG_MODULE_ERROR = 0;
    private static final int MSG_SYSTEM_ERROR = 1;

    private static final int ERROR_MONITOR_DEATH_COOKIE = 1001;

    public static final int EERORMONITOR_MODULE_NAME_VIDEO     = 0;
    public static final int EERORMONITOR_MODULE_NAME_AUDIO     = 1;
    public static final int EERORMONITOR_MODULE_NAME_DISPLAY   = 2;
    public static final int EERORMONITOR_MODULE_NAME_ENCODE    = 3;
    public static final int EERORMONITOR_MODULE_NAME_DVB       = 4;
    public static final int EERORMONITOR_MODULE_NAME_SYSTEM    = 5;
    public static final int EERORMONITOR_MODULE_NAME_SUBTITLE  = 6;
    public static final int EERORMONITOR_MODULE_NAME_DEVICE    = 7;
    public static final int EERORMONITOR_MODULE_NAME_HDMI      = 8;

    public static final int EERORMONITOR_SUBMODULE_VIDEO_OMX               = 0;
    public static final int EERORMONITOR_SUBMODULE_VIDEO_CODE              = 1;
    public static final int EERORMONITOR_SUBMODULE_VIDEO_DRMPALYER         = 2;
    public static final int EERORMONITOR_SUBMODULE_VIDEO_DRMSERVICES       = 3;
    public static final int EERORMONITOR_SUBMODULE_VIDEO_CAS               = 4;
    public static final int EERORMONITOR_SUBMODULE_VIDEO_VEDC              = 5;
    public static final int EERORMONITOR_SUBMODULE_SYSTEM_KERNEL           = 6;
    public static final int EERORMONITOR_SUBMODULE_SYSTEM_ANDROID          = 7;
    public static final int EERORMONITOR_SUBMODULE_SYSTEMCONTROL           = 8;
    public static final int EERORMONITOR_SUBMODULE_AUDIOHAL                 = 9;
    public static final int EERORMONITOR_SUBMODULE_HWC                      = 10;
    public static final int EERORMONITOR_SUBMODULE_HDMITX                   = 11;
    public static final int EERORMONITOR_SUBMODULE_CVBS                     = 12;
    public static final int EERORMONITOR_SUBMODULE_MEDIAHAL                 = 13;
    public static final int EERORMONITOR_SUBMODULE_WIFI                     = 14;
    public static final int EERORMONITOR_SUBMODULE_NUPLAYER                 = 15;

    public static final int EERORMONITOR_LEVEL_SERIOUS      = 0;
    public static final int EERORMONITOR_LEVEL_NORMAL       = 1;
    public static final int EERORMONITOR_LEVEL_SLIGHT       = 2;

    public static final long SYSTEM_ERROR_EVENT_PSTORE              = 0x1;
    public static final long SYSTEM_ERROR_EVENT_TOMBSTONE           = 0x2;
    public static final long SYSTEM_ERROR_EVENT_SYSTEM_SERVER_ANR   = 0x4;
    public static final long SYSTEM_ERROR_EVENT_SYSTEM_APP_ANR      = 0x8;
    public static final long SYSTEM_ERROR_EVENT_DATA_APP_ANR        = 0x10;

    public static final long VIDEO_ERROR_EVENT_BLACKSCREEN        = 0x1;
    public static final long VIDEO_ERROR_EVENT_LAG                = 0x2;
    public static final long VIDEO_ERROR_EVENT_FLOWER_SCREEN      = 0x4;
    public static final long VIDEO_ERROR_EVENT_FREEZE             = 0x8;
    public static final long VIDEO_ERROR_EVENT_SIZE_ABNORMAL      = 0x10;
    public static final long VIDEO_ERROR_EVENT_CHANGE_CH_SLOW     = 0x20;
    public static final long VIDEO_ERROR_EVENT_ENTER_SCREENSAVER  = 0x40;

    public static final long AUDIO_ERROR_EVENT_NO_SOUND                 = 0x1;
    public static final long AUDIO_ERROR_EVENT_VOL_CONTROL_ABNORMAL     = 0x2;
    public static final long AUDIO_ERROR_EVENT_AV_NONSYNC               = 0x4;
    public static final long AUDIO_ERROR_EVENT_POP_SOUND                = 0x8;

    public static final long SUBTITLE_ERROR_EVENT_FREEZE            = 0x1;
    public static final long SUBTITLE_ERROR_EVENT_SHOW_ABNORMAL     = 0x2;

    public static final long DISPLAY_ERROR_EVENT_DISP_NO_OUTPUT            = 0x1;
    public static final long DISPLAY_ERROR_EVENT_DISP_SHOW_ABNORMAL        = 0x2;
    public static final long DISPLAY_ERROR_EVENT_DISP_BLACKOUT             = 0x4;
    public static final long DISPLAY_ERROR_EVENT_HDMI_SETTING_ABNORMAL     = 0x8;
    public static final long DISPLAY_ERROR_EVENT_HDMI_SHOW_ABNORMAL        = 0x10;
    public static final long DISPLAY_ERROR_EVENT_HDMI_CEC_ABNORMAL         = 0x20;
    public static final long DISPLAY_ERROR_EVENT_HDCP_ABNORMAL             = 0x40;
    public static final long DISPLAY_ERROR_EVENT_HDMI_ABNORMAL             = 0x80;
    public static final long DISPLAY_ERROR_EVENT_CVBS_OUTPUT_ABNORMAL     = 0x100;

    public static final long DEVICE_ERROR_EVENT_WIFI_LIST_DISAPPEAR         = 0x1;
    public static final long DEVICE_ERROR_EVENT_WIFI_HOTPOT_CONNECT_FAIL    = 0x2;
    public static final long DEVICE_ERROR_EVENT_BT_MATCH_FAIL               = 0x4;
    public static final long DEVICE_ERROR_EVENT_BT_LOOPBACK_CONNECT_FAIL    = 0x8;

    public static final int EERORMONITOR_LOG_TYPE_LOGCAT       = 0x01;
    public static final int EERORMONITOR_LOG_TYPE_BUGREPORT    = 0x02;
    public static final int EERORMONITOR_LOG_TYPE_RSV_USB      = 0x04;

    private Context mContext = null;
    private ErrorMonitorCallbackListener mErrorMonitorCbl = null;
    private DropBoxManager mDropbox = null;
    private SharedPreferences mSharedPreferences = null;
    private SharedPreferences.Editor mEditor = null;
    private boolean mMonitorStarted = false;
    private String mErrorMsg = null;

    private final Map<Integer, Config> ConfigMap = new HashMap<>();


    // Mutex for all mutable shared state.
    private final Object mLock = new Object();

    static {
        if (Process.is64Bit()) {
            Log.d(TAG,"is 64bit process");
            System.load("/vendor/lib64/liberrormonitor_jni.so");

        } else {
            Log.d(TAG,"is 32bit process");
            System.load("/vendor/lib/liberrormonitor_jni.so");
        }
    }
    public class ErrorInfo {
        private int mainModule;
        private int subModule;
        private int level;
        private int logType;
        private long events;
        private String msg;
        public ErrorInfo(int mm, int sm, int l, int lt, long et, String message) {
            mainModule = mm;
            subModule = sm;
            level  = l;
            logType = lt;
            events = et;
            msg = message;
        }
        public int getMainModule() {
            return mainModule;
        }
        public int getSubModule() {
            return subModule;
        }
        public int getLevel() {
            return level;
        }
        public int getLogType() {
            return logType;
        }
        public long getEvents() {
            return events;
        }
        public String getMsg() {
            return msg;
        }
        public void setMsg(String m) {
            msg = m;
        }
    }

    public class Config {
        private int module;
        private int level;
        public Config(int m, int l) {
            module = m;
            level  = l;
        }
        public Config() {}
        public void setModule(int m) {
            module = m;
        }
        public int getModule() {
            return module;
        }
        public void setLevel(int l) {
            level = l;
        }
        public int getLevel() {
            return level;
        }
    }

    public ErrorMonitorManager(Context context){
        mContext = context;
        if (mContext != null) {
            mSharedPreferences = mContext.getSharedPreferences(SHARED_PREFERENCES_NAME, Context.MODE_PRIVATE);
            if (mSharedPreferences != null)
                mEditor = mSharedPreferences.edit();
        }
        native_ConnectErrorMonitor();
        loadSystemError();
    }

    public static ErrorMonitorManager getInstance(Context context) {
        if (null == mInstance)
            mInstance = new ErrorMonitorManager(context);
        return mInstance;
    }

    public static interface ErrorMonitorCallbackListener {
        void onReport(ErrorInfo info);
        void onError(String msg);
    }

    private native void native_ConnectErrorMonitor();
    private native void native_StartReceiver(WeakReference<ErrorMonitorManager> wo);
    private native int native_startErrorMonitor(String Config);
    private native void native_stopErrorMonitor();
    private native String native_getMonitorConfig();
    private native void native_updateMonitorConfig(String Config);
    private native void native_setLogLevelConfig(String Config);
    private native String native_getLogLevelConfig();
    private native void native_notifyError(int subModule, int logType, int errorType, int level, String errorMsg);

    public void setErrorMonitorCallbackListener(ErrorMonitorCallbackListener l){
        synchronized(mLock) {
            mErrorMonitorCbl = l;
            if (mErrorMonitorCbl != null && mErrorMsg != null) {
                mErrorMonitorCbl.onError(mErrorMsg);
            }
            if (ConfigMap.isEmpty() == false && mErrorMonitorCbl != null)
                loadErrorReport();
        }

    }

    public void startErrorMonitor(String jsonConfig) throws IllegalStateException , RemoteException {
        synchronized(mLock) {
            if (mMonitorStarted) {
                throw new IllegalStateException("startErrorMonitor:the Monitor has been started !!!");
            }
            Log.d(TAG,"startErrorMonitor jsonConfig:" + jsonConfig);
            try {
                native_StartReceiver(new WeakReference<ErrorMonitorManager>(this));
                if (HANDLE_SUCCESS != native_startErrorMonitor(jsonConfig))
                    throw new RemoteException("An unexpected error occurred");
                mMonitorStarted = true;
                Log.d(TAG,"finish start monitor");
            } catch (Exception e) {
                Log.e(TAG, "startErrorMonitor: ErrorMonitorService is dead!:" + e);
                throw new RemoteException("An unexpected error occurred :" + e);
            }
            return;
        }

    }
    public void startErrorMonitor(List<Config> monitorConfig) throws RemoteException {
        Log.d(TAG,"startErrorMonitor show monitorConfig:");
        ConfigMap.clear();
        if (monitorConfig.isEmpty() == true)
            return;
        for (int i = 0; i < monitorConfig.size(); i++) {
            Config config = monitorConfig.get(i);
            ConfigMap.put(config.getModule(),config);
            Log.d(TAG,"index = " + i + ",module = " + config.module + ",level = "+ config.level);
        }
        if (ConfigMap.isEmpty() == false && mErrorMonitorCbl != null)
            loadErrorReport();
        String jsonString = ConfigList2JsonString(monitorConfig);
        startErrorMonitor(jsonString);
    }

    public void stopErrorMonitor() throws IllegalStateException , RemoteException {
        synchronized(mLock) {
            if (!mMonitorStarted) {
                throw new IllegalStateException("stopErrorMonitor:the Monitor has been stopped !!!");
            }
            try {
                native_stopErrorMonitor();
                mMonitorStarted = false;
            } catch (Exception e) {
                Log.e(TAG, "stopErrorMonitor: ErrorMonitorService is dead!:" + e);
                throw new RemoteException("An unexpected error occurred :" + e);
            }

        }
    }

    public String getMonConfString() throws IllegalStateException , RemoteException {
        synchronized(mLock) {
            if (!mMonitorStarted) {
                throw new IllegalStateException("getMonConfString:the Monitor has been stopped !!!");
            }
            try {
                String monitorConfig = native_getMonitorConfig();
                Log.d(TAG,"ErrorMonitor get monitorConfig :" + monitorConfig);
                if (monitorConfig != null && monitorConfig.isEmpty())
                    throw new RemoteException("No usable config was obtained");
                return monitorConfig;
            } catch (Exception e) {
                Log.e(TAG, "getMonConfString: ErrorMonitorService is dead!:" + e);
                throw new RemoteException("An unexpected error occurred :" + e);
            }
        }
    }

    public List<Config> getMonConfList() throws RemoteException {
        String jsonString = getMonConfString();
        List<Config> config;
        try {
            config = JsonString2ConfigList(jsonString);
        } catch (Exception e) {
            Log.e(TAG, "getMonConfList: convert json string to config list fail" + e);
            throw new RemoteException("An unexpected error occurred :" + e);
        }
        Log.d(TAG,"getMonConfList show config:");
        for (int i = 0; i < config.size(); i++) {
            Config con = config.get(i);
            Log.d(TAG,"index = " + i + ",module = " + con.module + ",level = "+ con.level);
        }
        return config;
    }

    public void updateMonitorConfig(String monitorConfig) throws IllegalStateException , RemoteException {
        synchronized(mLock) {
            if (!mMonitorStarted) {
                throw new IllegalStateException("updateMonitorConfig:the Monitor has been stopped !!!");
            }
            Log.d(TAG,"ErrorMonitor update monitorConfig:" + monitorConfig);
            try {
                native_updateMonitorConfig(monitorConfig);
            } catch (Exception e) {
                Log.e(TAG, "updateMonitorConfig: ErrorMonitorService is dead!:" + e);
                throw new RemoteException("An unexpected error occurred :" + e);
            }

        }
    }
    public void updateMonitorConfig(List<Config> monitorConfig) throws RemoteException {
        String jsonString = ConfigList2JsonString(monitorConfig);
        updateMonitorConfig(jsonString);
    }

    public void setLogLevel(String logLevelConfig) throws RemoteException {
        synchronized(mLock) {
            Log.d(TAG,"start to set loglevel logLevelConfig: "+ logLevelConfig);
            try {
                native_setLogLevelConfig(logLevelConfig);
            } catch (Exception e) {
                Log.e(TAG, "setLogLevel: ErrorMonitorService is dead!:" + e);
                throw new RemoteException("An unexpected error occurred :" + e);
            }

        }
    }

    public void setLogLevel(List<Config> logLevelConfig) throws RemoteException {
        String jsonString = ConfigList2JsonString(logLevelConfig);
        setLogLevel(jsonString);
    }

    public String getLogLevelString() throws RemoteException {
        synchronized(mLock){
            try {
                String config = native_getLogLevelConfig();
                Log.d(TAG,"getLogLevelString config: "+ config);
                return config;
            } catch (Exception e) {
                Log.e(TAG, "getLogLevelString: ErrorMonitorService is dead!:" + e);
                throw new RemoteException("An unexpected error occurred :" + e);
            }
        }
    }

    public List<Config> getLogLevelList() throws RemoteException {
        String jsonString = getLogLevelString();
        List<Config> config;
        try {
            config = JsonString2ConfigList(jsonString);
        } catch (Exception e) {
            Log.e(TAG, "getLogLevelList: convert json string to config list fail" + e);
            throw new RemoteException("An unexpected error occurred :" + e);
        }
        Log.d(TAG,"getLogLevelList show config:");
        for (int i = 0; i < config.size(); i++) {
            Config con = config.get(i);
            Log.d(TAG,"index = " + i + ",module = " + con.module + ",level = "+ con.level);
        }
        return config;
    }

    public void notifyError(int subModule, int logType, int errorType, int level, String errorMsg) throws RemoteException {
        synchronized(mLock) {
            Log.d(TAG,"notifyError : subModule =" + subModule + ",logType = " + logType + ",errorType ="+ errorType + ",level = " + level + ",errorMsg = " + errorMsg);
            try{
                native_notifyError(subModule, logType, errorType,level, errorMsg);
            }catch (Exception e) {
                Log.e(TAG, "fail to notify error info" + e);
                throw new RemoteException("An unexpected error occurred :" + e);
            }
        }
    }

    private void getErrorInfoFromDropBox(DropBoxManager dropbox, String atag, long startTime) {
        DropBoxManager.Entry entry;
        long timestamp = startTime;
        long pstore_timestamp = 0;
        ErrorInfo pstoreInfo = null;
        boolean isGetPstore = false;
        Log.i(TAG,"getErrorInfoFromDropBox startTime =" + startTime);
        while (null != (entry = dropbox.getNextEntry(atag, timestamp))) {
            isGetPstore = false;
            String tag = entry.getTag();
            timestamp = entry.getTimeMillis();
            Log.e(TAG, "getErrorInfoFromDropBox getInputStream tag =" + tag + ",timestamp =" + timestamp);
            if (tag.equals(KERNEL_PSTORE_TAG) || tag.equals(ANDROID_TOMBSTONE_TAG) ||
                tag.equals(ANDROID_SYSTEM_SERVER_ANR_TAG) ||tag.equals(ANDROID_SYSTEM_APP_ANR_TAG) ||
                tag.equals(ANDROID_DATA_APP_ANR_TAG)){
                try {
                    InputStream input = entry.getInputStream();
                    if (input !=  null) {
                        BufferedReader reader = new BufferedReader(new InputStreamReader(input));
                        String line;
                        int mainModule = EERORMONITOR_MODULE_NAME_SYSTEM;
                        int subModule = EERORMONITOR_SUBMODULE_SYSTEM_ANDROID;
                        int level = -1;
                        int logType = EERORMONITOR_LOG_TYPE_BUGREPORT;
                        long events = 0;
                        if (atag != null && atag.equals(KERNEL_PSTORE_TAG) && tag.equals(KERNEL_PSTORE_TAG) && (timestamp > pstore_timestamp)) {
                            isGetPstore = true;
                            pstore_timestamp = timestamp;
                            pstoreInfo = new ErrorInfo(mainModule,EERORMONITOR_SUBMODULE_SYSTEM_KERNEL,EERORMONITOR_LEVEL_SERIOUS,
                                        EERORMONITOR_LOG_TYPE_RSV_USB,SYSTEM_ERROR_EVENT_PSTORE,"null");
                        }
                        while ((line = reader.readLine()) != null) {
                            if ((isGetPstore == true) && (pstoreInfo != null) && line.contains("Kernel panic - not syncing:")) {
                                pstoreInfo.setMsg(line);
                            } else if (tag.equals(ANDROID_TOMBSTONE_TAG) &&line.contains(", name:")) {
                                level = EERORMONITOR_LEVEL_SERIOUS;
                                if (line.contains("subtitleserver")) {
                                    mainModule = EERORMONITOR_MODULE_NAME_SUBTITLE;
                                    events = SUBTITLE_ERROR_EVENT_FREEZE;
                                }else if (line.contains("com.google.android.katniss")) {
                                    mainModule = EERORMONITOR_MODULE_NAME_AUDIO;
                                    events = AUDIO_ERROR_EVENT_NO_SOUND;
                                }else if (line.contains("com.android.systemui")) {
                                    mainModule = EERORMONITOR_MODULE_NAME_DISPLAY;
                                    events = DISPLAY_ERROR_EVENT_DISP_BLACKOUT;
                                }else {
                                    events = SYSTEM_ERROR_EVENT_TOMBSTONE;
                                }
                            } else if ((tag.equals(ANDROID_SYSTEM_SERVER_ANR_TAG) || tag.equals(ANDROID_SYSTEM_APP_ANR_TAG) ||
                                                    tag.equals(ANDROID_DATA_APP_ANR_TAG)) && line.contains("----- pid")) {
                                String apkname;
                                while ((apkname = reader.readLine()) != null) {
                                    if (apkname.contains("Cmd line:"))
                                       break;
                                }
                                if (apkname.contains("com.google.android.tungsten.setupwraith")) {
                                    mainModule = EERORMONITOR_MODULE_NAME_DISPLAY;
                                    level = EERORMONITOR_LEVEL_SERIOUS;
                                    logType = EERORMONITOR_LOG_TYPE_BUGREPORT;
                                    events = DISPLAY_ERROR_EVENT_DISP_BLACKOUT;
                                } else if (tag.equals(ANDROID_SYSTEM_SERVER_ANR_TAG)) {
                                    level = EERORMONITOR_LEVEL_SERIOUS;
                                    events = SYSTEM_ERROR_EVENT_SYSTEM_SERVER_ANR;
                                } else if (tag.equals(ANDROID_SYSTEM_APP_ANR_TAG)) {
                                    level = EERORMONITOR_LEVEL_NORMAL;
                                    events = SYSTEM_ERROR_EVENT_SYSTEM_APP_ANR;
                                } else if (tag.equals(ANDROID_DATA_APP_ANR_TAG)) {
                                    level = EERORMONITOR_LEVEL_SLIGHT;
                                    events = SYSTEM_ERROR_EVENT_DATA_APP_ANR;
                                }
                                line = line + apkname;
                            }
                            if (subModule >= 0 && level >= 0 && events >= 0 && line != null && !line.isEmpty()) {
                                synchronized(mLock) {
                                    if (mErrorMonitorCbl != null && ConfigMap.isEmpty() == false) {
                                        Config conf = ConfigMap.getOrDefault(mainModule,null);
                                        if (conf != null && level <= conf.getLevel()) {
                                           Log.d(TAG,"get the error info from dropbox, mainModule = " + mainModule +
                                              ",subModule =" + subModule +
                                              ",level = " + level +
                                              ",logType ="+ logType +
                                              ",events = " + events +
                                              ",msg = " + line);
                                           mErrorMonitorCbl.onReport(new ErrorInfo(mainModule,subModule,level,logType,events,line));
                                        } else {
                                            addToDataBase(new ErrorInfo(mainModule, subModule,level,logType,events,line));
                                        }
                                    } else {
                                        addToDataBase(new ErrorInfo(mainModule, subModule,level,logType,events,line));
                                    }
                                }
                                break;
                            }
                        }
                        reader.close();
                        input.close();
                    }

                } catch (IOException e) {
                    e.printStackTrace();
                }
            }
            entry.close();

        }
        long NewestPstoreTime = mSharedPreferences.getLong(SHARED_PREFERENCES_PSTORE_TIME_STAMP_TAG, -1);
        Log.e(TAG, "the newest pstore time = " + NewestPstoreTime + ",pstore_timestamp =" + pstore_timestamp);
        if ((NewestPstoreTime != pstore_timestamp) && pstoreInfo != null) {
            mEditor.remove(SHARED_PREFERENCES_PSTORE_TIME_STAMP_TAG);
            mEditor.apply();
            mEditor.putLong(SHARED_PREFERENCES_PSTORE_TIME_STAMP_TAG,pstore_timestamp);
            mEditor.apply();
            if (mErrorMonitorCbl != null && ConfigMap.isEmpty() == false) {
                Config conf = ConfigMap.getOrDefault(pstoreInfo.getMainModule(),null);
                if (conf != null && pstoreInfo.getLevel() <= conf.getLevel()) {
                    mErrorMonitorCbl.onReport(pstoreInfo);
                } else {
                    addToDataBase(pstoreInfo);
                }
            } else {
                addToDataBase(pstoreInfo);
            }
        }
    }

    private final BroadcastReceiver mDropBoxEntryAddedReceiver = new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, Intent intent) {
            String tag = intent.getStringExtra(DropBoxManager.EXTRA_TAG);
            long time = intent.getLongExtra(DropBoxManager.EXTRA_TIME, 0);
            Log.e(TAG, "ErrorMonitorManager onReceive tag =" + tag + ",time =" + time);
            long startTime = mSharedPreferences.getLong(SHARED_PREFERENCES_TIME_STAMP_TAG, -1);
            if (startTime < 0) {
                startTime = System.currentTimeMillis();
            } else {
                mEditor.remove(SHARED_PREFERENCES_TIME_STAMP_TAG);
                mEditor.apply();
            }
            getErrorInfoFromDropBox(mDropbox,null, startTime);
            long timestamp = System.currentTimeMillis();
            Log.i(TAG, "push database time = " + timestamp);
            mEditor.putLong(SHARED_PREFERENCES_TIME_STAMP_TAG,timestamp);
            mEditor.apply();

        }
    };

    private void loadSystemError() {
        mDropbox = (DropBoxManager) mContext.getSystemService(Context.DROPBOX_SERVICE);
        long time = mSharedPreferences.getLong(SHARED_PREFERENCES_TIME_STAMP_TAG, -1);
        if (time < 0) {
            time = System.currentTimeMillis();
        } else {
            mEditor.remove(SHARED_PREFERENCES_TIME_STAMP_TAG);
            mEditor.apply();
        }
        getErrorInfoFromDropBox(mDropbox,null,time);
        getErrorInfoFromDropBox(mDropbox,KERNEL_PSTORE_TAG,0);

        long timestamp = System.currentTimeMillis();
        Log.i(TAG, "push database time = " + timestamp);
        mEditor.putLong(SHARED_PREFERENCES_TIME_STAMP_TAG,timestamp);
        mEditor.apply();
        final IntentFilter intentFilter = new IntentFilter();
        intentFilter.addAction(DropBoxManager.ACTION_DROPBOX_ENTRY_ADDED);
        mContext.registerReceiver(mDropBoxEntryAddedReceiver, intentFilter);
    }

    private String ConfigList2JsonString(List<Config> configList) {
        JSONArray jsonArray = new JSONArray();
        try {
            for (Config config : configList) {
                JSONObject jsonObject = new JSONObject();
                jsonObject.put("module", config.module);
                jsonObject.put("level", config.level);
                jsonArray.put(jsonObject);
            }
        } catch (JSONException e) {
            e.printStackTrace();
        }
        return jsonArray.toString();
    }

    private List<Config> JsonString2ConfigList(String jsonString) {
        List<Config> configList = new ArrayList<>();
        try {
            JSONArray jsonArray = new JSONArray(jsonString);
            for (int i = 0; i < jsonArray.length(); i++) {
                JSONObject jsonObject = jsonArray.getJSONObject(i);
                int module = jsonObject.getInt("module");
                int level = jsonObject.getInt("level");
                Config config = new Config(module, level);
                configList.add(config);
            }
        } catch (JSONException e) {
            e.printStackTrace();
        }
        return configList;

    }

    private ErrorInfo JsonString2ErrorInfo(String jsonString) {
        try {
            JSONObject jsonObject = new JSONObject(jsonString);
            int mainModule = jsonObject.getInt("mainModule");
            int subModule = jsonObject.getInt("subModule");
            int level = jsonObject.getInt("level");
            int logType = jsonObject.getInt("logType");
            long events = jsonObject.getLong("events");
            String msg = jsonObject.getString("msg");
            if (subModule < 0 || level < 0 || logType < 0 || events < 0 || msg == null || msg.isEmpty()) {
               Log.e(TAG,"the info is not legal subModule:" + subModule + ",level:" + level + ",logType:" + logType + ",events =" + events);
               return null;
            }
            return new ErrorInfo(EERORMONITOR_MODULE_NAME_SYSTEM, subModule, level, logType, events, msg);
        } catch (JSONException e) {
            e.printStackTrace();
            return null;
        }
    }

    private String ErrorInfo2JsonString(ErrorInfo info) {
        try {
            JSONObject jsonObject = new JSONObject();
            jsonObject.put("mainModule", info.getMainModule());
            jsonObject.put("subModule", info.getSubModule());
            jsonObject.put("level", info.getLevel());
            jsonObject.put("logType", info.getLogType());
            jsonObject.put("events", info.getEvents());
            jsonObject.put("msg", info.getMsg());
            return jsonObject.toString();
        } catch (JSONException e) {
            e.printStackTrace();
            return null;
        }
    }

    private void loadErrorReport() {
        if (mErrorMonitorCbl != null && mSharedPreferences != null && mEditor != null) {
            Set<String> infoSet = mSharedPreferences.getStringSet(SHARED_PREFERENCES_ERROR_INFO_TAG,null);
            if (infoSet != null && infoSet.size() > 0) {
                for (String i : infoSet) {
                    ErrorInfo info = JsonString2ErrorInfo(i);
                    Config conf = ConfigMap.getOrDefault(info.getMainModule(),null);
                    if (conf != null && info.getLevel() <= conf.getLevel()) {
                        Log.d(TAG,"get the error info form database subModule =" + info.getSubModule() + ",getLogType = " +
                                        info.getLogType() + ",events ="+ info.getEvents()+",level = " + info.getLevel() + ",msg = " + info.getMsg());
                        mErrorMonitorCbl.onReport(new ErrorInfo(EERORMONITOR_MODULE_NAME_SYSTEM, info.getSubModule(), info.getLevel(),
                                                            info.getLogType(),info.getEvents(), info.getMsg()));
                    }
                }
                mEditor.remove(SHARED_PREFERENCES_ERROR_INFO_TAG);
                mEditor.apply();
            }
        }
    }

    private void addToDataBase(ErrorInfo info) {
        if (mSharedPreferences == null || mEditor == null)
            return;
        String infoString = ErrorInfo2JsonString(info);
        if (infoString == null || infoString.isEmpty())
            return;
        Set<String> infoSet = mSharedPreferences.getStringSet(SHARED_PREFERENCES_ERROR_INFO_TAG,null);
        if (infoSet == null) {
            infoSet = new HashSet<>();
        }
        infoSet.add(infoString);
        mEditor.remove(SHARED_PREFERENCES_ERROR_INFO_TAG);
        mEditor.apply();
        mEditor.putStringSet(SHARED_PREFERENCES_ERROR_INFO_TAG,infoSet);
        mEditor.apply();
    }
    private int onMessage(int callbackType, int mainModule, int subModule,
                            int level,int logType,long events,String msg) {
        Log.i(TAG, "onMessage type=" + callbackType + ",mainModule ="+ mainModule +",subModule="+subModule +
                            ",level=" + level + ",logType="+logType + ",events=" + events +",msg="+msg);
        if (mErrorMonitorCbl == null) {
            if (callbackType == MSG_SYSTEM_ERROR) {
                mErrorMsg = msg;
            }
            return 0;
        }
        if (callbackType == MSG_MODULE_ERROR) {
            mErrorMonitorCbl.onReport(new ErrorInfo(mainModule,subModule, level, logType, events, msg));
        }else if (callbackType == MSG_SYSTEM_ERROR) {
            mErrorMonitorCbl.onError(msg);
        }
        return 0;

    }

    static int native_proc(Object o, int callbackType,int mainModule, int subModule,
                           int level,int logType,long events, String msg){
        WeakReference<ErrorMonitorManager> wo;
        ErrorMonitorManager f;
        if (o == null)
          return 0;
        try {
          wo = (WeakReference<ErrorMonitorManager>) o;
          f = wo.get();
          if (f == null )
            return 0;
          return f.onMessage(callbackType, mainModule, subModule, level, logType, events, msg);
        }catch (Throwable e) {
          e.printStackTrace();
          return 0;
        }

    }


    final class DeathRecipient implements HwBinder.DeathRecipient {
        DeathRecipient() {
        }

        @Override
        public void serviceDied(long cookie) {
            if (ERROR_MONITOR_DEATH_COOKIE == cookie) {
                Log.e(TAG, "errormonitor service died cookie: " + cookie);
            }
        }
    }

}
