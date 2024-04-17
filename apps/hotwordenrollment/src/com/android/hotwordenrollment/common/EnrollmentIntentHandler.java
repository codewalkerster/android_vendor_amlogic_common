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

import android.content.Context;
import android.content.Intent;
import android.text.TextUtils;
import android.util.Log;

/**
 * Common utility for handling enrollment intents.
 *
 * Regardless of how the intent was received by the application, the utility methods here perform
 * all the work necessary for handling an intent.
 */
public class EnrollmentIntentHandler {
    private static final String TAG = "EnrollmentIntentHandler";

    public static boolean isValidIntent(Intent intent) {
        // This service must only be started to manage keyphrases.
        if (!EnrollmentUtil.ACTION_MANAGE_VOICE_KEYPHRASES.equals(
                intent.getAction())) {
            Log.e(TAG, "Enrollment should be started only to manage keyphrases");
            return false;
        }

        if (intent.hasExtra(EnrollmentUtil.EXTRA_VOICE_KEYPHRASE_ACTION)
                && !TextUtils.isEmpty(
                intent.getStringExtra(EnrollmentUtil.EXTRA_VOICE_KEYPHRASE_HINT_TEXT))
                && !TextUtils.isEmpty(
                intent.getStringExtra(EnrollmentUtil.EXTRA_VOICE_KEYPHRASE_LOCALE))) {
            return true;
        }

        Log.e(TAG, "Invalid enrollment intent: " + intent);
        return false;
    }

    public static boolean handleIntent(Context context, Intent intent) {
        EnrollmentUtil enrollmentUtil = new EnrollmentUtil(context);
        int keyphraseAction = intent.getIntExtra(EnrollmentUtil.EXTRA_VOICE_KEYPHRASE_ACTION, -1);
        String keyphraseText = intent.getStringExtra(
                EnrollmentUtil.EXTRA_VOICE_KEYPHRASE_HINT_TEXT);
        String keyprhaseLocale = intent.getStringExtra(EnrollmentUtil.EXTRA_VOICE_KEYPHRASE_LOCALE);
        boolean actionResult = enrollmentUtil.startAction(keyphraseAction, keyphraseText,
                keyprhaseLocale);
        if (actionResult) {
            Log.i(TAG, "action " + keyphraseAction + ", keyphrase " + keyphraseText
                    + ", operation succeeded for "
                    + keyprhaseLocale);
        } else {
            Log.i(TAG, "action " + keyphraseAction + ", keyphrase " + keyphraseText
                    + ", operation failed for "
                    + keyprhaseLocale);
        }
        return actionResult;
    }
}
