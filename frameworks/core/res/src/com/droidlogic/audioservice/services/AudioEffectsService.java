/*
 * Copyright (c) 2014 Amlogic, Inc. All rights reserved.
 *
 * This source code is subject to the terms and conditions defined in the
 * file 'LICENSE' which is part of this source code package.
 *
 * Description:
 *     AMLOGIC AudioEffectsSettingManagerService
 */

package com.droidlogic.audioservice.services;

import android.app.Service;
import android.content.Context;
import android.content.ContentProviderClient;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.BroadcastReceiver;
import android.database.ContentObserver;
import android.media.AudioSystem;
import android.media.tv.TvContract;
import android.net.Uri;
import android.os.IBinder;
import android.os.UserHandle;
import android.os.Binder;
import android.os.Handler;
import android.os.Message;
import android.os.SystemProperties;
import android.provider.Settings;
import android.text.TextUtils;
import android.util.Log;

import com.droidlogic.app.AudioEffectManager;
import com.droidlogic.app.DroidLogicUtils;
import com.droidlogic.app.SystemControlManager;
import com.droidlogic.audioservice.settings.SoundEffectManager;


/**
 * This Service modifies Audio and Picture Quality TV Settings.
 * It contains platform specific implementation of the TvTweak IOemSettings interface.
 */
public class AudioEffectsService extends Service {
    private static final String TAG = AudioEffectsService.class.getSimpleName();
    public static final String PACKEGE_NANME = "com.droidlogic";

    private static boolean DEBUG = true;
    private SoundEffectManager mSoundEffectManager;
    private AudioEffectsService mAudioEffectsService;
    private Context mContext = null;

    public AudioEffectsService() {
        mAudioEffectsService = this;
    }

    private final AudioSystem.ErrorCallback mAudioSystemCallback = new AudioSystem.ErrorCallback() {
        public void onError(int error) {
            switch (error) {
                case AudioSystem.AUDIO_STATUS_SERVER_DIED:
                    Log.i(TAG, "onError: audioserver was died! Recreate AudioEffect.");
                    mHandler.sendEmptyMessage(MSG_AUDIO_SERVER_DIED);
                    break;
                default:
                    break;
            }
        }
    };

    private static final int MSG_AUDIO_SERVER_DIED = 0;
    private static final int MSG_AUDIO_SERVER_CHECK_SATE = 1;
    private static final int MSG_AUDIO_SERVER_READY = 2;
    private Handler mHandler = new Handler() {
        public void handleMessage(Message msg) {
            switch (msg.what) {
                case MSG_AUDIO_SERVER_DIED:
                    mSoundEffectManager.deinit();
                    this.sendEmptyMessageDelayed(MSG_AUDIO_SERVER_CHECK_SATE, 60);
                    break;
                case MSG_AUDIO_SERVER_CHECK_SATE:
                    if (AudioSystem.checkAudioFlinger() != AudioSystem.AUDIO_STATUS_OK) {
                        this.sendEmptyMessageDelayed(MSG_AUDIO_SERVER_CHECK_SATE, 80);
                        Log.d(TAG, "AudioServer is not ready");
                    } else {
                        this.sendEmptyMessage(MSG_AUDIO_SERVER_READY);
                        Log.d(TAG, "AudioServer is ready");
                    }
                    break;
                case MSG_AUDIO_SERVER_READY:
                    mSoundEffectManager.init();
                    Log.d(TAG, "Restore AudioEffects over!");
                    break;
                default:
                    break;
            }
        }
    };

    @Override
    public void onCreate() {
        if (DEBUG) Log.d(TAG, "AudioEffectsService onCreate");
        mContext = this;
        mSoundEffectManager = SoundEffectManager.getInstance(mContext);
        AudioSystem.setErrorCallback(mAudioSystemCallback);
        handleActionStartUp();
    }

    @Override
    public void onDestroy() {
        if (DEBUG) Log.d(TAG, "onDestroy");
        if (mSoundEffectManager != null) {
            mSoundEffectManager.deinit();
        }
        unregisterCommandReceiver(this);
    }

    @Override
    public void onLowMemory() {
        if (DEBUG) Log.w(TAG, "onLowMemory");
    }

    @Override
    public IBinder onBind(Intent intent) {
        if (DEBUG) Log.d(TAG, "onBind");
        return mBinder;
    }

    private final IAudioEffectsService.Stub mBinder = new IAudioEffectsService.Stub() {
        public void init() {
            mSoundEffectManager.init();
        }

        public void deinit() {
            mSoundEffectManager.deinit();
        }

        public void reset() {
            mSoundEffectManager.reset();
        }

        public boolean isSupportVirtualX() {
            return mSoundEffectManager.isSupportVirtualX();
        }

        public void setDtsVirtualXMode(int virtualXMode) {
            mSoundEffectManager.setDtsVirtualXMode(virtualXMode);
        }

        public int getDtsVirtualXMode() {
            return mSoundEffectManager.getDtsVirtualXMode();
        }

        public void setDtsTruVolumeHdEnable(boolean enable) {
            mSoundEffectManager.setDtsTruVolumeHdEnable(enable);
        }

        public boolean getDtsTruVolumeHdEnable() {
            return mSoundEffectManager.getDtsTruVolumeHdEnable();
        }

        public int getSoundModeStatus () {
            return mSoundEffectManager.getSoundModeStatus();
        }

        public int getTrebleStatus () {
            return mSoundEffectManager.getTrebleStatus();
        }

        public int getBassStatus () {
            return mSoundEffectManager.getBassStatus();
        }

        public int getBalanceStatus () {
            return mSoundEffectManager.getBalanceStatus();
        }

        public int getVirtualSurroundStatus() {
            return mSoundEffectManager.getVirtualSurroundStatus();
        }

        public void setSoundMode (int mode) {
            mSoundEffectManager.setSoundMode(mode);
        }

        public void setUserSoundModeParam(int bandNumber, int value, int bandSum) {
            mSoundEffectManager.setUserSoundModeParam(bandNumber, value, bandSum);
        }

        public int getUserSoundModeParam(int bandNumber) {
            return mSoundEffectManager.getUserSoundModeParam(bandNumber);
        }

        public void setTreble (int step) {
            mSoundEffectManager.setTreble (step);
        }

        public void setBass (int step) {
            mSoundEffectManager.setBass (step);
        }

        public void setBalance (int step) {
            mSoundEffectManager.setBalance (step);
        }

        public void setVirtualSurround (int mode) {
            mSoundEffectManager.setVirtualSurround (mode);
        }

        public void setDapParam(int id, int value) {
            //mSoundEffectManager.saveDapParam(id, value);
            mSoundEffectManager.setDapParam(id, value);
        }

        public int getDapParam(int id) {
            return mSoundEffectManager.getDapParam(id);
        }

        public void initDapAudioEffect() {
            mSoundEffectManager.initDapAudioEffect();
        }

        public void setDpeParam(int id, int value) {
            mSoundEffectManager.setDpeParam(id, value);
        }

        public int getDpeParam(int id) {
            return mSoundEffectManager.getDpeParam(id);
        }

        public void initDpeAudioEffect() {
            mSoundEffectManager.initDpeAudioEffect();
        }

        public void setAudioEffectOn(int id, boolean dbSwitch) {
            mSoundEffectManager.setAudioEffectOn(id, dbSwitch);
        }

        public boolean isAudioEffectOn(int id) {
            return mSoundEffectManager.isAudioEffectOn(id);
        }
        public void setHpeqBandNum(int id, int value) {
            mSoundEffectManager.setHpeqBandNum(id, value);
        }
        public int getHpeqBandNum(int id) {
            return mSoundEffectManager.getHpeqBandNum(id);
        }
    };

    private void handleActionStartUp() {
        Log.i(TAG, "handleActionStartUp: is tv:" + DroidLogicUtils.isTv());
        // This will apply the saved audio settings on boot
        mSoundEffectManager.init();
    }

    private static final String RESET_ACTION = "droid.action.resetsoundeffect";
    private void registerCommandReceiver(Context context) {
        IntentFilter intentFilter = new IntentFilter();
        intentFilter.addAction(RESET_ACTION);
        context.registerReceiver(mSoundEffectSettingsReceiver, intentFilter, context.RECEIVER_EXPORTED);
    }

    private void unregisterCommandReceiver(Context context) {
        context.unregisterReceiver(mSoundEffectSettingsReceiver);
    }

    private final BroadcastReceiver mSoundEffectSettingsReceiver = new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, Intent intent) {
            if (DEBUG) Log.d(TAG, "intent = " + intent);
            if (intent != null) {
                if (RESET_ACTION.equals(intent.getAction())) {
                    mSoundEffectManager.reset();
                }
            }
        }
    };
}
