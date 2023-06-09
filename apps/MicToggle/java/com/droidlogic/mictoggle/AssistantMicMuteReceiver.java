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
import android.content.BroadcastReceiver;
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

public class AssistantMicMuteReceiver extends BroadcastReceiver {

    private static final String TAG = "AssistantMicMuteReceiver";

    private boolean isServiceStart = false;
    @Override
    public void onReceive(Context context, Intent intent) {
        if (isServiceStart)
            return;
        isServiceStart = true;
        context.startService(new Intent(context,AssistantMicMuteService.class));
    }
}
