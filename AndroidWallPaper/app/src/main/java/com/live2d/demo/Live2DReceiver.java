package com.live2d.demo;

import java.util.Random;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;

public class Live2DReceiver extends BroadcastReceiver {

    private boolean airPlaneMode;

    public Live2DReceiver (){
        super();
        airPlaneMode = false;
    }

    @Override
    public void onReceive(Context context, Intent intent) {
        switch (intent.getAction()) {
            case Intent.ACTION_SCREEN_ON:
                JniBridgeJava.nativeStartRandomMotion();
                break;
            // Android API 31から非推奨
            case Intent.ACTION_CLOSE_SYSTEM_DIALOGS:
                Random rnd = new Random();
                float r = rnd.nextFloat();
                float g = rnd.nextFloat();
                float b = rnd.nextFloat();
                JniBridgeJava.nativeSetClearColor(r,g,b);
                break;
            case Intent.ACTION_AIRPLANE_MODE_CHANGED:
                airPlaneMode = !airPlaneMode;
                if (airPlaneMode)
                {
                    JniBridgeJava.SetBackGroundSpriteAlpha(1.0f);
                }
                else
                {
                    JniBridgeJava.SetBackGroundSpriteAlpha(0.0f);
                }
                break;
        }
    }
}
