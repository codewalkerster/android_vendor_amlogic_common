/*
 * Copyright (c) 2014 Amlogic, Inc. All rights reserved.
 *
 * This source code is subject to the terms and conditions defined in the
 * file 'LICENSE' which is part of this source code package.
 *
 * Description: JAVA file
 */

package com.droidlogic.audioservice.settings;

import java.io.IOException;
import java.io.BufferedReader;
import java.io.BufferedWriter;
import java.io.File;
import java.io.FileReader;
import java.io.FileWriter;
import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.lang.ref.WeakReference;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.Calendar;
import java.util.Map;
import java.util.NoSuchElementException;
import java.util.HashMap;
import java.util.List;
import java.util.StringTokenizer;
import java.util.TimeZone;
import java.util.regex.Matcher;
import java.util.regex.Pattern;

import android.content.Context;
import android.graphics.ImageFormat;
import android.graphics.Bitmap;
import android.graphics.Matrix;
import android.media.tv.TvContract;
import android.os.Build;
import android.os.Handler;
import android.os.HwBinder;
import android.os.IBinder;
import android.os.Looper;
import android.os.Message;
import android.os.Parcel;
import android.os.RemoteException;
import android.os.SystemProperties;
import android.text.TextUtils;

import android.util.Log;
import android.view.View;
import android.view.Surface;
import android.view.SurfaceHolder;
import java.lang.reflect.Method;

//import static com.droidlogic.app.tv.TvControlCommand.*;
//import com.droidlogic.app.tv.EasEvent;

import vendor.amlogic.hardware.tvserver.V1_0.ITvServer;
import vendor.amlogic.hardware.tvserver.V1_0.ITvServerCallback;
import vendor.amlogic.hardware.tvserver.V1_0.SignalInfo;
import vendor.amlogic.hardware.tvserver.V1_0.FormatInfo;
import vendor.amlogic.hardware.tvserver.V1_0.TvHidlParcel;
import vendor.amlogic.hardware.tvserver.V1_0.ConnectType;
import vendor.amlogic.hardware.tvserver.V1_0.Result;
import vendor.amlogic.hardware.tvserver.V1_0.FreqList;
import vendor.amlogic.hardware.tvserver.V1_0.RRTSearchInfo;


public class TvControlManager {
    private static final String TAG = "A.TvControlManager";
    private static final String OPEN_TV_LOG_FLG = "open.libtv.log.flg";


    private long mNativeContext; // accessed by native methods
    private EventHandler mEventHandler;
    private AudioEventListener mAudioListener = null;
    private boolean tvLogFlg = false;
    private static TvControlManager mInstance;
    private final int AUDIO_EVENT_CALLBACK = 550;

    public interface AudioEventListener {
        void HandleAudioEvent(int cmd, int param1, int param2);
    }

    public void SetAudioEventListener(AudioEventListener l) {
        libtv_log_open();
        mAudioListener  = l;
    }
    private void libtv_log_open(){
        //if (tvLogFlg) {
        StackTraceElement traceElement = ((new Exception()).getStackTrace())[1];
        Log.i(TAG, traceElement.getMethodName());
        //}
    }
    class EventHandler extends Handler {
        int dataArray[];
        int cmdArray[];
        int msgPdu[];

        public EventHandler(Looper looper) {
            super(looper);
            dataArray = new int[512];//max data buf
            cmdArray = new int[128];
            msgPdu = new int[1200];
        }

        @Override
        public void handleMessage(Message msg) {
            int i = 0, loop_count = 0, tmp_val = 0;
            TvHidlParcel parcel = ((TvHidlParcel) (msg.obj));
            switch (msg.what) {
                case AUDIO_EVENT_CALLBACK:
                    Log.i(TAG,"get AUDIO_EVENT_CALLBACK");
                    if (mAudioListener != null) {
                        int cmd = parcel.bodyInt.get(0);
                        int param1 = parcel.bodyInt.get(1);
                        int param2 = parcel.bodyInt.get(2);

                        Log.d(TAG, "tvinput cmd:"+cmd);
                        Log.d(TAG, "tvinput param1:"+param1);
                        Log.d(TAG, "tvinput param2:"+param2);
                        mAudioListener.HandleAudioEvent(cmd, param1, param2);
                    }
                    break;
                default:
                    Log.e(TAG, "Unknown message type " + msg.what);
                    break;
            }
        }
    }

    public static TvControlManager getInstance() {
        if (null == mInstance) mInstance = new TvControlManager();
        return mInstance;
    }

    public TvControlManager() {
        Looper looper = Looper.myLooper();
        if (looper != null) {
            mEventHandler = new EventHandler(looper);
        } else if ((looper = Looper.getMainLooper()) != null) {
            mEventHandler = new EventHandler(looper);
        } else {
            mEventHandler = null;
            Log.e(TAG, "looper is null, so can not do anything");
        }
        mHALCallback = new HALCallback(this);
        connectToProxy();
        String LogFlg = TvMiscConfigGet(OPEN_TV_LOG_FLG, "");
        if ("log_open".equals(LogFlg))
            tvLogFlg =true;
    }

    public String TvMiscConfigGet(String key_str, String def_str) {
        synchronized (mLock) {
            try {
                return mProxy.getMiscCfg(key_str, def_str);
            } catch (RemoteException e) {
                Log.e(TAG, "TvMiscConfigGet:" + e);
            }
        }
        return "";
    }

    private static class HALCallback extends ITvServerCallback.Stub {
        TvControlManager tvCtrlMgr;
        HALCallback(TvControlManager tcm) {
            tvCtrlMgr = tcm;
        }

        public void notifyCallback(TvHidlParcel parcel) {
            Log.i(TAG, "notifyCallback msg type:" + parcel.msgType);

            if (tvCtrlMgr.mEventHandler != null) {
                Message msg = tvCtrlMgr.mEventHandler.obtainMessage(parcel.msgType, 0, 0, parcel);
                tvCtrlMgr.mEventHandler.sendMessage(msg);
            }
        }
    }
    private static final int TVSERVER_DEATH_COOKIE = 1000;

    // Callback when the UsbPort status is changed by the kernel.
    // Mostly due a command sent by the remote Usb device.
    private HALCallback mHALCallback;

    // Notification object used to listen to the start of the tvserver daemon.
    //private final ServiceNotification mServiceNotification = new ServiceNotification();

    private ITvServer mProxy = null;
    // Mutex for all mutable shared state.
    private final Object mLock = new Object();

    private void connectToProxy() {
        synchronized (mLock) {
            if (mProxy != null) {
                return;
            }

            try {
                mProxy = ITvServer.getService();
                mProxy.linkToDeath(new DeathRecipient(), TVSERVER_DEATH_COOKIE);
                mProxy.setCallback(mHALCallback, ConnectType.TYPE_EXTEND);
            } catch (NoSuchElementException e) {
                Log.e(TAG, "connectToProxy: tvserver HIDL service not found."
                        + " Did the service fail to start?", e);
            } catch (RemoteException e) {
                Log.e(TAG, "connectToProxy: tvserver HIDL service not responding", e);
            }
        }
        Log.i(TAG, "connect to tvserver HIDL service success");
    }

    final class DeathRecipient implements HwBinder.DeathRecipient {
        DeathRecipient() {
        }

        @Override
        public void serviceDied(long cookie) {
            if (TVSERVER_DEATH_COOKIE == cookie) {
                Log.e(TAG, "tvserver HIDL service died cookie: " + cookie);
                synchronized (mLock) {
                    mProxy = null;
                }
            }
        }
    }
}
