/*
 * Copyright (c) 2014 Amlogic, Inc. All rights reserved.
 *
 * This source code is subject to the terms and conditions defined in the
 * file 'LICENSE' which is part of this source code package.
 *
 * Description:
 *     AMLOGIC ShutdownService
 */

package com.droidlogic.mdnsoffload;

import android.os.IInterface;
import android.os.RemoteException;
import android.util.Log;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

import device.google.atv.mdns_offload.IMdnsOffload;

import android.app.Service;
import android.content.Intent;
import android.os.IBinder;

import device.google.atv.mdns_offload.IMdnsOffload.MdnsProtocolData;
import device.google.atv.mdns_offload.IMdnsOffload.MdnsProtocolData.MatchCriteria;
import device.google.atv.mdns_offload.IMdnsOffload.PassthroughBehavior;
import device.google.atv.mdns_offload.IMdnsOffloadManager;

import java.util.function.Supplier;
import vendor.amlogic.hardware.droidmdnsoffload.IDroidMdnsOffload;
import android.os.Binder;
import android.os.ServiceManager;
import com.android.internal.annotations.GuardedBy;

import android.os.RemoteException;

public class DroidMdnsOffloadService extends Service {

    private static String TAG = "DroidMdnsOffloadService";

    @Nullable
    private Supplier<IDroidMdnsOffload> mService_mdnsoffload;

    @Override
    public void onCreate() {
        Log.d(TAG, "onCreate");
        initMdnsService(new VintfHalCache());
        super.onCreate();
    }

    @Override
    public void onDestroy() {
        Log.d(TAG, "onDestroy");
        super.onDestroy();
    }

    @Override
    public IBinder onBind(Intent intent) {
        Log.d(TAG, "onBind:" + intent);
        return mMdnsOffloadBinder;
    }

    private void initMdnsService(Supplier<IDroidMdnsOffload> service) {
        mService_mdnsoffload = service.get() != null ? service : null;
    }

    private final IMdnsOffload.Stub mMdnsOffloadBinder = new IMdnsOffload.Stub() {
        @Override
        public boolean setOffloadState(boolean enabled){
            Log.d(TAG, "setOffloadState:" + enabled);
            boolean ret = false;
            try {
                 if (mService_mdnsoffload != null) {
                    Log.d(TAG, "mService_mdnsoffload is not null");
                    ret = mService_mdnsoffload.get().setOffloadState(enabled);
                 } else
                    Log.d(TAG, "mService_mdnsoffload is null");
            } catch (RemoteException ex) {
                 Log.e(TAG, "Failed setOffloadState", ex);
            }
            Log.d(TAG, " ret:" + ret);
            return ret;
        }
        @Override
        public void resetAll(){
            Log.d(TAG, "resetAll");
            try {
                    if (mService_mdnsoffload != null) {
                        Log.d(TAG, "mService_mdnsoffload is not null");
                        mService_mdnsoffload.get().resetAll();
                    } else
                        Log.d(TAG, "mService_mdnsoffload is null");

            } catch (RemoteException ex) {
                 Log.e(TAG, "Failed resetAll", ex);
            }
        }
        @Override
        public int addProtocolResponses(String networkInterface,
                        MdnsProtocolData offloadData) {
            Log.d(TAG, "addProtocolResponses:" + networkInterface);
            if (offloadData == null) {
                Log.d(TAG, "offloadData is null!");
                return -1;
            }
            int size_data = offloadData.matchCriteriaList.size();
            int[] type = new int[size_data];
            int[] name_offset = new int[size_data];
            for (int i = 0; i < size_data; i++) {
                MatchCriteria criteria = offloadData.matchCriteriaList.get(i);
                type[i] = criteria.type;
                name_offset[i] = criteria.nameOffset;
            }
            int ret = -1;
            try {
                    if (mService_mdnsoffload != null) {
                        Log.d(TAG, "mService_mdnsoffload is not null");
                        ret = mService_mdnsoffload.get().addProtocolResponses(networkInterface, offloadData.rawOffloadPacket, type, name_offset);
                    } else
                        Log.d(TAG, "mService_mdnsoffload is null");

            } catch (RemoteException ex) {
                 Log.e(TAG, "Failed resetAll", ex);
            }

            Log.d(TAG, " ret:" + ret);
            return ret;
        }
        @Override
        public void removeProtocolResponses(int recordKey){
            Log.d(TAG, "removeProtocolResponses:" + recordKey);
            try {
                    if (mService_mdnsoffload != null) {
                        Log.d(TAG, "mService_mdnsoffload is not null");
                        mService_mdnsoffload.get().removeProtocolResponses(recordKey);
                    } else
                        Log.d(TAG, "mService_mdnsoffload is null");
            } catch (RemoteException ex) {
                 Log.e(TAG, "Failed removeProtocolResponses", ex);
            }
        }

        @Override
        public int getAndResetHitCounter(int recordKey){
            Log.d(TAG, "getAndResetHitCounter:" + recordKey);
            int ret = 0;
            try {
                    if (mService_mdnsoffload != null) {
                        Log.d(TAG, "mService_mdnsoffload is not null");
                        ret = mService_mdnsoffload.get().getAndResetHitCounter(recordKey);
                    } else
                        Log.d(TAG, "mService_mdnsoffload is null");
            } catch (RemoteException ex) {
                 Log.e(TAG, "Failed getAndResetHitCounter", ex);
            }
            Log.d(TAG, "  ret:" + ret);
            return ret;//-1;
        }
         @Override
        public int getAndResetMissCounter(){
            Log.d(TAG, "getAndResetMissCounter");
            int ret = 0;
            try {
                    if (mService_mdnsoffload != null) {
                        Log.d(TAG, "mService_mdnsoffload is not null");
                        ret = mService_mdnsoffload.get().getAndResetMissCounter();
                    } else
                        Log.d(TAG, "mService_mdnsoffload is null");
            } catch (RemoteException ex) {
                 Log.e(TAG, "Failed getAndResetHitCounter", ex);
            }
            Log.d(TAG, "  ret:" + ret);
            return ret;//-1;
        }

        @Override
        public boolean addToPassthroughList(String networkInterface, String qname){
            Log.d(TAG, "addToPassthroughList:" + networkInterface + "," + qname);
            boolean ret = false;
            try {
                    if (mService_mdnsoffload != null) {
                        Log.d(TAG, "mService_mdnsoffload is not null");
                        ret = mService_mdnsoffload.get().addToPassthroughList(networkInterface, qname);
                    } else
                        Log.d(TAG, "mService_mdnsoffload is null");

            } catch (RemoteException ex) {
                 Log.e(TAG, "Failed resetAll", ex);
            }

            Log.d(TAG, "  ret:" + ret);
            return ret;
        }

        @Override
        public void removeFromPassthroughList(String networkInterface, String qname){
            Log.d(TAG, "removeFromPassthroughList:" + networkInterface + "," + qname);
            try {
                    if (mService_mdnsoffload != null) {
                        Log.d(TAG, "mService_mdnsoffload is not null");
                        mService_mdnsoffload.get().removeFromPassthroughList(networkInterface, qname);
                    } else
                        Log.d(TAG, "mService_mdnsoffload is null");
            } catch (RemoteException ex) {
                 Log.e(TAG, "Failed removeFromPassthroughList", ex);
            }
        }

         @Override
        public void setPassthroughBehavior(String networkInterface,
                    byte behavior) {
            Log.d(TAG, "setPassthroughBehavior:" + networkInterface);
            try {
                    if (mService_mdnsoffload != null) {
                        Log.d(TAG, "mService_mdnsoffload is not null");
                        mService_mdnsoffload.get().setPassthroughBehavior(networkInterface, behavior);
                    } else
                        Log.d(TAG, "mService_mdnsoffload is null");
            } catch (RemoteException ex) {
                 Log.e(TAG, "Failed removeFromPassthroughList", ex);
            }
        }

         @Override
         public String getInterfaceHash() throws RemoteException {
             String ret = "";
            try {
                    if (mService_mdnsoffload != null) {
                        Log.d(TAG, "mService_mdnsoffload is not null");
                        ret = mService_mdnsoffload.get().getInterfaceHash();
                    } else
                        Log.d(TAG, "mService_mdnsoffload is null");
            } catch (RemoteException ex) {
                 Log.e(TAG, "Failed removeFromPassthroughList", ex);
            }
            Log.d(TAG, "getInterfaceHash:" + ret);
             return ret;//null;
         }

         @Override
         public int getInterfaceVersion() throws RemoteException {
             int ret = 0;
             try {
                    if (mService_mdnsoffload != null) {
                        Log.d(TAG, "mService_mdnsoffload is not null");
                        ret = mService_mdnsoffload.get().getInterfaceVersion();
                    } else
                        Log.d(TAG, "mService_mdnsoffload is null");
            } catch (RemoteException ex) {
                 Log.e(TAG, "Failed removeFromPassthroughList", ex);
            }
            Log.d(TAG, "getInterfaceVersion:" + ret);
            return ret;//0;
         }
    };

    private static class VintfHalCache implements Supplier<IDroidMdnsOffload>, IBinder.DeathRecipient {
        @GuardedBy("this")
        private IDroidMdnsOffload mInstance = null;

        @Override
        public synchronized IDroidMdnsOffload get() {
            if (mInstance == null) {
                IBinder binder = Binder.allowBlocking(
                        ServiceManager.waitForDeclaredService(IDroidMdnsOffload.DESCRIPTOR + "/default"));
                if (binder != null) {
                    Log.d(TAG, "binder is not null");
                    mInstance = IDroidMdnsOffload.Stub.asInterface(binder);
                    try {
                        binder.linkToDeath(this, 0);
                    } catch (RemoteException e) {
                        Log.e(TAG, "Unable to register DeathRecipient for " + mInstance);
                    }
                } else
                    Log.d(TAG, "binder is null");
            }
            return mInstance;
        }

        @Override
        public synchronized void binderDied() {
            Log.d(TAG, "binderDied");
            mInstance = null;
        }
    }
}
