/*
 * Copyright(c) Live2D Inc. All rights reserved.
 * <p>
 * Use of this source code is governed by the Live2D Open Software license
 * that can be found at https://www.live2d.com/eula/live2d-open-software-license-agreement_en.html.
 */

package com.live2d.demo;

import android.app.ActivityManager;
import android.content.Context;
import android.content.pm.ConfigurationInfo;
import android.view.MotionEvent;
import android.view.SurfaceHolder;

import net.rbgrn.android.glwallpaperservice.*;

public class LiveWallpaperService extends GLWallpaperService {

    public LiveWallpaperService() {
        super();
    }

    public Engine onCreateEngine() {
        Live2DWallpaperEngine engine = new Live2DWallpaperEngine();
        return engine;
    }

    @Override
    public void onDestroy() {
        super.onDestroy();
    }

    class Live2DWallpaperEngine extends GLEngine {
        Live2DGLRenderer renderer;

        public Live2DWallpaperEngine() {
            super();

            JniBridgeJava.SetContext(getApplicationContext());
            JniBridgeJava.nativeOnStart();
        }

        @Override
        public void onCreate(SurfaceHolder surfaceHolder) {
            super.onCreate(surfaceHolder);

            // Check if the system supports OpenGL ES 2.0.
            final ActivityManager activityManager =
                    (ActivityManager) getSystemService(Context.ACTIVITY_SERVICE);
            final ConfigurationInfo configurationInfo =
                    activityManager.getDeviceConfigurationInfo();
            final boolean supportsEs2 =
                    configurationInfo.reqGlEsVersion >= 0x20000;

            if (supportsEs2)
            {
                // Request an OpenGL ES 2.0 compatible context.
                setEGLContextClientVersion(2);

                // On Honeycomb+ devices, this improves the performance when
                // leaving and resuming the live wallpaper.
                setPreserveEGLContextOnPause(true);

                // handle prefs, other initialization
                // Set the renderer to our user-defined renderer.
                renderer = new Live2DGLRenderer(getApplicationContext());
                setRenderer(renderer);
            }
        }

        @Override
        public void onTouchEvent(MotionEvent event) {
            float pointX = event.getX();
            float pointY = event.getY();
            switch (event.getAction()) {
                case MotionEvent.ACTION_DOWN:
                    JniBridgeJava.nativeOnTouchesBegan(pointX, pointY);
                    break;
                case MotionEvent.ACTION_UP:
                    JniBridgeJava.nativeOnTouchesEnded(pointX, pointY);
                    break;
                case MotionEvent.ACTION_MOVE:
                    JniBridgeJava.nativeOnTouchesMoved(pointX, pointY);
                    break;
            }
        }

        public void onDestroy() {
            super.onDestroy();
            JniBridgeJava.nativeOnDestroy();

            if (renderer != null) {
                renderer.release();
            }
            renderer = null;
        }
    }
}
