/*
 * Copyright (C) 2020 Google Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License"); you may not
 * use this file except in compliance with the License. You may obtain a copy of
 * the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the
 * License for the specific language governing permissions and limitations under
 * the License.
 */

package com.android.hotwordenrollment.common;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.SharedPreferences;
import android.hardware.soundtrigger.SoundTrigger.KeyphraseSoundModel;
import android.util.Log;

/**
 * {@link BroadcastReceiver} that handles re-enrollment on reboots/package updates,
 * when there's a newer version of the model that has been supplied to the framework.
 * This gets called with {@link Intent#ACTION_BOOT_COMPLETED} or
 * {@link Intent#ACTION_MY_PACKAGE_REPLACED}.
 */
public class StartupReceiver extends BroadcastReceiver {
    private static final String TAG = "EnrollmentStartupRcv";
    private static final String PREFS_NAME = "enrollment_prefs";
    private static final String KEY_ENROLLED_VERSION = "key_enrolled_version";

    @Override
    public void onReceive(Context context, Intent intent) {
        if (!Intent.ACTION_BOOT_COMPLETED.equals(intent.getAction())
                && !Intent.ACTION_MY_PACKAGE_REPLACED.equals(intent.getAction())) {
            return;
        }

        // Do the update check on another thread.
        final PendingResult result = goAsync();
        Thread thread = new Thread() {
            public void run() {
                updateSoundModel(context);
                result.finish();
            }
        };
        thread.start();
    }

    private static void updateSoundModel(Context context) {
        EnrollmentUtil util = new EnrollmentUtil(context);
        SharedPreferences settings = context.getSharedPreferences(PREFS_NAME, 0);
        // See if the enrolled version according to the preferences is different from
        // util.getCurrentVersion(). If they are different, refresh the model(s).
        // Note: This only works for non-(speaker dependent) sound models currently.
        int enrolledVersion = settings.getInt(KEY_ENROLLED_VERSION, 0);
        if (enrolledVersion == util.getCurrentVersion()) {
            Log.v(TAG, "KeyphraseSoundModel id " + util.getKeyphraseId() + " is fresh at version "
                + enrolledVersion);
            return;
        } else {
            Log.i(TAG, "KeyphraseSoundModel id " + util.getKeyphraseId() + "is out of date:"
                    + " current=" + enrolledVersion
                    + ", new=" + util.getCurrentVersion());
        }

        boolean allSuccessfullyUpdated = true;
        // For each supported locale, see if we've enrolled and update the sound model if necessary.
        for (String bcp47Locale : util.getSupportedLocales()) {
            KeyphraseSoundModel model = util.getSoundModel(bcp47Locale);
            if (model == null || model.getKeyphrases() == null
                    || model.getKeyphrases().length == 0) {
                continue;
            }

            // Update the sound model, its version has changed.
            byte[] data = util.readHotwordResource(bcp47Locale);
            KeyphraseSoundModel newModel = new KeyphraseSoundModel(
                    model.getUuid(), util.getVendorUUID(), data, model.getKeyphrases(),
                    util.getCurrentVersion());
            Log.d(TAG, "updating model " + newModel);

            boolean success = util.addOrUpdateSoundModel(newModel);
            if (!success) {
                Log.e(TAG, "Failed to update out-of-date KeyphraseSoundModel id "
                    + util.getKeyphraseId());
            }
            allSuccessfullyUpdated &= success;
        }

        if (allSuccessfullyUpdated) {
            // Update the preference to ensure that we don't keep doing this again and again.
            settings.edit().putInt(KEY_ENROLLED_VERSION, util.getCurrentVersion()).commit();
            return;
        }
    }
}
