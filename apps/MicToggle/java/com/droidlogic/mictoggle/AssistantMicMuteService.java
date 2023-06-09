/*
 * Copyright (c) 2014 Amlogic, Inc. All rights reserved.
 *
 * This source code is subject to the terms and conditions defined in the
 * file 'LICENSE' which is part of this source code package.
 *
 * Description:
 *     AMLOGIC ShutdownService
 */

package com.droidlogic.mictoggle;

import android.app.Service;
import android.content.ContentProvider;
import android.content.ContentValues;
import android.content.Context;
import android.content.UriMatcher;
import android.database.Cursor;
import android.database.sqlite.SQLiteDatabase;
import android.net.Uri;

import android.util.Log;
import android.content.pm.ApplicationInfo;
import android.content.pm.PackageManager;
import android.text.TextUtils;

import android.database.sqlite.SQLiteOpenHelper;
import com.droidlogic.app.DroidLogicUtils;
import com.droidlogic.app.SystemControlManager;

import android.content.Intent;
import android.content.IntentFilter;
import android.media.AudioManager;
import android.content.BroadcastReceiver;
import android.content.ContentResolver;
import android.os.IBinder;

public class AssistantMicMuteService extends Service {

    private static final String TAG = "AssistantMicMuteService";
    private Context mContext;

    /*private static final String HOTWORDMIC_AUTH = "atv.hotwordmic";
    //table name
    private static final String TOGGLESTATE = "togglestate";
    public static final Uri HOTWORDMIC_URI = new Uri.Builder().scheme("content")
                                                  .authority(HOTWORDMIC_AUTH)
                                                  .appendPath(TOGGLESTATE)
                                                  .build();
    //colume name
    public static final String COLUME_ID = "state" ;*/

    @Override
    public void onCreate() {
        super.onCreate();
        mContext = this;

        /*IntentFilter micToggleFilter = new IntentFilter();

        micToggleFilter.addAction(AudioManager.ACTION_MICROPHONE_MUTE_CHANGED);
        registerReceiver(mMuteChangeReceiver, micToggleFilter, mContext.RECEIVER_EXPORTED);*/

    }

    @Override
    public IBinder onBind(Intent intent) {
         return null;
     }

   /* private final BroadcastReceiver mMuteChangeReceiver = new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, Intent intent) {
            Log.d(TAG, "onReceive!!");
            updateMicToggleState(context);
        }
    };
    private void updateMicToggleState (Context context) {
        ContentResolver resolver =  context.getContentResolver();
        ContentValues value = new ContentValues();

       // boolean mic_enable = getMicToggleState();
        AudioManager audiomanage = new AudioManager();
        boolean isStreamMuted = audiomanage.isMicrophoneMute();

        if (!isStreamMuted)
            value.put(COLUME_ID, 1);
        else
            value.put(COLUME_ID, 0);
        int ret = resolver.update(HOTWORDMIC_URI, value, "", null);
    }*/
}
