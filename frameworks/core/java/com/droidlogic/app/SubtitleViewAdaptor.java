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





class SubtitleViewAdaptor {
    private static final String TAG = SubtitleViewAdaptor.class.getSimpleName();
    private static final boolean DEBUG_LAYOUT = false;
    private Context mContext;
    private FrameLayout mSubLayout = null;
    private TextView mTextView;
    private ImageView mImageView;
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
    private int mCoordinateX = 0;
    private int mCoordinateY = 0;
    private int mDisplayFlag = 0;
    private int mSubtitleType = -1;
    private boolean mDisableDisplay = false;

    //surface rect axis.
    private int mWindowX = 0;
    private int mWindowY = 0;
    private int mWindowW = 0;
    private int mWindowH = 0;

    private Bitmap interBitmap ;
    private String mTitle;
    // To support setDisplayRect.
    private int mDisplayBoundWidth;
    private int mDisplayBoundHeight;

    private static final String[] NEW_LINE_FLAG = {"\\N","|"};

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
        ensureSubLayoutCreated();
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
        tlayout.setPadding(0, 0, 0, 50);
        tlayout.setGravity(Gravity.BOTTOM | Gravity.CENTER_HORIZONTAL);
        mTextView = (TextView) new TextView(mContext);
        tlayout.addView(mTextView, lparams);

        RelativeLayout ilayout = new RelativeLayout(mContext);
        ilayout.setLayoutParams(lparams);
        mImageView = new ImageView (mContext);
        ilayout.addView(mImageView, lparams);
        //ilayout.setBackgroundColor(0x7f0000FF);

        RelativeLayout cclayout = new RelativeLayout(mContext);
        ilayout.setLayoutParams(lparams);
        mCcSubtitleView = new CCSubtitleView(mContext);
        cclayout.setPadding(0, 0, 0, 50);
        cclayout.addView(mCcSubtitleView, lparams);
        mSubLayout.addView(tlayout, tparams);
        mSubLayout.addView(ilayout, tparams);
        mSubLayout.addView(cclayout, tparams);
        Log.d(TAG, "mSubLayout2:"+mSubLayout);

        mTextView.setVisibility(View.INVISIBLE);
        mImageView.setVisibility(View.INVISIBLE);
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

        mDisplayBoundWidth = w;
        mDisplayBoundHeight =h;
    }

    public void addSubtitleView(String title) {
        checkCallerOnUIThread();
        if (mIsWindowCreated) return;

        ensureSubLayoutCreated();

        mDisplay = mWindowManager.getDefaultDisplay();
        initialLayoutParams(TYPE_APPLICATION_MEDIA_OVERLAY/*LayoutParams.TYPE_APPLICATION_PANEL*/, title, 0, 0, mDisplay.getWidth(), mDisplay.getHeight());

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
        initialLayoutParams(TYPE_APPLICATION_MEDIA_OVERLAY, title, 0, 0, mDisplay.getWidth(), mDisplay.getHeight());
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
        if(View.VISIBLE == mImageView.getVisibility()) {
            mImageView.setVisibility(View.INVISIBLE);
        }
        mWindowManager.removeViewImmediate(mSubLayout);
        mIsWindowCreated = false;
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

       RelativeLayout.LayoutParams tt = new RelativeLayout.LayoutParams(mTextView.getLayoutParams());
       tt.addRule(RelativeLayout.ALIGN_PARENT_BOTTOM);
       tt.addRule(RelativeLayout.CENTER_HORIZONTAL);
       mTextView.setLayoutParams(tt);

       if (mPosHeight == 0) {
           DisplayMetrics outMetrics = new DisplayMetrics();
           mWindowManager.getDefaultDisplay().getMetrics(outMetrics);
           mPosHeight = outMetrics.heightPixels/10;
       }

       mTextView.setGravity(Gravity.CENTER);
       ViewGroup.MarginLayoutParams params = (ViewGroup.MarginLayoutParams) mTextView.getLayoutParams();
       params.setMargins(0, 0, 0, mPosHeight);
       mTextView.setLayoutParams(params);

       mImageView.setVisibility(View.INVISIBLE);;
       if (!showing) {
            mTextView.setVisibility(View.INVISIBLE);
            return;
       }

        mTextView.setVisibility(View.VISIBLE);
        //mCcSubtitleView.hide();
        mCcSubtitleView.setVisibility(View.INVISIBLE);
        for (int i=0; i<NEW_LINE_FLAG.length; i++) {
            text = text.replace(NEW_LINE_FLAG[i], "\n");
        }
        Pattern pattern1 = Pattern.compile("(?<=\\{)[^\\}]+");
        Matcher m = pattern1.matcher(text);
        while (m.find()) {
            text = text.replace("{"+m.group()+"}", "");
        }
        Log.d(TAG, "showText:"+text);
        if (text != null) {
            text = text.replaceAll ("\r", "");
            byte tmpStrByte[] = text.getBytes();

            if (tmpStrByte.length > 0 && 0 == tmpStrByte[tmpStrByte.length - 1]) {
                tmpStrByte[tmpStrByte.length - 1] = ' ';
            }

            if (mTextView != null) {
                mTextView.setVisibility(View.VISIBLE);
                mTextView.setText(new String(tmpStrByte));
                Log.d(TAG, "Layout" + mSubLayout + ", Text:" + mTextView.getText() + ", " + mTextView);
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
            RelativeLayout.LayoutParams tt = new RelativeLayout.LayoutParams(mImageView.getLayoutParams());
            tt.removeRule(RelativeLayout.CENTER_VERTICAL);
            mImageView.setLayoutParams(tt);
            mImageView.setVisibility(View.VISIBLE);
        }
        else if (SubtitleManager.SUBTITLE_CC_JASON == mDisplayFlag) {
            mCcSubtitleView.setVisibility(View.VISIBLE);
        }
    }

    public void startTtxLoading(int loadingId) {
        checkCallerOnUIThread();
        if (mAnimationDrawable == null || !mAnimationDrawable.isRunning()) {
            mImageView.setBackground(mContext.getResources().getDrawable(loadingId));
            RelativeLayout.LayoutParams tt = new RelativeLayout.LayoutParams(mImageView.getLayoutParams());
            tt.addRule(RelativeLayout.CENTER_VERTICAL);
            tt.addRule(RelativeLayout.CENTER_HORIZONTAL);
            mImageView.setLayoutParams(tt);

            mAnimationDrawable = (AnimationDrawable) mImageView.getBackground();
            mImageView.setImageBitmap(null);
            mImageView.setVisibility(View.VISIBLE);
            mAnimationDrawable.start();
        }
    }

    public void stopTtxLoading() {
        checkCallerOnUIThread();
        Log.d(TAG, "stopTtxLoading");
        mImageView.setBackground(null);
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
            mImageView.setVisibility(View.INVISIBLE);
        }
        else if (SubtitleManager.SUBTITLE_CC_JASON == mDisplayFlag) {
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
            mImageView.setImageBitmap(null);
        }
   }
    public void showCaptionClose(String str) {
        checkCallerOnUIThread();
        if (mDisableDisplay)
            return;
        mTextView.setVisibility(View.INVISIBLE);
        mImageView.setVisibility(View.INVISIBLE);
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
    }
    public void setTextSize(int size ) {
        checkCallerOnUIThread();
        mTextSize = size;
    }
    public void setTextColor(int color) {
        checkCallerOnUIThread();
         mTextColor = color;
    }
    public void setCoordinate (int x, int y) {
        mCoordinateX = x;
        mCoordinateY = y;
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

        if (mSubtitleType == SubtitleManager.TYPE_SUBTITLE_DVB) {
            tempwScale = wScale * RATIO_DVB_SUBTITLE_SCALE;
            temphScale = hScale * RATIO_DVB_SUBTITLE_SCALE;
        }
        Matrix matrix = new Matrix();
        matrix.postScale(tempwScale, temphScale);
        Log.d(TAG, "showBitmap:matrix-"+matrix+", tempwScale="+tempwScale+", temphScale="+temphScale);
        Bitmap resizedBitmap = Bitmap.createBitmap(bitmap, 0, 0, w, h, matrix, true);
        return resizedBitmap;
    }

    public void showBitmap(Bitmap bitmap, float wScale, float hScale, boolean showing) {
        checkCallerOnUIThread();
        if (mDisableDisplay)
            return;
        mTextView.setVisibility(View.INVISIBLE);
        mCcSubtitleView.setVisibility(View.INVISIBLE);

        if (!showing) {
            Log.d(TAG, "hidden!");
            mImageView.setVisibility(View.INVISIBLE);
            return;
        }

        Log.d(TAG, "showBitmap:" + bitmap);
        if (mSubtitleType == SubtitleManager.TYPE_SUBTITLE_DVB_TELETEXT) {
           interBitmap = createTTxBitmap(bitmap,wScale, hScale, (int)mWindowLayoutParams.width, (int)mWindowLayoutParams.height);
        } else {
           interBitmap = creatBitmapByScale(bitmap, wScale, hScale, mWmax, mHmax);
        }

        ViewGroup.MarginLayoutParams params = (ViewGroup.MarginLayoutParams) mImageView.getLayoutParams();
        android.view.ViewGroup.LayoutParams  layoutParams = mImageView.getLayoutParams();
        if ((mSubtitleType == SubtitleManager.TYPE_SUBTITLE_DVB)
            || (mSubtitleType == SubtitleManager.TYPE_SUBTITLE_SCTE27)
            || (mSubtitleType == SubtitleManager.TYPE_SUBTITLE_PGS)) {
            //Log.d(TAG, "mCoordinateX="+mCoordinateX+", mCoordinateY="+mCoordinateY + ",wScale:" + wScale + ",hScale:" + hScale);
            if (mSubtitleType == SubtitleManager.TYPE_SUBTITLE_DVB && interBitmap != null) {
                mCoordinateX = (int)(mCoordinateX*wScale)
                            + (int)((float)interBitmap.getWidth()/RATIO_DVB_SUBTITLE_SCALE
                            * WIDTH_RATIO_DVB_SUBTITLE_EXTEND);
                mCoordinateY = (int)(mCoordinateY*hScale) - HEIGHT_DVB_SUBTITLE_ADJUST;
            } else {
                mCoordinateX = (int)(mCoordinateX*wScale);
                mCoordinateY = (int)(mCoordinateY*hScale);
            }
            params.setMargins(mCoordinateX, mCoordinateY, 0, 0);
            mImageView.setLayoutParams(params);

            Log.d(TAG, "mCoordinateX="+mCoordinateX+", mCoordinateY="+mCoordinateY);
        } else {
            RelativeLayout.LayoutParams tt = new RelativeLayout.LayoutParams(mImageView.getLayoutParams());
            if (mSubtitleType == SubtitleManager.TYPE_SUBTITLE_DVB_TELETEXT) {
                stopTtxLoading();
                tt.addRule(RelativeLayout.CENTER_VERTICAL);
            } else {
                tt.addRule(RelativeLayout.ALIGN_PARENT_BOTTOM);
            }
            tt.addRule(RelativeLayout.CENTER_HORIZONTAL);
            mImageView.setLayoutParams(tt);
        }
        if ( (interBitmap != null) && (mImageView != null) ) {
            mImageView.setImageBitmap(interBitmap);
            mImageView.setVisibility(View.VISIBLE);
            Log.d(TAG, "Layout>>"+mSubLayout+", bitmap:"+interBitmap.getWidth()+", "+mImageView);
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

        ensureSubLayoutCreated();
        initialLayoutParams(LayoutParams.TYPE_APPLICATION_OVERLAY, title, x, y, w, h);

        // Add window for subtitle
        mWindowManager.addView(mSubLayout, mWindowLayoutParams);
        mIsWindowCreated = true;
        Log.d(TAG, "addSystemSubtitleView:" + mTitle);
        displayView();
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
