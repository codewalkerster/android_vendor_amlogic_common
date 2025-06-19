package com.droidlogic.app;

import android.content.Context;
import android.graphics.*;
import android.graphics.drawable.AnimationDrawable;
import android.graphics.drawable.BitmapDrawable;
import android.graphics.drawable.Drawable;
import android.os.Process;
import android.os.RemoteException;
import android.util.AttributeSet;
import android.util.Log;
import android.view.*;
import android.view.WindowManager.LayoutParams;
import android.widget.ImageView;
import android.widget.TextView;
import android.widget.FrameLayout;
import android.widget.LinearLayout;
import android.widget.RelativeLayout;
import java.util.regex.*;
import android.graphics.Bitmap.Config;
import android.graphics.BlendMode;
import android.util.DisplayMetrics;

import android.net.Uri;

import java.io.ByteArrayOutputStream;
import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.List;
import java.lang.reflect.Field;
import java.lang.reflect.Method;
import android.os.Handler;
import android.os.Looper;

import android.os.Message;

import android.text.SpannableString;
import android.text.Spanned;
import android.text.style.ForegroundColorSpan;



class SubtitleViewAdaptor {
    private static final String TAG = SubtitleViewAdaptor.class.getSimpleName();
    private static final boolean DEBUG_LAYOUT = false;
    private static final int MAX_OBJECT_SEGMENT_ID = 2;
    private Context mContext;
    private FrameLayout mSubLayout = null;
    private TextView mTextView;
    private ImageView[] mImageView;
    private boolean bOpenAiAdaptiveArea = false;
    private boolean mAiTranslationEnabled = false;

    private CCSubtitleView mCcSubtitleView;
    boolean mIsWindowCreated;

    private AnimationDrawable mAnimationDrawable;

    //fallback display window media overlay type
    private int TYPE_APPLICATION_MEDIA_OVERLAY = 1004;

    private int HEIGHT_DVB_SUBTITLE_ADJUST = 10;
    private float RATIO_DVB_SUBTITLE_SCALE = 0.8f;
    private float WIDTH_RATIO_DVB_SUBTITLE_EXTEND = (1.0f - RATIO_DVB_SUBTITLE_SCALE) / 2.0f;

    private Display mDisplay;
    private WindowManager mWindowManager;
    private WindowManager.LayoutParams mWindowLayoutParams;

    private float mWscale = 1.000f;
    private float mHscale = 1.000f;
    private int mWmax = 0;
    private int mHmax = 0;
    private int mGravity = 0;
    private int mTextStyle = -1;
    private int mPosHeight = 0;
    private int mTextSize = 0;
    private int mTextColor = 0;
    private int[] mCoordinateX = {0,0};
    private int[] mCoordinateY = {0,0};
    private int mDisplayFlag = 0;
    private int mSubtitleType = -1;
    private boolean mDisableDisplay = false;

    //surface rect axis.
    private int mWindowX = 0;
    private int mWindowY = 0;
    private int mWindowW = 0;
    private int mWindowH = 0;

    private Bitmap interBitmap;

    private String mTitle;
    // To support setDisplayRect.
    private int mDisplayBoundWidth;
    private int mDisplayBoundHeight;

    private static final String[] NEW_LINE_FLAG = {"\\N","|"};
    private boolean dump2File = false;
    private static final String PROP_SAVE_IMAGE_FILE = "debug.subtitle.save_display_file";
    private  static final int  TEXT_MSG_NOT_SHOW = 2001;
    private  static final int  IMAGE0_MSG_NOT_SHOW = 2002;
    private  static final int  IMAGE1_MSG_NOT_SHOW = 2003;
    private String PIC_W ="vendor.hwc.aisubtile.pic_width";
    private String PIC_H = "vendor.hwc.aisubtile.pic_height";

    //2.width height 400*50
    private String AREA_W ="vendor.hwc.aisubtile.area_width";
    private String AREA_H ="vendor.hwc.aisubtile.area_height";

    //3.real adaptive area: 0,
    private String AREA_NUBER ="vendor.hwc.aisubtile.out_area";//not use now
    private DisplayMetrics outMetrics;

    /**
     *
     *    WARNING: ALL public method in this class MUST called in main(UI) thread!
     *
     */

    private void checkCallerOnUIThread() {
        if (Process.myPid() != Process.myTid()) {
            Log.d(TAG, "Please call this method in ui thread!");
            Log.d(TAG, "You can simply wrap a function, post runnable to let it call in UI thread:");
            throw new RuntimeException("need call in UI thread!");
        }
    }

    public SubtitleViewAdaptor(Context ctx) {
        checkCallerOnUIThread();
        mContext = ctx;
        mIsWindowCreated = false;
        mWindowManager = (WindowManager)mContext.getSystemService(Context.WINDOW_SERVICE);
        mImageView = new ImageView[MAX_OBJECT_SEGMENT_ID];
        ensureSubLayoutCreated();
        bOpenAiAdaptiveArea = false;
        setAIAdaptiveArea(bOpenAiAdaptiveArea);
        outMetrics = new DisplayMetrics();
        mWindowManager.getDefaultDisplay().getMetrics(outMetrics);
    }
    public boolean isDisplayWindowAdded() {
        checkCallerOnUIThread();
        Log.d(TAG, "isDisplayWindowAdded:"+mIsWindowCreated);
        return mIsWindowCreated;
    }

    public void setDisplayFlag(boolean flag) {
        mDisableDisplay = flag;
    }

    private void ensureSubLayoutCreated() {
        if (mSubLayout != null) {
            return;
        }

        // Construct view Hierarchy layout for subtitle.
        mSubLayout = new FrameLayout(mContext);
        FrameLayout.LayoutParams tparams=new FrameLayout.LayoutParams(
            ViewGroup.LayoutParams.FILL_PARENT,
            ViewGroup.LayoutParams.FILL_PARENT);

        Log.d(TAG, "mSubLayout:"+mSubLayout);
        LinearLayout.LayoutParams lparams = new LinearLayout.LayoutParams(
            ViewGroup.LayoutParams.WRAP_CONTENT,
            ViewGroup.LayoutParams.WRAP_CONTENT);

        RelativeLayout tlayout = new RelativeLayout(mContext);
        tlayout.setLayoutParams(lparams);
        //tlayout.setPadding(0, 0, 0, 50);
        //tlayout.setGravity(Gravity.BOTTOM | Gravity.CENTER_HORIZONTAL);
        mTextView = (TextView) new TextView(mContext);
        tlayout.addView(mTextView, lparams);
         //mSubLayout.addView(tlayout, tparams);
        for (int i=0; i<MAX_OBJECT_SEGMENT_ID; i++) {
           RelativeLayout ilayout = new RelativeLayout(mContext);
           ilayout.setLayoutParams(lparams);
           mImageView[i] = new ImageView(mContext);
           ilayout.addView(mImageView[i], lparams);
           mSubLayout.addView(ilayout, tparams);
            mImageView[i].setVisibility(View.INVISIBLE);
        }
        //for cc
        RelativeLayout cclayout = new RelativeLayout(mContext);
        mCcSubtitleView = new CCSubtitleView(mContext);
        cclayout.setPadding(0, 0, 0, 50);
        cclayout.addView(mCcSubtitleView, lparams);
        mSubLayout.addView(tlayout, tparams);
        mSubLayout.addView(cclayout, tparams);
        Log.d(TAG, "mSubLayout2:"+mSubLayout);

        mTextView.setVisibility(View.INVISIBLE);
        mCcSubtitleView.setVisibility(View.INVISIBLE);
        //mCcSubtitleView.hide();

        mDisplay = mWindowManager.getDefaultDisplay();
        mDisplayBoundWidth = mDisplay.getWidth();
        mDisplayBoundHeight = mDisplay.getWidth();
        //mSubLayout.setBackgroundColor(0x60888888);
    }

    private void initialLayoutParams(int windowType, String windowTitle, int x, int y, int w, int h) {
        // Add window for subtitle
        mTitle = windowTitle;
        mWindowLayoutParams = new WindowManager.LayoutParams();
        mWindowLayoutParams.type = windowType;
        mWindowLayoutParams.format = PixelFormat.TRANSLUCENT;
        mWindowLayoutParams.flags = WindowManager.LayoutParams.FLAG_NOT_TOUCH_MODAL
                  | WindowManager.LayoutParams.FLAG_NOT_FOCUSABLE
                  | WindowManager.LayoutParams.FLAG_LAYOUT_NO_LIMITS
                  | WindowManager.LayoutParams.FLAG_NOT_TOUCHABLE;
        mWindowLayoutParams.gravity = Gravity.LEFT | Gravity.TOP;
        mWindowLayoutParams.setTitle(windowTitle);
        mWindowLayoutParams.x = x;
        mWindowLayoutParams.y = y;
        mWindowLayoutParams.width = w;
        mWindowLayoutParams.height = h;
        Log.d(TAG, "initialLayoutParams,type:" + windowType +",x:" + x + ",y:" + y + ",w:" + w + ",h:" + h);

        mDisplayBoundWidth = w;
        mDisplayBoundHeight =h;
    }

    public void addSubtitleView(String title) {
        checkCallerOnUIThread();
        if (mIsWindowCreated) return;

        ensureSubLayoutCreated();

        mDisplay = mWindowManager.getDefaultDisplay();
        if (mSubtitleType == SubtitleManager.TYPE_SUBTITLE_DVB_TELETEXT) {
            initialLayoutParams(TYPE_APPLICATION_MEDIA_OVERLAY/*LayoutParams.TYPE_APPLICATION_PANEL*/, title, 0, 0, mDisplay.getWidth(), mDisplay.getHeight());
        } else {
            initialLayoutParams(LayoutParams.TYPE_APPLICATION_PANEL, title, 0, 0, mDisplay.getWidth(), mDisplay.getHeight());
        }

        // Add window for subtitle
        try {
            mWindowManager.addView(mSubLayout, mWindowLayoutParams);
            mIsWindowCreated = true;
        } catch (RuntimeException ex) {
            Log.e(TAG, "Add Subtitle failed:" + ex);
            mIsWindowCreated = false;
            return;
        }
        Log.d(TAG, "addSubtitleView:"+title);
    }

    public void addSystemSubtitleView(String title) {
        checkCallerOnUIThread();

        if (mIsWindowCreated) return;

        ensureSubLayoutCreated();
        mDisplay = mWindowManager.getDefaultDisplay();
        initialLayoutParams(LayoutParams.TYPE_APPLICATION_OVERLAY, title, 0, 0, mDisplay.getWidth(), mDisplay.getHeight());
        // Add window for subtitle
        try {
            mWindowManager.addView(mSubLayout, mWindowLayoutParams);
            mIsWindowCreated = true;
        } catch (RuntimeException ex) {
            Log.e(TAG, "Add System Subtitle failed:" + ex);
            mIsWindowCreated = false;
            return;
        }

        Log.d(TAG, "addSystemSubtitleView:"+title);
    }

    public void addSystemSurfaceRectView(String title) {
        setSurfaceDisplayRect(mWindowX, mWindowY, mWindowW, mWindowH, title);
    }

    public void removeSubtitleView() {
        Log.d(TAG, "removeSubtitleView");
        checkCallerOnUIThread();

        if (!mIsWindowCreated) {
            return;
        }
        if (View.VISIBLE == mCcSubtitleView.getVisibility()) {
            mCcSubtitleView.setVisibility(View.INVISIBLE);
        }
        if (View.VISIBLE == mTextView.getVisibility()) {
            mTextView.setVisibility(View.INVISIBLE);
        }
        for (int i=0; i<MAX_OBJECT_SEGMENT_ID; i++) {
            if (View.VISIBLE == mImageView[i].getVisibility()) {
                mImageView[i].setVisibility(View.INVISIBLE);
            }
        }
        mWindowManager.removeViewImmediate(mSubLayout);
        mIsWindowCreated = false;
        mSubLayout = null;//for switch resolution, the surface not update which cause the subtitle size and position error
    }
    //add temp for translation subtitle,start

    Handler handler = new Handler()
    {
        public void handleMessage(Message msg) {
            Log.d(TAG, "msg.what =" + msg.what);
            switch (msg.what) {
            case TEXT_MSG_NOT_SHOW:
                Log.d(TAG, "not show message, setVisible false");
                 mTextView.setVisibility(View.INVISIBLE);
                break;
            case IMAGE0_MSG_NOT_SHOW:
                mImageView[0].setVisibility(View.INVISIBLE);
                break;
            case IMAGE1_MSG_NOT_SHOW:
                mImageView[1].setVisibility(View.INVISIBLE);
                break;
            }
        }
    };

    public void setAiTranslationLanguage(String lang) {
        if (lang != "") {
            mAiTranslationEnabled = true;
        } else {
            mAiTranslationEnabled = false;
        }
    }

    public void setAIAdaptiveArea(boolean openAi) {
         Log.d(TAG, "setAIadaptiveArea:" + openAi);
         bOpenAiAdaptiveArea = openAi;
         SystemControlManager mSystemControl = SystemControlManager.getInstance();
         mSystemControl.writeSysFs("/sys/module/aml_media/parameters/uvm_open_aisubtitle", openAi?"1":"0");
         Log.d(TAG, "setAIAdaptiveArea end");
    }

    public void showSubtitleString(String text, boolean showing) {
        checkCallerOnUIThread();
        if (mDisableDisplay)
            return;
        mTextView.setGravity(mGravity);
        mTextView.setTextAppearance(mTextStyle);
        if (0 == mTextSize) {
            mTextSize = 20;
        }
        if (0 == mTextColor) {
            mTextColor = Color.WHITE;
        }
        mTextView.setTextSize(mTextSize);
        mTextView.setTextColor(mTextColor);
        if (text != null) {
            if (text.contains("<i>") && text.contains("</i>")) {
                 mTextView.setTypeface(null, Typeface.ITALIC);
                text = text.replaceAll("<.*?>","");
                Log.d(TAG, "find italic style, text:" + text);
            } else if (text.contains("<b>") && text.contains("</b>")) {
                mTextView.setTypeface(null, Typeface.BOLD);
                text = text.replaceAll("<.*?>","");
                Log.d(TAG, "find bold style,text:" + text);
            } else {
                 mTextView.setTypeface(null, Typeface.NORMAL);
            }
        }

       RelativeLayout.LayoutParams tt = new RelativeLayout.LayoutParams(mTextView.getLayoutParams());
       if (bOpenAiAdaptiveArea) {
            // add for adaptive area text show 20250509
            SystemControlManager mSystemControl = SystemControlManager.getInstance();
            String tmp = mSystemControl.getProperty(PIC_W);
            int picW = Integer.parseInt(tmp.equals("")? "600": tmp);
            tmp =  mSystemControl.getProperty(PIC_H);
            int picH = Integer.parseInt(tmp.equals("")? "480": tmp);
            String area_1 = mSystemControl.readSysFs("/sys/module/aml_media/parameters/uvm_set_aisubtitle_area");
            Log.d(TAG, "area_1 content:" + area_1);
            int areaNumber = 0;
            try {
                areaNumber = Integer.parseInt(area_1.equals("")? "0": area_1);
            } catch (NumberFormatException e){
                Log.e(TAG, "area_1,parse area error:" + e.toString());
            }
            tmp =  mSystemControl.getProperty(AREA_W);
            int areaW =  Integer.parseInt(tmp.equals("")? "400": tmp);
            tmp =  mSystemControl.getProperty(AREA_H);
            int areaH =  Integer.parseInt(tmp.equals("")? "50": tmp);
            Log.d(TAG, "picW:" + picW + ",picH:" + picH + ",areaW:" + areaW + ",areaH:" + areaH + ",number:" + areaNumber);


            int heightPixels = outMetrics.heightPixels;
            int widthPixels = outMetrics.widthPixels;
            int dpi = outMetrics.densityDpi;
            //Log.d(TAG, "window h:" + heightPixels +",window w:" + widthPixels + ",dpi:" + dpi);

            int pos = 0;
            switch (areaNumber) {
                case 1:
                   pos = 1;
                   break;
               case 2:
                   pos = 15;
                   break;
               case 3:
                   pos = 14;
                   break;
               case 0:
               default:
                   pos = 0;
                   break;
            }
            float scale_H = heightPixels*1.0f/picH;
            float scale_W = widthPixels*1.0f/picW;
            int heightScale = (int) (areaH*scale_H );
            int widthScale = (int) (areaW*scale_W );
            tt.width = widthScale;
            tt.height = RelativeLayout.LayoutParams.WRAP_CONTENT;
            float dpiScale = dpi*1.0f/160;
            //Log.d(TAG, "scale h:" + scale_H + ", new height:" + heightScale + ",w scale:" + scale_W +", new width:" + widthScale);

            if (areaNumber == 2 || areaNumber == 3) {
                  tt.removeRule(RelativeLayout.ALIGN_PARENT_BOTTOM);
                  tt.addRule(RelativeLayout.ALIGN_PARENT_TOP);
                  tt.addRule(RelativeLayout.ALIGN_PARENT_LEFT);

                  tt.topMargin = (heightPixels) * (15-pos) / 15;
                  tt.leftMargin = (widthPixels - widthScale)/2;
                  //Log.d(TAG, "leftMargin:" + tt.leftMargin +",top margin:" + tt.topMargin);
            } else  {
                  tt.removeRule(RelativeLayout.ALIGN_PARENT_TOP);
                  tt.addRule(RelativeLayout.ALIGN_PARENT_BOTTOM);
                  tt.addRule(RelativeLayout.ALIGN_PARENT_LEFT);
                  tt.bottomMargin = (heightPixels) * pos / 15;
                  tt.leftMargin = (widthPixels - widthScale)/2;
                  //Log.d(TAG, "leftMargin:" + tt.leftMargin +",bottom margin:" + tt.bottomMargin );
            }

            mTextView.setLayoutParams(tt);
            //mTextView.setGravity(Gravity.LEFT);
            Log.d(TAG, "adaptive area, mTextView:" + mTextView);
        } else {
            tt.removeRule(RelativeLayout.ALIGN_PARENT_TOP);
            tt.removeRule(RelativeLayout.CENTER_HORIZONTAL);
            tt.addRule(RelativeLayout.ALIGN_PARENT_BOTTOM);
            tt.addRule(RelativeLayout.ALIGN_PARENT_LEFT);

            tt.width=  RelativeLayout.LayoutParams.MATCH_PARENT;
            tt.height = RelativeLayout.LayoutParams.WRAP_CONTENT;
            mTextView.setLayoutParams(tt);
            if (mPosHeight == 0) {
               DisplayMetrics outMetrics = new DisplayMetrics();
               mWindowManager.getDefaultDisplay().getMetrics(outMetrics);
               mPosHeight = outMetrics.heightPixels/10;
               Log.d(TAG, "mPosHeight:" + mPosHeight);
            }
            Log.d(TAG, "mPosHeight:" + mPosHeight);
            SystemControlManager mSystemControl = SystemControlManager.getInstance();
            ViewGroup.MarginLayoutParams params = (ViewGroup.MarginLayoutParams) mTextView.getLayoutParams();
            int posx = mSystemControl.getPropertyInt("vendor.media.subtiltle_posx", 120);
            Log.d(TAG, "posx:" + posx);
            params.setMargins(posx, 0, 0, mPosHeight);
            if (mAiTranslationEnabled) {
                mTextView.setGravity(Gravity.LEFT);
            } else {
                mTextView.setGravity(Gravity.CENTER);
            }
            mTextView.setLayoutParams(params);
        }
        /*
        for (int i=0; i<MAX_OBJECT_SEGMENT_ID; i++) {
           mImageView[i].setVisibility(View.INVISIBLE);
        }
        */
        if (!showing) {
            //mTextView.setVisibility(View.INVISIBLE);//add for subtitle translate temp
            return;
        }
        //add for subtitle translate temp,start
        handler.removeMessages(TEXT_MSG_NOT_SHOW);//add temp for translation subtitle
        handler.sendEmptyMessageDelayed(TEXT_MSG_NOT_SHOW, 6*1000);//add temp for translation subtitle,6S
        //add end

        //for hebrew language,need app config "android:supportsRtl="true", has verified on U
        mTextView.setTextDirection(View.TEXT_DIRECTION_LTR);

        mTextView.setVisibility(View.VISIBLE);
        //mCcSubtitleView.hide();
        mCcSubtitleView.setVisibility(View.INVISIBLE);
        if (text != null) {
            for (int i=0; i<NEW_LINE_FLAG.length; i++) {
                text = text.replace(NEW_LINE_FLAG[i], "\n");
            }

            Pattern pattern1 = Pattern.compile("(?<=\\{)[^\\}]+");
            Matcher m = pattern1.matcher(text);
            while (m.find()) {
                text = text.replace("{"+m.group()+"}", "");
            }
        }
        Log.d(TAG, "showText:"+text);
        if (text != null) {
            text = text.replaceAll ("\r", "");
            byte tmpStrByte[] = text.getBytes();

            if (tmpStrByte.length > 0 && 0 == tmpStrByte[tmpStrByte.length - 1]) {
                tmpStrByte[tmpStrByte.length - 1] = ' ';
            }

            String newText = new String(tmpStrByte);

            /* AI translated text likes below:

                This is original English text
                >>>>>>这是翻译出来的中文或者法文<<<<<<
            */
            int index = newText.indexOf(">>>>>>");
            if (index >= 0) {
                newText = newText.replace(">>>>>>", "");
            }
            int endIndex =  newText.indexOf("<<<<<<");
            if (endIndex >= 0) {
                newText = newText.replace("<<<<<<", "");
            }
            SpannableString spannableString = new SpannableString(newText);
            if (index >= 0) {
                 ForegroundColorSpan firstColorSpan = new ForegroundColorSpan(Color.YELLOW);
                 spannableString.setSpan(firstColorSpan, 0, index, Spanned.SPAN_EXCLUSIVE_EXCLUSIVE);
            }
            if (mTextView != null) {
                mTextView.setVisibility(View.VISIBLE);
                if (mAiTranslationEnabled) {
                    mTextView.setGravity(Gravity.LEFT);
                } else {
                    mTextView.setGravity(Gravity.CENTER);
                }
                mTextView.setText(spannableString);
                Log.d(TAG, "Layout = " + mSubLayout + ", Text= " + mTextView.getText() + ", view = " + mTextView);
            }
        }
    }

    public void setDisplayType(int type) {
        checkCallerOnUIThread();
        mDisplayFlag = type;
    }

    public void setSubtitleType(int type) {
        mSubtitleType= type;
    }

    public void displayView() {
        checkCallerOnUIThread();
        if (SubtitleManager.SUBTITLE_TXT == mDisplayFlag ) {
            mTextView.setVisibility(View.VISIBLE);
        }
        else if ((SubtitleManager.SUBTITLE_IMAGE == mDisplayFlag) ||
                  (SubtitleManager.SUBTITLE_IMAGE_CENTER == mDisplayFlag)) {
            for (int i=0; i<MAX_OBJECT_SEGMENT_ID; i++) {
                RelativeLayout.LayoutParams tt = new RelativeLayout.LayoutParams(mImageView[i].getLayoutParams());
                tt.removeRule(RelativeLayout.CENTER_VERTICAL);
                mImageView[i].setLayoutParams(tt);
                mImageView[i].setVisibility(View.VISIBLE);
            }

        }
        else if (SubtitleManager.SUBTITLE_CC_JASON == mDisplayFlag) {
            mCcSubtitleView.setVisibility(View.VISIBLE);
        }
    }

    public void startTtxLoading(int loadingId) {
        checkCallerOnUIThread();
        if (mAnimationDrawable == null || !mAnimationDrawable.isRunning()) {
             for (int i=0; i<MAX_OBJECT_SEGMENT_ID; i++) {
                mImageView[i].setBackground(mContext.getResources().getDrawable(loadingId));
                RelativeLayout.LayoutParams tt = new RelativeLayout.LayoutParams(mImageView[i].getLayoutParams());
                tt.addRule(RelativeLayout.CENTER_VERTICAL);
                tt.addRule(RelativeLayout.CENTER_HORIZONTAL);
                mImageView[i].setLayoutParams(tt);

                mAnimationDrawable = (AnimationDrawable) mImageView[i].getBackground();
                mImageView[i].setImageBitmap(null);
                mImageView[i].setVisibility(View.VISIBLE);
                mAnimationDrawable.start();
            }
        }
    }

    public void stopTtxLoading() {
        checkCallerOnUIThread();
        Log.d(TAG, "stopTtxLoading");
        for (int i=0; i<MAX_OBJECT_SEGMENT_ID; i++) {
            mImageView[i].setBackground(null);
            mImageView[i].setImageBitmap(null);
        }

        mCcSubtitleView.clearContent();//sometimes last cc subtitle will show after switch channel in iptv apk, so clear cc content
        if (mAnimationDrawable != null && mAnimationDrawable.isRunning()) {
            mAnimationDrawable.stop();
        }
    }


    public void hideView() {
        checkCallerOnUIThread();
        stopTtxLoading();
        if (SubtitleManager.SUBTITLE_TXT == mDisplayFlag ) {
            mTextView.setVisibility(View.INVISIBLE);
        }
        else if ((SubtitleManager.SUBTITLE_IMAGE == mDisplayFlag) ||
                  (SubtitleManager.SUBTITLE_IMAGE_CENTER == mDisplayFlag)) {
            for (int i=0; i<MAX_OBJECT_SEGMENT_ID; i++) {
               mImageView[i].setVisibility(View.INVISIBLE);
            }

        }
        else if (SubtitleManager.SUBTITLE_CC_JASON == mDisplayFlag) {
            mCcSubtitleView.clearContent();//sometimes last cc subtitle will show after switch channel in iptv apk, so clear cc content
            mCcSubtitleView.setVisibility(View.INVISIBLE);
        }
    }

    public void clearContent() {
        checkCallerOnUIThread();
        if (SubtitleManager.SUBTITLE_TXT == mDisplayFlag ) {
            mTextView.setText("");
        }
        else if ((SubtitleManager.SUBTITLE_IMAGE == mDisplayFlag) ||
                  (SubtitleManager.SUBTITLE_IMAGE_CENTER == mDisplayFlag)) {
            for (int i=0; i<MAX_OBJECT_SEGMENT_ID; i++) {
               mImageView[i].setImageBitmap(null);
            }
        }
   }
    public void showCaptionClose(String str) {
        checkCallerOnUIThread();
        if (mDisableDisplay)
            return;
        mTextView.setVisibility(View.INVISIBLE);
        for (int i=0; i<MAX_OBJECT_SEGMENT_ID; i++) {
            mImageView[i].setVisibility(View.INVISIBLE);
        }
        mCcSubtitleView.setVisibility(View.VISIBLE);
        mCcSubtitleView.showJsonStr(str);

    }
    public void resetForSeek() {
        checkCallerOnUIThread();
        // simply: invisible now
        //mTextView.setVisibility(View.INVISIBLE);
        //mImageView.setVisibility(View.INVISIBLE);;
    }

    public void setImgSubRatio(float ratioW, float ratioH, int maxW, int maxH) {
        checkCallerOnUIThread();
        mWscale = ratioW;
        mHscale = ratioH;
        mWmax = maxW;
        mHmax = maxH;
    }
    public void setGravity(int gravity) {
        checkCallerOnUIThread();
        mGravity = gravity;
    }
    public void setTextStype(int style) {
        checkCallerOnUIThread();
        mTextStyle = style;
     }
    public void setPosHeight(int posheight) {
        checkCallerOnUIThread();
       mPosHeight = posheight;
       Log.d(TAG, "setPosHeight:" + posheight);
    }
    public void setTextSize(int size ) {
        checkCallerOnUIThread();
        mTextSize = size;
    }
    public void setTextColor(int color) {
        checkCallerOnUIThread();
         mTextColor = color;
    }
    public void setCoordinate (int x, int y, int subtitleObjectSegmentId) {
        mCoordinateX[subtitleObjectSegmentId] = x;
        mCoordinateY[subtitleObjectSegmentId] = y;
      }

    //set the teletext Subtitlebitmap by scale
    public Bitmap createTTxBitmap (Bitmap bitmap, float wTtxScale, float hTtxScale ,int width, int height) {// for ttx Bitmap
        Log.d(TAG, "create TTxBitmap:" + ",wTtxScale:" + wTtxScale + ",hTtxScale:" + hTtxScale + ",width:" + width + ",height:" + height);
        if (bitmap == null) {
            return null;
        }
        float tempTTxwScale = 1.0f;
        float tempTTxhScale = 1.0f;
        int w1 = bitmap.getWidth();
        int h1 = bitmap.getHeight();
        if (SubtitleManager.SUBTITLE_IMAGE == mDisplayFlag) {
            tempTTxwScale = wTtxScale;
            tempTTxhScale = hTtxScale;
            // Apply scale for display.
        } else if(SubtitleManager.SUBTITLE_IMAGE_CENTER == mDisplayFlag) {
            tempTTxwScale = ((float)mWindowLayoutParams.width/w1) * 0.8f;
            tempTTxhScale = ((float)mWindowLayoutParams.height/h1) * 0.8f;
        }
        Matrix matrix = new Matrix();
        matrix.postScale(tempTTxwScale, tempTTxhScale);
        Bitmap TTxBitmap = Bitmap.createBitmap((int)(width * 0.8f), (int)(height * 0.8f), Config.ARGB_8888);
        Log.d(TAG, "TTxBitmap" + TTxBitmap);
        Canvas canvas = new Canvas(TTxBitmap);
        Paint paint = new Paint();
        paint.setAntiAlias(false);
        paint.setFilterBitmap(false);
        paint.setDither(false);
        canvas.drawBitmap(bitmap, matrix , paint);
        return TTxBitmap;
    }

    //set the bitmap by scale
    public Bitmap creatBitmapByScale(Bitmap bitmap, float wScale, float hScale, int wmax, int hmax) {
        Log.d(TAG, "creatBitmapByScale" + ",wScale:" + wScale + ",hScale:" + hScale + ",wmax:" + wmax + ",hmax:" + hmax);
        if (bitmap == null) {
            return null;
        }
        float tempwScale = 1.0f;
        float temphScale = 1.0f;
        int w = bitmap.getWidth();
        int h = bitmap.getHeight();
        Log.d(TAG, "showBitmap:w-"+w+", h-"+h);
        if (SubtitleManager.SUBTITLE_IMAGE == mDisplayFlag) {
            tempwScale = wScale;
            temphScale = hScale;
            // Apply scale for display.
        } else if(SubtitleManager.SUBTITLE_IMAGE_CENTER == mDisplayFlag) {
            tempwScale = ((float)mWindowLayoutParams.width/w) * 0.8f;
            temphScale = ((float)mWindowLayoutParams.height/h) * 0.8f;
        }


        Matrix matrix = new Matrix();
        matrix.postScale(tempwScale, temphScale);
        Log.d(TAG, "showBitmap:matrix-"+matrix+", tempwScale="+tempwScale+", temphScale="+temphScale);
        Bitmap resizedBitmap = Bitmap.createBitmap(bitmap, 0, 0, w, h, matrix, true);
        return resizedBitmap;
    }

    public void showBitmap(Bitmap bitmap, float wScale, float hScale, boolean showing, int subtitleObjectSegmentId) {
        checkCallerOnUIThread();
        if (mDisableDisplay)
            return;
        // mTextView.setVisibility(View.INVISIBLE);
        mCcSubtitleView.setVisibility(View.INVISIBLE);

        if (!showing) {
            Log.d(TAG, "hidden!");

            switch (subtitleObjectSegmentId) {
                case 0:
                case 1:
                    mImageView[subtitleObjectSegmentId].setVisibility(View.INVISIBLE);
                    break;
                default:
                    Log.d(TAG, "not support the object id:" + subtitleObjectSegmentId);
                    break;
            }
            return;
        }

        //add for subtitle translate temp,start
        int msgValue = (subtitleObjectSegmentId==0 ? IMAGE0_MSG_NOT_SHOW : IMAGE1_MSG_NOT_SHOW);
        handler.removeMessages(msgValue);//add temp for translation subtitle
        handler.sendEmptyMessageDelayed(msgValue, 6*1000);//add temp for translation subtitle,6S
        //add end

        Log.d(TAG, "showBitmap:" + bitmap + ",object id:" + subtitleObjectSegmentId);
        if (mSubtitleType == SubtitleManager.TYPE_SUBTITLE_DVB_TELETEXT) {
           interBitmap = createTTxBitmap(bitmap,wScale, hScale, (int)mWindowLayoutParams.width, (int)mWindowLayoutParams.height);
        } else {



           interBitmap = creatBitmapByScale(bitmap, wScale, hScale, mWmax, mHmax);
        }


        ViewGroup.MarginLayoutParams params = (ViewGroup.MarginLayoutParams) mImageView[subtitleObjectSegmentId].getLayoutParams();
        android.view.ViewGroup.LayoutParams  layoutParams = mImageView[subtitleObjectSegmentId].getLayoutParams();
        if ((mSubtitleType == SubtitleManager.TYPE_SUBTITLE_DVB)
            || (mSubtitleType == SubtitleManager.TYPE_SUBTITLE_SCTE27)
            || (mSubtitleType == SubtitleManager.TYPE_SUBTITLE_PGS)
            ||(mSubtitleType == SubtitleManager.TYPE_SUBTITLE_EXTERNAL)) {
            //Log.d(TAG, "mCoordinateX="+mCoordinateX+", mCoordinateY="+mCoordinateY + ",wScale:" + wScale + ",hScale:" + hScale);

            mCoordinateX[subtitleObjectSegmentId] = (int)(mCoordinateX[subtitleObjectSegmentId]*wScale);
            mCoordinateY[subtitleObjectSegmentId] = (int)(mCoordinateY[subtitleObjectSegmentId]*hScale);

            params.setMargins(mCoordinateX[subtitleObjectSegmentId], mCoordinateY[subtitleObjectSegmentId], 0, 0);
            mImageView[subtitleObjectSegmentId].setLayoutParams(params);

            Log.d(TAG, "mCoordinateX="+mCoordinateX[subtitleObjectSegmentId]+", mCoordinateY="+mCoordinateY[subtitleObjectSegmentId]);
        } else {
            RelativeLayout.LayoutParams tt = new RelativeLayout.LayoutParams(mImageView[subtitleObjectSegmentId].getLayoutParams());
            if (mSubtitleType == SubtitleManager.TYPE_SUBTITLE_DVB_TELETEXT) {
                stopTtxLoading();
                tt.addRule(RelativeLayout.CENTER_VERTICAL);
            } else {
                tt.addRule(RelativeLayout.ALIGN_PARENT_BOTTOM);
            }
            tt.addRule(RelativeLayout.CENTER_HORIZONTAL);
            mImageView[subtitleObjectSegmentId].setLayoutParams(tt);
        }
        if ( (interBitmap != null) && (mImageView[subtitleObjectSegmentId] != null) ) {
            mImageView[subtitleObjectSegmentId].setImageBitmap(interBitmap);
            mImageView[subtitleObjectSegmentId].setVisibility(View.VISIBLE);
            Log.d(TAG, "Layout>>"+mSubLayout+", bitmap:"+interBitmap.getWidth()+", "+mImageView[subtitleObjectSegmentId]);

        }
        if (DEBUG_LAYOUT) dumpViewHirarchy(mSubLayout);
    }

    public void setSurfaceDisplayParam(int x, int y, int w, int h) {
        Log.d(TAG, "setSurfaceDisplayParam x:" + x + ",y:" + y + ",w:" + w +",h:" + h);
        mWindowX = x;
        mWindowY = y;
        mWindowW = w;
        mWindowH = h;
    }

    public void setSurfaceDisplayRect(int x, int y, int w, int h, String title) {
        checkCallerOnUIThread();

        if (mIsWindowCreated) {
            removeSubtitleView();
        }
        mSubLayout = null;//for switch resolution, the surface not update which cause the subtitle size and position error

        ensureSubLayoutCreated();
        initialLayoutParams(LayoutParams.TYPE_APPLICATION_OVERLAY, title, x, y, w, h);

        // Add window for subtitle
        mWindowManager.addView(mSubLayout, mWindowLayoutParams);
        mIsWindowCreated = true;
        Log.d(TAG, "addSystemSubtitleView:" + title);
        //displayView();//when receive UI_SHOW to show
    }

    private void dumpViewHirarchy(View view) {
        StringBuilder sb = new StringBuilder("");
        dumpViewHierarchy(sb, "  ", view);
        Log.d(TAG, sb.toString());
    }

    private void dumpViewHierarchy(StringBuilder sb, String prefix, View view) {
        sb.append(prefix);
        if (view == null) {
          sb.append("null");
          return;
        }
        sb.append(view.toString());
        sb.append("\n");
        if (!(view instanceof ViewGroup)) {
          return;
        }
        ViewGroup grp = (ViewGroup)view;
        final int N = grp.getChildCount();
        if (N <= 0) {
          return;
        }
        prefix = prefix + "  ";
        for (int i=0; i<N; i++) {
          dumpViewHierarchy(sb, prefix, grp.getChildAt(i));
        }
    }
}
