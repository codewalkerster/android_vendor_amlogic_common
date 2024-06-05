/*
 * Copyright (c) 2014 Amlogic, Inc. All rights reserved.
 *
 * This source code is subject to the terms and conditions defined in the
 * file 'LICENSE' which is part of this source code package.
 *
 * Description:
 *     AMLOGIC BtSetupActivity
 */

package com.droidlogic;



import android.app.Activity;
import android.content.Context;
import android.os.Bundle;
import android.os.Handler;
import android.os.Message;
import android.util.Log;
import android.view.View.OnClickListener;
import android.widget.ImageView;
import android.widget.TextView;
import com.droidlogic.R;

import android.app.AlertDialog;
import android.content.DialogInterface;
import android.widget.Button;
import android.widget.TextView;
import android.view.View.OnClickListener;
import android.view.View;
import android.view.Gravity;
import android.view.WindowManager;
import android.content.Intent;

public class USBPowerActivity extends Activity {
    private static final String TAG = "USBPowerActivity";
    //private Context mContext;

   @Override
   protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.usb_power_notice);

        getWindow().setGravity(Gravity.BOTTOM);
        getWindow().setLayout(WindowManager.LayoutParams.MATCH_PARENT,WindowManager.LayoutParams.WRAP_CONTENT);

        Intent intent = getIntent();
        int power_level = intent.getIntExtra("POWER_LEVEL", 0);
        Log.i(TAG,  "usb power power_level:" + power_level);
        TextView text = findViewById(R.id.textview1);
        TextView text2 = findViewById(R.id.textView2);
        TextView text3 = findViewById(R.id.textview3);
        if (power_level == 0) {
            text.setText("The power adaptor which you are using has insufficient capacity, power level is \"0.5a\"");
            text2.setText("Some functions cannot be used, such as wifi, video cannot be played, CPU frequency reduction, GPU frequency reduction, etc.");
            text3.setText("If you want to enable these features, please replace the power adapter.");
        } else if (power_level == 1) {
            text.setText("The power adaptor which you are using has insufficient capacity, power level is \"1.5a\"");
            text2.setText("Some functions cannot be used, such as turning off PQ, CPU frequency reduction, etc.");
            text3.setText("If you want to enable these features, please replace the power adapter.");
        } else if (power_level == 2) {
            text.setText("The power adapter you are using has sufficient capacity, power level is \"3a\"");
            text2.setText("You can use all functions.");
            //text3.setText("The power supply current is sufficient and there is no need to replace the power adapter.");
            text3.setText("");
    }


        Button btn = (Button) findViewById(R.id.button);
        btn.setOnClickListener(new OnClickListener() {
                  public void onClick(View v) {
                      finish();
                  }
               });

     }


    @Override
    public void onStart() {
        super.onStart();
    }

}
