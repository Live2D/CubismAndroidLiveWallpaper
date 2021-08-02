package com.live2d.demo;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;

public class Live2DReceiver extends BroadcastReceiver {
    @Override
    public void onReceive(Context context, Intent intent) {
        switch (intent.getAction()) {
            case Intent.ACTION_SCREEN_ON:
                JniBridgeJava.nativeRandomStartMotion();
        }
    }
}
