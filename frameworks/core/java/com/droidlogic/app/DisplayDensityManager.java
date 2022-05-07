/*
 * Copyright (c) 2014 Amlogic, Inc. All rights reserved.
 *
 * This source code is subject to the terms and conditions defined in the
 * file 'LICENSE' which is part of this source code package.
 *
 * Description:
 *     AMLOGIC DispayDensityManager
 */
package com.droidlogic.app;

import android.content.Context;
import android.content.Intent;
import android.hardware.display.DisplayManager;
import android.util.Size;
import android.os.IBinder;
import android.os.RemoteException;
import android.os.SystemProperties;
import android.os.UserHandle;
import android.util.Log;
import android.view.Display;
import android.util.DisplayMetrics;
import android.provider.Settings;
import java.util.ArrayList;
import java.util.Set;
import java.lang.reflect.InvocationTargetException;
import java.lang.reflect.Method;

public class DisplayDensityManager {
    private static final int MEDIA_DISPLAY_DENSITY_HIGH = 320;
    private static final int MEDIA_DISPLAY_DENSITY_MIDDLE = 213;
    private static final String PROCESS_ONLY = "com.droidlogic";
    private static final String DENSITY_PROP = "ro.sf.lcd_density";
    private static final String ENV_IS_BEST_MODE = "ubootenv.var.is.bestmode";
    private static final String MAX_SIZE_HEIGHT_PROP = "ro.surface_flinger.max_graphics_height";
    private static final String MAX_SIZE_WIDTH_PROP = "ro.surface_flinger.max_graphics_width";
    private static final String DISPLAY_MODE_TRUE = "true";
    private static final String DISPLAY_MODE_FALSE = "false";
    private static final String MODE_KEYWORD_1080 = "1080";
    private static final String MODE_KEYWORD_4K = "4K2K";
    private static final String MODE_KEYWORD_4K2KSMPTE = "4K2KSMPTE";
    private static final String MODE_KEYWORD_720 = "720";
    private static final String MODE_KEYWORD_576 = "576";
    private static final String MODE_KEYWORD_480 = "480";
    private static final String MODE_KEYWORD_768 = "768";
    private static final String TAG = DisplayDensityManager.class.getSimpleName();
    private static final Integer MAX_HEIGHT_OF_UI = 1080;
    private static DisplayDensityManager mInstance;
    private static int mDefaultDensity;
    private static Size mDefaultSize;
    private static ArrayList<DensityWithName> multimap = new ArrayList();
    private static ArrayList<Integer> densityList = new ArrayList();
    private static int mMxDensity;
    private DisplayManager mDisplayManager;
    private Context mContext;

    private DisplayDensityManager(Context context,boolean create) {
        if (!create)
            return;
        mContext = context;
        mDisplayManager = (DisplayManager) context.getSystemService(Context.DISPLAY_SERVICE);
        mDefaultDensity = SystemProperties.getInt(DENSITY_PROP, DisplayMetrics.DENSITY_MEDIUM);
        SystemControlManager mSystemControlManager = SystemControlManager.getInstance();
        intialDisplayManager();
        syncDensity(Display.DEFAULT_DISPLAY);
    }

    public static synchronized DisplayDensityManager getInstance(Context cxt) {
        if (mInstance == null) {
            if (Enabled()) {
                multimap.add(new DensityWithName(MODE_KEYWORD_1080, new Size(1920, 1080)));
                multimap.add(new DensityWithName(MODE_KEYWORD_4K2KSMPTE, new Size(4096, 2160)));
                multimap.add(new DensityWithName(MODE_KEYWORD_4K, new Size(3840, 2160)));
                multimap.add(new DensityWithName(MODE_KEYWORD_720, new Size(1280, 720)));
                multimap.add(new DensityWithName(MODE_KEYWORD_576, new Size(720, 576)));
                multimap.add(new DensityWithName(MODE_KEYWORD_768, new Size(1366, 768)));
                multimap.add(new DensityWithName(MODE_KEYWORD_480, new Size(720, 480)));

                //  densityList.add(DisplayMetrics.DENSITY_XXXHIGH); //not used now largetest screen
                densityList.add(DisplayMetrics.DENSITY_XXHIGH);
                densityList.add(DisplayMetrics.DENSITY_XHIGH);
                densityList.add(DisplayMetrics.DENSITY_HIGH);
                densityList.add(DisplayMetrics.DENSITY_TV);
                densityList.add(DisplayMetrics.DENSITY_MEDIUM);
                densityList.add(DisplayMetrics.DENSITY_LOW);

                mInstance = new DisplayDensityManager(cxt,true);
            }else {
                mInstance = new DisplayDensityManager(cxt,false);
            }
        }
        return mInstance;
    }

    private void intialDisplayManager() {
        int mxHeight = SystemProperties.getInt(MAX_SIZE_HEIGHT_PROP,1920);
        int mxWidth = SystemProperties.getInt(MAX_SIZE_WIDTH_PROP,1080);
        mDefaultSize = new Size(mxWidth, mxHeight);
        if (mxHeight >0 && mxWidth > 0) {
            mMxDensity = getPrefDensity(mxWidth, mxHeight);
            int removePos = -1;
            for (int denfilter: densityList) {
                if (denfilter <= mMxDensity) {
                    break;
                }
                removePos++;
            }
            if (removePos >= 0 ) {
                for (int i = 0 ;i <= removePos;i++) {
                    densityList.remove(0);
                }
            }
            Log.d(TAG,"mMxDensity"+mMxDensity+"remove "+removePos+" "+densityList.size()+" mMXSize"+mxWidth +"x"+mxHeight);
        }
    }

    private void syncDensity(int displayId) {
        Display.Mode mode = mDisplayManager.getDisplay(displayId).getMode();
        adjustDisplayDensityByMode(displayId, mode.getPhysicalWidth(), mode.getPhysicalHeight());
    }
    public static boolean Enabled() {
        String currentProcName = "";
        try {
            Class activityThreadClass = Class.forName("android.app.ActivityThread");
            Method method = activityThreadClass.getMethod("currentActivityThread");

            Object activityThread = method.invoke(null);
            Method getProcessNameMethod = activityThreadClass.getMethod("getProcessName");

            Object processName = getProcessNameMethod.invoke(activityThread);
            currentProcName = processName.toString();
        } catch (Exception ex) {
            ex.printStackTrace();
        }
        Log.d(TAG,"processName"+currentProcName);
        return (!currentProcName.isEmpty() && currentProcName.equals(PROCESS_ONLY));
    }
    private int getPrefDensity(int width, int height) {
        Log.i(TAG, "getPrefDensity display { " + width + "x" + height +" width * 1.0f / mDefaultSize.getWidth()"+
        width * 1.0f / mDefaultSize.getWidth()+"&"+ height * 1.0f / mDefaultSize.getHeight());
        float scaleSize = width * 1.0f / mDefaultSize.getWidth() < height * 1.0f / mDefaultSize.getHeight() ?
                width * 1.0f / mDefaultSize.getWidth() : height * 1.0f / mDefaultSize.getHeight();
        int targetDensity = DisplayMetrics.DENSITY_LOW;

        int density = (int) (scaleSize * mDefaultDensity);
        for (int normalDensity : densityList) {
            if (density >= normalDensity) {
                targetDensity = normalDensity;
                break;
            }
        }
        Log.e(TAG, "density: " + density + " ->" + targetDensity + " scaleSize:" + scaleSize);
        return targetDensity;
    }
    public void adjustDisplayDensityByMode(int displayId, int width, int height) {
        if (!Enabled()) return;
        int targetDensity = getPrefDensity(width,height);

        try {
            Class globalclass = Class.forName("android.view.WindowManagerGlobal");
            Method getWmServiceMethod = globalclass.getDeclaredMethod("getWindowManagerService");
            getWmServiceMethod.setAccessible(true);
            Object iWindowManager = getWmServiceMethod.invoke(null);
            Method setForcedDisplayDensityForUser = iWindowManager.getClass().getMethod("setForcedDisplayDensityForUser", int.class, int.class, int.class);
            setForcedDisplayDensityForUser.invoke(iWindowManager, displayId, targetDensity, 0);
        } catch (NoSuchMethodException e) {
            e.printStackTrace();
        } catch (ClassNotFoundException e) {
            e.printStackTrace();
        } catch (IllegalAccessException e) {
            e.printStackTrace();
        } catch (InvocationTargetException e) {
            e.printStackTrace();
        }
    }

    static class DensityWithName {
        String mName;
        Size mSize;

        DensityWithName(String name, Size size) {
            mName = name;
            mSize = size;
        }
    }
}
