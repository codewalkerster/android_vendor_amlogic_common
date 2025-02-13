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

import android.content.Context;
import android.graphics.PixelFormat;
import android.os.Handler;
import android.view.Gravity;
import android.view.LayoutInflater;
import android.view.View;
import android.view.WindowManager;
import android.util.Log;
import android.widget.TextView;
import android.widget.RelativeLayout;
import android.widget.Toast;

import android.widget.ImageView;


public class HdrAudioView {

    private View mFloatView;
    TextView audioTextView;
    TextView hdrTextView;
    private WindowManager wm;
    private WindowManager.LayoutParams mParams;
    private boolean isShowing;
    private Handler mHandler;
    private static HdrAudioView mInstance;
    private static final String TAG = "HdrAudioView";
    private RelativeLayout CertificationView;
    private boolean isShowAudioToast = true;
    private Context mContext;
    public synchronized static HdrAudioView getInstance() {
        if (mInstance == null) {
            mInstance = new HdrAudioView();
        }
        return mInstance;
    }

    private HdrAudioView() {
    }

    public void createView(Context context) {
        LayoutInflater inflater = LayoutInflater.from(context);
        mFloatView = inflater.inflate(R.layout.hdr_audio_logo_layout, null, false);
        wm = (WindowManager) context.getSystemService(Context.WINDOW_SERVICE);
        mParams = new WindowManager.LayoutParams();
        mParams.type = WindowManager.LayoutParams.TYPE_SYSTEM_OVERLAY;
        mParams.gravity = Gravity.RIGHT | Gravity.TOP;
        mParams.format = PixelFormat.TRANSLUCENT;
        mParams.flags = WindowManager.LayoutParams.FLAG_NOT_TOUCH_MODAL
                | WindowManager.LayoutParams.FLAG_NOT_TOUCHABLE
                | WindowManager.LayoutParams.FLAG_NOT_FOCUSABLE
                | WindowManager.LayoutParams.FLAG_LAYOUT_NO_LIMITS;

        hdrTextView = mFloatView.findViewById(R.id.text_hdr_logo);
        audioTextView = mFloatView.findViewById(R.id.text_audio_logo);
        CertificationView = mFloatView.findViewById(R.id.CertificationView);
        isShowing = false;
        mContext = context;

    }
    public void clearAudioParam() {
        //setHdrLogoText("");
        setAudioLogoText("");
        setAudioLogoTextViewVisibility(false);
        isShowAudioToast = true;
        HiddenAudioLogoCertification();
        //HiddenDolbyVisionCertification();
    }
    public void clearHdrParam() {
        setHdrLogoText("");
        setHdrLogoTextViewVisibility(false);
        HiddenDolbyVisionCertification();
    }
    public void setHdrLogoText(String text) {
       Log.d(TAG, "setHdrLogoText:" + text);
       hdrTextView.setText(text);
      /* if (text.equals("")) {
          hdrTextView.setVisibility(View.GONE);
       } else {
          hdrTextView.setVisibility(View.VISIBLE);
       }*/
    }

    public void setHdrLogoTextViewVisibility(boolean visible) {
         Log.d(TAG, "setHdrLogoTextViewVisibility:" + visible);
         if (visible ) {
            hdrTextView.setVisibility(View.VISIBLE);
         } else {
            hdrTextView.setVisibility(View.GONE);
         }
    }
    public void setAudioLogoText(String text) {
       Log.d(TAG, "setAudioLogoText:" + text);
       audioTextView.setText(text);
       /*if (text.equals("")) {
          audioTextView.setVisibility(View.GONE);
       } else {
          audioTextView.setVisibility(View.VISIBLE);
       }*/
    }
     public void setAudioLogoTextViewVisibility(boolean visible) {
        Log.d(TAG, "setAudioLogoTextViewVisibility:" + visible);
        if (visible ) {
            audioTextView.setVisibility(View.VISIBLE);
         } else {
            audioTextView.setVisibility(View.GONE);
         }
    }
    public void showDolbyVisionCertification() {
           ImageView certificationVideoImageView = (ImageView) CertificationView.findViewById (R.id.CertificationDV);

           Log.d(TAG,"show dolby vision logo");
           certificationVideoImageView.setImageResource(R.drawable.cert_amdolby_vision_w);
           certificationVideoImageView.setVisibility(View.VISIBLE);
    }
    public void HiddenDolbyVisionCertification() {

            ImageView certificationVideoImageView = (ImageView) CertificationView.findViewById (R.id.CertificationDV);
            certificationVideoImageView.setVisibility(View.GONE);
    }
    public void showAudioLogoCertification(int currentAudioFormat) {

        ImageView certificationAudioImageView = (ImageView) CertificationView.findViewById (R.id.CertificationAudio);
        TextView certificationAtomsTextView = (TextView)CertificationView.findViewById(R.id.doblyatomstext);

            int res_id = -1;
            switch (currentAudioFormat) {
                /* dts Audio */
               case ShowHdrAudioLogoService.AUDIO_TYPE_DTS:
                    res_id = R.drawable.cert_white;
                    RelativeLayout.LayoutParams layoutParams = new RelativeLayout.LayoutParams(certificationAudioImageView.getLayoutParams());
                    layoutParams.height = mContext.getResources().getDimensionPixelSize(R.dimen.logo_dts_height);
                    layoutParams.width = mContext.getResources().getDimensionPixelSize(R.dimen.logo_dts_width);
                    certificationAudioImageView.setLayoutParams(layoutParams);
                    Log.d(TAG, "dts logo h:" + layoutParams.height + ",w:" + layoutParams.width);
                    break;
               case ShowHdrAudioLogoService.AUDIO_TYPE_DTS_EXPRESS:
               case ShowHdrAudioLogoService.AUDIO_TYPE_DTS_HD_MA:
               case ShowHdrAudioLogoService.AUDIO_TYPE_DTS_HD:
                    res_id = R.drawable.cert_hd_white;
                   RelativeLayout.LayoutParams layoutParams2 = new RelativeLayout.LayoutParams(certificationAudioImageView.getLayoutParams());
                   layoutParams2.height = mContext.getResources().getDimensionPixelSize(R.dimen.logo_dts_hd_height);
                   layoutParams2.width = mContext.getResources().getDimensionPixelSize(R.dimen.logo_dts_hd_width);
                   certificationAudioImageView.setLayoutParams(layoutParams2);
                   Log.d(TAG, "dtshd logo h:" + layoutParams2.height + ",w:" + layoutParams2.width);
                    break;

                case ShowHdrAudioLogoService.AUDIO_TYPE_D_T_S_X:
                    res_id = R.drawable.cert_x_dt;
                    break;

                //Dolby atoms
                case ShowHdrAudioLogoService.AUDIO_TYPE_DDP_ATMOS:
                case ShowHdrAudioLogoService.AUDIO_TYPE_TRUE_HD_ATMOS:
                case ShowHdrAudioLogoService.AUDIO_TYPE_MAT_ATMOS:
                case ShowHdrAudioLogoService.AUDIO_TYPE_AC4_ATMOS:
                    res_id = R.drawable.cert_amdolby_atmos_w;
                    break;
                case ShowHdrAudioLogoService.AUDIO_TYPE_DTS_HP:
                    if (isShowAudioToast) {
                        showToastAtBottom(mContext, R.string.error_unsupported_audio_stream_enable_dts_headphone);
                        isShowAudioToast = false;
                        return;
                    }
                    audioTextView.setVisibility(View.GONE);
                    break;
                case ShowHdrAudioLogoService.AUDIO_TYPE_DDP_PROMPT_ON:
                case ShowHdrAudioLogoService.AUDIO_TYPE_THD_PROMPT_ON:
                case ShowHdrAudioLogoService.AUDIO_TYPE_MAT_PROMPT_ON:
                case ShowHdrAudioLogoService.AUDIO_TYPE_AC4_PROMPT_ON:
                    certificationAtomsTextView.setVisibility(View.VISIBLE);
                    audioTextView.setVisibility(View.GONE);
                    return;

            }

            if (res_id <= 0) {
                Log.d(TAG, "res id <= 0, don't pop up audio logo");
                certificationAudioImageView.setVisibility(View.GONE);
                audioTextView.setVisibility(View.VISIBLE);
            } else {
                certificationAudioImageView.setImageResource(res_id);
                certificationAudioImageView.setVisibility(View.VISIBLE);
                audioTextView.setVisibility(View.GONE);
            }
    }

    public void HiddenAudioLogoCertification() {
       ImageView certificationAudioImageView = (ImageView) CertificationView.findViewById (R.id.CertificationAudio);
       TextView certificationAtomsTextView = (TextView)CertificationView.findViewById(R.id.doblyatomstext);
       certificationAudioImageView.setVisibility(View.GONE);
       certificationAtomsTextView.setVisibility(View.GONE);
    }

    public  void showToastAtBottom(Context context, int resId) {
        Toast toast = Toast.makeText (context.getApplicationContext(), resId, Toast.LENGTH_SHORT);
        toast.setGravity (Gravity.BOTTOM,0, 0);
        toast.setDuration (Toast.LENGTH_LONG);
        toast.show();
    }

    public void show() {
        if (!isShowing) {
            Log.d(TAG, "show view");
            wm.addView(mFloatView, mParams);
            isShowing = true;
        }
    }

    public void hide() {
        if (isShowing) {
            Log.d(TAG, "hide view");
            wm.removeViewImmediate(mFloatView);
            isShowing = false;
        }
    }


    public boolean isShow() {
        return isShowing;
    }

    public boolean isCreated() {
        return !(mFloatView == null);
    }




}
