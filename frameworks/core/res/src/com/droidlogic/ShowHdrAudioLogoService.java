/*
 * Copyright (C) 2015 The Android Open Source Project
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
 * limitations under the License
 */

package com.droidlogic;

import android.app.Service;
import android.content.Intent;
import android.os.Binder;
import android.os.Handler;
import android.os.HandlerThread;
import android.os.IBinder;
import android.os.Looper;
import android.os.Message;
import android.util.Log;

import android.content.Context;
import android.os.Handler;
import com.droidlogic.app.SystemControlManager;
import com.droidlogic.app.SystemControlEvent;


import android.content.BroadcastReceiver;

import android.content.IntentFilter;
import android.view.LayoutInflater;
import android.view.View;
import android.widget.ImageView;
import android.widget.LinearLayout;
import android.widget.RelativeLayout;
import android.widget.TextView;




import com.droidlogic.R;


public class ShowHdrAudioLogoService extends Service implements SystemControlEvent.HdrInfoListener {
    public static final String TAG = "ShowHdrAudioLogoService";
    private final IBinder mBinder = new ViViDBinder();

    public static int mThreadId = -1;

    private static final int R_EVENT_READ = 1;
    private static final int R_EVENT_HIDE_LOGO = 2;

    private static final int R_EVENT_CHECK_TYPE = 3;
    private static final int MSG_UPDATE_HDR = 5;
    private static final int MSG_UPDATE_AUDIO = 6;
    private static final int MSG_CLEAR_AUDIO_CONTENT = 8;
    private static final int MSG_CLEAR_HDR_CONTENT = 9;
    private static final int R_EVENT_HIDE_LOGO_AUDIO = 10;
    private static final int R_EVENT_HIDE_LOGO_HDR = 12;

    private static final int READ_TIME = 500;
    // must the same as driver types
    public static final int AUDIO_TYPE_DTS_EXPRESS = 1;
    public static final int AUDIO_TYPE_AC3 = 2;
    public static final int AUDIO_TYPE_DTS = 3;
    public static final int AUDIO_TYPE_EAC3 = 4;
    public static final int AUDIO_TYPE_DTS_HD = 5;
    public static final int AUDIO_TYPE_MULTI_PCM = 6;
    public static final int AUDIO_TYPE_TRUE_HD = 7;
    public static final int AUDIO_TYPE_DTS_HD_MA = 8;
    public static final int AUDIO_TYPE_PCM_HIGH_SR = 9;
    public static final int AUDIO_TYPE_AC4 = 10;
    public static final int AUDIO_TYPE_MAT = 11;
    public static final int AUDIO_TYPE_DDP_ATMOS = 12;
    public static final int AUDIO_TYPE_TRUE_HD_ATMOS = 13;
    public static final int AUDIO_TYPE_MAT_ATMOS = 14;
    public static final int AUDIO_TYPE_AC4_ATMOS = 15;
    public static final int AUDIO_TYPE_DTS_HP = 16;
    public static final int AUDIO_TYPE_DDP_PROMPT_ON = 17;
    public static final int AUDIO_TYPE_THD_PROMPT_ON = 18;
    public static final int AUDIO_TYPE_MAT_PROMPT_ON = 19;
    public static final int AUDIO_TYPE_AC4_PROMPT_ON = 20;

    public static final int AUDIO_TYPE_D_T_S_X = 23;//for d t s x
    private int currentAudioFormat = -1;
    private int mSourceHdrInfo = -1;
    private SystemControlManager mSystemControlManager = null;
    private SystemControlEvent mSystemControlEvent;

    private static HdrAudioView mLogoView;
    private Handler mHandler;

   private static final String CONTROL_SHOW_PROP = "vendor.nrdp.suppress-notification";
   private static final int PROP_NOT_SHOW = 1;

    private boolean hasShowLogoForVideo = false;
    private boolean audioLogoShowing = false;
    private boolean hdrLogoShowing = false;
    private boolean hasDelay = false;

    public ShowHdrAudioLogoService() {
            mSystemControlManager = SystemControlManager.getInstance();
            mLogoView = HdrAudioView.getInstance();

            mHandler = new Handler() {
                @Override
                public void handleMessage(Message msg) {
                    switch (msg.what) {
                        case R_EVENT_HIDE_LOGO_HDR:
                            mLogoView.HiddenDolbyVisionCertification();
                            mLogoView.setHdrLogoTextViewVisibility(false);
                            hdrLogoShowing = false;
                            if (!hdrLogoShowing && !audioLogoShowing) {
                                showLogoView(false);
                            }

                            break;
                        case R_EVENT_HIDE_LOGO_AUDIO:

                            mLogoView.HiddenAudioLogoCertification();
                            mLogoView.setAudioLogoTextViewVisibility(false);
                            audioLogoShowing = false;
                            if (!hdrLogoShowing && !audioLogoShowing) {
                                showLogoView(false);
                            }
                            break;
                        case MSG_UPDATE_HDR:
                            if (audioLogoShowing && !hasDelay) {
                                hasDelay = true;
                                Log.i(TAG, "delay hdr show");
                                mHandler.sendEmptyMessageDelayed(MSG_UPDATE_HDR, 3400);
                            } else {
                               hasDelay = false;
                               hdrLogoShowing = true;
                               mLogoView.clearAudioParam();
                               updateHdrTextLog();
                            }

                            break;
                        case MSG_UPDATE_AUDIO:
                            if (hdrLogoShowing && !hasDelay) {
                                hasDelay = true;
                                Log.i(TAG, "delay audio show");
                                mHandler.sendEmptyMessageDelayed(MSG_UPDATE_AUDIO,  3400);
                            } else {
                                hasDelay = false;
                                audioLogoShowing = true;
                                mLogoView.setHdrLogoTextViewVisibility(false);
                                updateAudioTextLog();
                            }

                            break;
                        case MSG_CLEAR_AUDIO_CONTENT:
                            clearAudioContent();
                            audioLogoShowing = false;
                            break;
                        case MSG_CLEAR_HDR_CONTENT:
                            clearHdrContent();
                            hdrLogoShowing = false;
                            break;

                    }
                    super.handleMessage(msg);
                }
            };

    }


    @Override
    public IBinder onBind(Intent intent) {
        return mBinder;
    }

    public class ViViDBinder extends Binder {
        ShowHdrAudioLogoService getService() {
            return ShowHdrAudioLogoService.this;
        }
    }

    public boolean isShowing() {
        return mLogoView != null && mLogoView.isShow();
    }

    @Override
    public int onStartCommand(Intent intent, int flags, int startId) {

        //workHandler.sendEmptyMessageDelayed(R_EVENT_READ, READ_TIME);//read file every 500ms

        Log.d(TAG, "onStartCommand");

        return START_REDELIVER_INTENT;
    }

    @Override
    public void onCreate() {
        super.onCreate();
        Log.i(TAG,  "onCreate");
        registerAudioFormatReceiver();
        mSystemControlEvent = SystemControlEvent.getInstance(null);
        mSystemControlManager.setListener(mSystemControlEvent);
        mSystemControlEvent.setHdrInfoListener(this);
         mLogoView.createView(this);

    }

    @Override
    public void onDestroy() {
        super.onDestroy();
        unRegisterAudioFormatReceiver();
        mSystemControlEvent.setHdrInfoListener(null);
    }

    @Override
     public void onHdrInfoChange(int newHdrInfo) {
          Log.d(TAG, "onHdrInfoChange: hdr info is: " + newHdrInfo);
           if (mSystemControlManager.getPropertyInt(CONTROL_SHOW_PROP, 0) == PROP_NOT_SHOW) {
                Log.d(TAG, "vendor.nrdp.suppress-notification is 1, not show, return");
                return;
           }

          if (newHdrInfo != mSourceHdrInfo) {
              mSourceHdrInfo = newHdrInfo;
              if (mSourceHdrInfo >= 1 && mSourceHdrInfo <= 5) {
                  mHandler.removeMessages(MSG_UPDATE_HDR);

                  mHandler.sendEmptyMessageDelayed(MSG_UPDATE_HDR, 50);
              } else {
                  Log.d(TAG, "onHdrInfoChange: clear hdr info");
                  mHandler.sendEmptyMessageDelayed(MSG_CLEAR_HDR_CONTENT, 0);
              }
          } else {
              //Log.d(TAG, "onHdrInfoChange: same hdr info.");
          }
      }
     private void updateHdrTextLog() {

        if (mSourceHdrInfo == 3) {
            mLogoView.setHdrLogoText(GetSourceHdrType());
            mLogoView.setHdrLogoTextViewVisibility(false);
            mLogoView.showDolbyVisionCertification();
        } else {
            mLogoView.setHdrLogoText(GetSourceHdrType());
            mLogoView.setHdrLogoTextViewVisibility(true);
        }

        //if (!isShowing()) {
            showLogoView(true);
        //}
        mHandler.sendEmptyMessageDelayed(R_EVENT_HIDE_LOGO_HDR, 3200);
     }
     private void updateAudioTextLog() {
          mLogoView.setAudioLogoText(GetAudioStringFromBroadcast());
          mLogoView.showAudioLogoCertification(currentAudioFormat);

          showLogoView(true);

          mHandler.sendEmptyMessageDelayed(R_EVENT_HIDE_LOGO_AUDIO, 3200);

     }
     private void clearAudioContent() {
         Log.d(TAG, "clearContent");
         mLogoView.clearAudioParam();
     }
     private void clearHdrContent() {
         Log.d(TAG, "clearContent");
         mLogoView.clearHdrParam();
     }



    //display Logo view
    private void showLogoView(boolean show) {
        Log.d(TAG, "mLogoView.isCreated()" + mLogoView.isCreated() + ",show:" + show);

        if (show) {
            if (!mLogoView.isCreated()) {
                mLogoView.createView(getApplicationContext());
            }
            mLogoView.show();
        } else {
            mLogoView.hide();
        }
    }
    private BroadcastReceiver mAudioFormatReceiver = new BroadcastReceiver() {
        public void onReceive (Context context, Intent intent) {
            currentAudioFormat = intent.getIntExtra("audio_format_value", -1);
            Log.d(TAG, "receive broadcast, audio format:" + currentAudioFormat);
            if (mSystemControlManager.getPropertyInt(CONTROL_SHOW_PROP, 0) == PROP_NOT_SHOW) {
                Log.d(TAG, "vendor.nrdp.suppress-notification is 1, not show, return");
                return;
            }
            if (currentAudioFormat > 0 ) {
                mHandler.removeMessages(MSG_UPDATE_AUDIO);
                mHandler.sendEmptyMessageDelayed(MSG_UPDATE_AUDIO, 0);
            } else {
                 mHandler.sendEmptyMessageDelayed(MSG_CLEAR_AUDIO_CONTENT, 0);
            }
        }
    };


    public void registerAudioFormatReceiver() {
        IntentFilter intentFilter = new IntentFilter("droidlogic.audioservice.action.AUDIO_FORMAT");
        Intent intent = registerReceiver(mAudioFormatReceiver, intentFilter);

        Log.d (TAG, "[registerAudioFormatReceiver]mAudioFormatReceiver:" + mAudioFormatReceiver);
    }

     public void unRegisterAudioFormatReceiver() {
        Log.d (TAG, "[unRegisterAudioFormatReceiver]mAudioFormatReceiver:" + mAudioFormatReceiver);
        unregisterReceiver(mAudioFormatReceiver);


     }
     private String GetAudioStringFromBroadcast() {
         switch (currentAudioFormat) {
                /* dts Audio */
                case AUDIO_TYPE_DTS:
                case AUDIO_TYPE_DTS_HP:

                    Log.d(TAG, "GetDtsStringFromBroadcast()--DTS");
                    return "DTS";

               case AUDIO_TYPE_DTS_EXPRESS:
               case AUDIO_TYPE_DTS_HD_MA:
               case AUDIO_TYPE_DTS_HD:
                    Log.d(TAG, "GetDtsStringFromBroadcast()--DTSHD");
                    return "DTSHD";

                case AUDIO_TYPE_D_T_S_X:
                    Log.d(TAG, "GetDtsStringFromBroadcast()--DTSX");
                    return "DTSX";
                case AUDIO_TYPE_DDP_ATMOS:
                case AUDIO_TYPE_TRUE_HD_ATMOS:
                case AUDIO_TYPE_MAT_ATMOS:
                case AUDIO_TYPE_AC4_ATMOS:
                    return "DOLBY ATMOS";

                case AUDIO_TYPE_DDP_PROMPT_ON:
                case AUDIO_TYPE_THD_PROMPT_ON:
                case AUDIO_TYPE_MAT_PROMPT_ON:
                case AUDIO_TYPE_AC4_PROMPT_ON:
                case AUDIO_TYPE_AC3:
                case AUDIO_TYPE_EAC3:
                case AUDIO_TYPE_TRUE_HD:
                case AUDIO_TYPE_AC4:
                case AUDIO_TYPE_MAT:
                     return "DOLBY AUDIO";
                default :
                     //return "PCM";

            }
         return "";
    }
     private String GetSourceHdrType() {
        //SystemControlManager mSystemControl = SystemControlManager.getInstance();
        //int type = mSystemControl.GetSourceHdrType();
        //Log.d(TAG, "GetSourceHdrType, type:" + type);
        switch (mSourceHdrInfo) {
            case 1:
                return "HDR10";
            case 2:
                return "HDR10PLUS";
            case 3:
                return "DOVI";
            case 4:
                return "AHDR";

            case 5:
                return "HLG";

            case 6:
            default:
                return "";

        }

    }

}

