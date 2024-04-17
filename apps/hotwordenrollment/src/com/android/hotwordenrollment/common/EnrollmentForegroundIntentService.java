/*
 * Copyright (C) 2020 Google Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

package com.android.hotwordenrollment.common;

import android.app.IntentService;
import android.app.Notification;
import android.app.NotificationChannel;
import android.app.NotificationManager;
import android.content.Context;
import android.content.Intent;
import android.util.Log;

import androidx.core.app.NotificationCompat;

/**
 * Enrollment service implementation to handle intents returned by {@link
 * android.service.voice.AlwaysOnHotwordDetector}.
 * Intents sent to this service should be invoked with
 * {@link Context#startForegroundService(Intent)}.
 *
 * @deprecated Use {@link EnrollmentDelegateReceiver} instead for communication. This must be left
 * supported because AGSA Android R prebuilt APK invokes the enrollment intent with {@link
 * Context#startForegroundService(Intent)}.
 */
@Deprecated
public class EnrollmentForegroundIntentService extends IntentService {

    private static final String TAG = "EnrollmentForegroundSrv";

    public EnrollmentForegroundIntentService() {
        super(EnrollmentForegroundIntentService.class.getSimpleName());
    }

    /**
     * Creates an IntentService.  Invoked by your subclass's constructor.
     *
     * @param name Used to name the worker thread, important only for debugging.
     */
    public EnrollmentForegroundIntentService(String name) {
        super(name);
    }

    @Override
    public void onCreate() {
        Log.d(TAG, "onCreate");
        super.onCreate();
    }

    private void startForegroundWithNotification() {
        Package p = EnrollmentForegroundIntentService.class.getPackage();
        assert p != null;
        String NOTIFICATION_CHANNEL_ID = p.getName();
        assert NOTIFICATION_CHANNEL_ID != null;

        String channelName = EnrollmentForegroundIntentService.class.getSimpleName();
        NotificationChannel channel = new NotificationChannel(NOTIFICATION_CHANNEL_ID,
                channelName, NotificationManager.IMPORTANCE_NONE);
        channel.setLockscreenVisibility(Notification.VISIBILITY_PRIVATE);
        NotificationManager manager =
                getSystemService(NotificationManager.class);
        assert manager != null;

        manager.createNotificationChannel(channel);

        NotificationCompat.Builder notificationBuilder =
                new NotificationCompat.Builder(this, NOTIFICATION_CHANNEL_ID);
        int msgIdentifier;
        msgIdentifier = getResources().getIdentifier("notification_setup_language_msg",
                "string", getPackageName());
        Notification notification = notificationBuilder.setOngoing(true)
                .setSmallIcon(getResources().getIdentifier("ic_app", "drawable", getPackageName()))
                .setContentTitle(getResources().getString(msgIdentifier))
                .setPriority(NotificationManager.IMPORTANCE_MIN)
                .setCategory(Notification.CATEGORY_SERVICE)
                .build();

        startForeground(1, notification);
    }

    @Override
    public void onDestroy() {
        Log.d(TAG, "onDestroy");
        super.onDestroy();
    }

    @Override
    protected void onHandleIntent(Intent intent) {
        Log.d(TAG, "onHandleIntent");
        if (!EnrollmentIntentHandler.isValidIntent(intent)) {
            return;
        }

        startForegroundWithNotification();
        EnrollmentIntentHandler.handleIntent(this, intent);
        stopForeground(true);
    }
}
