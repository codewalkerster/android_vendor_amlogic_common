/*
 * Copyright (C) 2014 Google Inc.
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

import android.annotation.Nullable;
import android.annotation.SystemApi;
import android.content.Context;
import android.content.pm.PackageManager;
import android.content.res.Resources;
import android.content.res.Resources.NotFoundException;
import android.content.res.XmlResourceParser;
import android.hardware.soundtrigger.KeyphraseEnrollmentInfo;
import android.hardware.soundtrigger.SoundTrigger;
import android.hardware.soundtrigger.SoundTrigger.Keyphrase;
import android.hardware.soundtrigger.SoundTrigger.KeyphraseSoundModel;
import android.os.RemoteException;
import android.os.ServiceManager;
import android.os.UserManager;
import android.service.voice.AlwaysOnHotwordDetector;
import android.util.ArrayMap;
import android.util.ArraySet;
import android.util.Log;

import com.android.internal.app.IVoiceInteractionManagerService;

import org.xmlpull.v1.XmlPullParserException;

import java.io.BufferedReader;
import java.io.ByteArrayOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.util.Collections;
import java.util.Locale;
import java.util.Set;
import java.util.UUID;

/**
 * Utility class for the enrollment operations like enroll;re-enroll & un-enroll.
 *
 * @hide
 */
@SystemApi
public class EnrollmentUtil {
    /**
     * Activity Action: Show activity for managing the keyphrases for hotword detection.
     * This needs to be defined by an activity that supports enrolling users for hotword/keyphrase
     * detection.
     */
    public static final String ACTION_MANAGE_VOICE_KEYPHRASES =
            KeyphraseEnrollmentInfo.ACTION_MANAGE_VOICE_KEYPHRASES;
    /**
     * Intent extra: The intent extra for the specific manage action that needs to be performed.
     * Possible values are {@link AlwaysOnHotwordDetector#MANAGE_ACTION_ENROLL},
     * {@link AlwaysOnHotwordDetector#MANAGE_ACTION_RE_ENROLL}
     * or {@link AlwaysOnHotwordDetector#MANAGE_ACTION_UN_ENROLL}.
     */
    public static final String EXTRA_VOICE_KEYPHRASE_ACTION =
            KeyphraseEnrollmentInfo.EXTRA_VOICE_KEYPHRASE_ACTION;
    /**
     * Intent extra: The hint text to be shown on the voice keyphrase management UI.
     */
    public static final String EXTRA_VOICE_KEYPHRASE_HINT_TEXT =
            KeyphraseEnrollmentInfo.EXTRA_VOICE_KEYPHRASE_HINT_TEXT;
    /**
     * Intent extra: The voice locale to use while managing the keyphrase.
     */
    public static final String EXTRA_VOICE_KEYPHRASE_LOCALE =
            KeyphraseEnrollmentInfo.EXTRA_VOICE_KEYPHRASE_LOCALE;
    private static final String TAG = "EnrollmentUtil";
    private static final boolean DBG = false;
    private static final String XML_NAMESPACE = "http://schemas.android.com/apk/res/android";

    private final IVoiceInteractionManagerService mModelManagementService;
    private final Context mContext;
    private UserManager mUserManager;
    // True only after the xml has been parsed.
    private boolean mParsedMetadata;
    private int mKeyphraseId;
    private int mRecognitionMode;
    private ArraySet<String> mSupportedLocales;

    private int mCurrentVersion;

    // True only after the strings have been parsed.
    private boolean mParsedConfig;
    private UUID mVendorUUID;
    private ArrayMap<String, String> mLocaleRemap;

    public EnrollmentUtil(Context context) {
        mModelManagementService = IVoiceInteractionManagerService.Stub.asInterface(
                ServiceManager.getService(Context.VOICE_INTERACTION_MANAGER_SERVICE));
        mContext = context;
        ensureMetadataXmlIsParsed();
        ensureConfigXmlParsed();
    }

    /**
     * Adds/Updates a sound model.
     * The sound model must contain a valid UUID,
     * exactly 1 keyphrase,
     * and users for which the keyphrase is valid - typically the current user.
     *
     * @param soundModel The sound model to add/update.
     * @return {@code true} if the call succeeds, {@code false} otherwise.
     */
    public boolean addOrUpdateSoundModel(KeyphraseSoundModel soundModel) {
        if (!verifyKeyphraseSoundModel(soundModel)) {
            return false;
        }

        int status = SoundTrigger.STATUS_ERROR;
        try {
            status = mModelManagementService.updateKeyphraseSoundModel(soundModel);
        } catch (RemoteException e) {
            Log.e(TAG, "RemoteException in updateKeyphraseSoundModel", e);
        }
        return status == SoundTrigger.STATUS_OK;
    }

    /**
     * Gets the sound model for the given keyphrase, null if none exists.
     * This should be used for re-enrollment purposes.
     * If a sound model for a given keyphrase exists, and it needs to be updated,
     * it should be obtained using this method, updated and then passed in to
     * {@link #addOrUpdateSoundModel(KeyphraseSoundModel)} without changing the IDs.
     *
     * @param bcp47Locale The locale for with to look up the sound model for.
     * @return The sound model if one was found, null otherwise.
     */
    @Nullable
    public KeyphraseSoundModel getSoundModel(String bcp47Locale) {
        KeyphraseSoundModel model = null;
        try {
            model = mModelManagementService.getKeyphraseSoundModel(getKeyphraseId(), bcp47Locale);
        } catch (RemoteException e) {
            Log.e(TAG, "RemoteException in updateKeyphraseSoundModel");
        }

        if (model == null) {
            return null;
        } else {
            return model;
        }
    }

    /**
     * Gets the value of the RecognitionMode from the enrollment_application.xml.
     */
    public int getRecognitionMode() {
        return mRecognitionMode;
    }

    /**
     * Gets the value of the keyphrase id from the enrollment_application.xml.
     */
    public int getKeyphraseId() {
        return mKeyphraseId;
    }

    /**
     * Gets a set of the supported locales from the enrollment_application.xml.
     *
     * Note that we may not actually have a resource associated with each locale, and any missing
     * resource files will generate a runtime failure.
     */
    public Set<String> getSupportedLocales() {
        return mSupportedLocales;
    }

    /**
     * Gets the vendor UUID defined in some strings.xml file.
     */
    public UUID getVendorUUID() {
        return mVendorUUID;
    }

    /**
     * Gets the current version of the sound model from the raw resources file 'version'.
     *
     * We store the version as a raw resource so that it's way easier to extract from a compiled APK
     * to check what models are actually being loaded.
     */
    public synchronized int getCurrentVersion() {
        if (mCurrentVersion == 0) {
            InputStream is = null;
            try {
                Resources resources = mContext.getResources();
                is = resources.openRawResource(
                        resources.getIdentifier("version", "raw", mContext.getPackageName()));
                BufferedReader reader = new BufferedReader(new InputStreamReader(is));
                mCurrentVersion = Integer.parseInt(reader.readLine());
            } catch (NotFoundException e) {
                Log.e(TAG, "NotFoundException while trying to read version", e);
            } catch (IOException e) {
                Log.e(TAG, "IOException while reading current version", e);
            } catch (NumberFormatException e) {
                Log.e(TAG, "Error reading current version");
            } finally {
                try {
                    if (is != null) is.close();
                } catch (IOException ignored) {
                }
            }
        }

        return mCurrentVersion;
    }

    /**
     * Deletes the sound model for the given locale.
     *
     * @param bcp47Locale The locale for with to look up the sound model for.
     * @return {@code true} if the call succeeds, {@code false} otherwise.
     */
    @Nullable
    public boolean deleteSoundModel(String bcp47Locale) {
        int status = SoundTrigger.STATUS_ERROR;
        try {
            status = mModelManagementService.deleteKeyphraseSoundModel(getKeyphraseId(),
                    bcp47Locale);
        } catch (RemoteException e) {
            Log.e(TAG, "RemoteException in updateKeyphraseSoundModel");
        }
        return status == SoundTrigger.STATUS_OK;
    }

    private boolean verifyKeyphraseSoundModel(KeyphraseSoundModel soundModel) {
        if (soundModel == null) {
            Log.e(TAG, "KeyphraseSoundModel must be non-null");
            return false;
        }
        if (soundModel.getUuid() == null) {
            Log.e(TAG, "KeyphraseSoundModel must have a UUID");
            return false;
        }
        if (soundModel.getData() == null) {
            Log.e(TAG, "KeyphraseSoundModel must have data");
            return false;
        }
        if (soundModel.getKeyphrases() == null || soundModel.getKeyphrases().length != 1) {
            Log.e(TAG, "Keyphrase must be exactly 1");
            return false;
        }
        Keyphrase keyphrase = soundModel.getKeyphrases()[0];
        if (keyphrase.getId() <= 0) {
            Log.e(TAG, "Keyphrase must have a valid ID");
            return false;
        }
        if (keyphrase.getRecognitionModes() < 0) {
            Log.e(TAG, "Recognition modes must be valid");
            return false;
        }
        if (keyphrase.getLocale() == null) {
            Log.e(TAG, "Locale must not be null");
            return false;
        }
        if (keyphrase.getText() == null) {
            Log.e(TAG, "Text must not be null");
            return false;
        }
        if (keyphrase.getUsers() == null || keyphrase.getUsers().length == 0) {
            Log.e(TAG, "Keyphrase must have valid user(s)");
            return false;
        }
        return true;
    }

    public byte[] readHotwordResource(String locale) {
        InputStream is = null;
        ByteArrayOutputStream baOs = null;
        byte[] data = null;
        try {
            Resources resources = mContext.getResources();
            String resourceName = getResourceNameForLocale(locale);
            Log.d(TAG, "opening raw resource: " + resourceName + " for locale " + locale);
            is = resources.openRawResource(resources.getIdentifier(resourceName, "raw",
                    mContext.getPackageName()));
            baOs = new ByteArrayOutputStream();
            int read;
            while ((read = is.read()) != -1) {
                baOs.write(read);
            }
            data = baOs.toByteArray();
        } catch (NotFoundException e) {
            Log.e(TAG, "NotFoundException while trying to read hotword data", e);
        } catch (IOException e) {
            Log.e(TAG, "IOException while reading hotword data", e);
        } finally {
            try {
                if (is != null) is.close();
            } catch (IOException ignored) {
            }
            try {
                if (baOs != null) baOs.close();
            } catch (IOException ignored) {
            }
        }

        if (data == null) {
            Log.e(TAG, "Failed to load the hotword data from resources");
        }
        return data;
    }

    /**
     * Returns the resource name to use for the given locale.
     *
     * This applies the map specified in the string-array 'locale-map' to allow for the same model
     * to be used for multiple locales.
     */
    private String getResourceNameForLocale(String locale) {
        if (mLocaleRemap.containsKey(locale)) {
            locale = mLocaleRemap.get(locale);
        }
        return locale.replace('-', '_').toLowerCase();
    }

    /**
     * Helper function to parse the enrollment_application.xml file and save the results in members.
     */
    private synchronized void ensureMetadataXmlIsParsed() {
        Log.d(TAG, "ensureMetadataXmlIsParsed");
        if (mParsedMetadata) {
            return;
        }

        Resources resources = mContext.getResources();
        PackageManager packageManager = mContext.getPackageManager();

        try (XmlResourceParser parser = resources.getXml(packageManager.getApplicationInfo(
                        mContext.getPackageName(), PackageManager.GET_META_DATA)
                .metaData.getInt("android.voice_enrollment"))) {
            int eventType = parser.getEventType();
            while (eventType != XmlResourceParser.END_DOCUMENT) {
                if (eventType == XmlResourceParser.START_TAG
                        && parser.getName().equals("voice-enrollment-application")) {
                    mSupportedLocales = new ArraySet<>();
                    Collections.addAll(mSupportedLocales, parser.getAttributeValue(XML_NAMESPACE,
                            "searchKeyphraseSupportedLocales").split(","));
                    mKeyphraseId =
                            parser.getAttributeIntValue(XML_NAMESPACE, "searchKeyphraseId", -1);
                    mRecognitionMode = parser.getAttributeIntValue(XML_NAMESPACE,
                            "searchKeyphraseRecognitionFlags", -1);
                    Log.d(TAG, "parsed keyphraseID=" + mKeyphraseId
                            + ", parsed recognitionMode=" + mRecognitionMode
                            + ", parsed supported locales=" + mSupportedLocales);
                }
                eventType = parser.nextToken();
            }
        } catch (Exception e) {
            throw new RuntimeException("Failed parsing enrollment application XML", e);
        }

        if (mSupportedLocales == null) {
            // If it's null, that means we weren't able to find any of the attributes.
            throw new RuntimeException("Failed to find correct attributes in enrollment xml.");
        }

        mParsedMetadata = true;
    }

    /**
     * Helper function to parse the string resources and save the results in members.
     */
    private synchronized void ensureConfigXmlParsed() {
        if (mParsedConfig) {
            return;
        }
        mLocaleRemap = new ArrayMap<>();

        Resources resources = mContext.getResources();

        try (XmlResourceParser parser = resources.getXml(resources.getIdentifier(
                "config", "xml", mContext.getPackageName()))) {
            while (parser.getEventType() != XmlResourceParser.END_DOCUMENT) {
                String nodeName = parser.getName();
                if ("vendor-uuid".equals(nodeName)
                        && XmlResourceParser.START_TAG == parser.getEventType()) {
                    mVendorUUID = UUID.fromString(parser.nextText());
                    Log.d(TAG, "vendorUUID set : " + mVendorUUID);
                } else if ("locale-map-entry".equals(nodeName)) {
                    String locale = parser.getAttributeValue(null, "locale");
                    String modelFileName = parser.getAttributeValue(null, "modelFileName");
                    mLocaleRemap.put(locale, modelFileName);
                }

                parser.next();
            }
        } catch (XmlPullParserException | IOException e) {
            throw new RuntimeException("Failed to parse voice model config XML", e);
        }

        Log.d(TAG, "local to model config parsed: " + mLocaleRemap);
        mParsedConfig = true;
    }

    public boolean startAction(int keyphraseAction, String hintText, String bcp47Locale) {
        mUserManager = (UserManager) mContext.getSystemService(Context.USER_SERVICE);

        switch (keyphraseAction) {
            case KeyphraseEnrollmentInfo.MANAGE_ACTION_ENROLL:
                return handleEnrollAction(hintText, bcp47Locale);
            case KeyphraseEnrollmentInfo.MANAGE_ACTION_RE_ENROLL:
                return handleReEnrollAction(hintText, bcp47Locale);
            case KeyphraseEnrollmentInfo.MANAGE_ACTION_UN_ENROLL:
                return handleUnEnrollAction(bcp47Locale);
            default:
                Log.e(TAG, "Enrollment started with an unknown action " + keyphraseAction);
                return false;
        }
    }

    private boolean handleEnrollAction(String hintText, String bcp47Local) {
        byte[] data = readHotwordResource(bcp47Local);
        if (data == null) {
            return false;
        }
        int currentUserHandle = mUserManager.getProcessUserId();
        Keyphrase[] keyphrases = new Keyphrase[1];
        keyphrases[0] = new Keyphrase(getKeyphraseId(),
                getRecognitionMode(), Locale.forLanguageTag(bcp47Local),
                hintText, new int[]{currentUserHandle});
        KeyphraseSoundModel soundModel = new KeyphraseSoundModel(
                UUID.randomUUID(), getVendorUUID(),
                data, keyphrases, getCurrentVersion());
        return addOrUpdateSoundModel(soundModel);
    }

    private boolean handleReEnrollAction(String hintText, String bcp47Local) {
        byte[] data = readHotwordResource(bcp47Local);
        if (data == null) {
            return false;
        }

        KeyphraseSoundModel model = getSoundModel(bcp47Local);
        if (model == null || model.getUuid() == null) {
            Log.w(TAG, "No models present for re-enrollment");
            return false;
        }

        // update this model.
        int currentUserHandle = mUserManager.getProcessUserId();
        Keyphrase[] keyphrases = new Keyphrase[1];
        keyphrases[0] = new Keyphrase(getKeyphraseId(),
                getRecognitionMode(), Locale.forLanguageTag(bcp47Local),
                hintText, new int[]{currentUserHandle});
        KeyphraseSoundModel soundModel = new KeyphraseSoundModel(
                model.getUuid(), getVendorUUID(),
                data, keyphrases, getCurrentVersion());
        return addOrUpdateSoundModel(soundModel);
    }

    private boolean handleUnEnrollAction(String bcp47Local) {
        return deleteSoundModel(bcp47Local);
    }
}
