/*
 * Copyright(c) Live2D Inc. All rights reserved.
 * <p>
 * Use of this source code is governed by the Live2D Open Software license
 * that can be found at https://www.live2d.com/eula/live2d-open-software-license-agreement_en.html.
 */

package com.live2d.demo;

import android.view.MotionEvent;
import net.rbgrn.android.glwallpaperservice.*;

public class LiveWallpaper extends GLWallpaperService {

    public LiveWallpaper() {
        super();
    }

    public Engine onCreateEngine() {
        Live2DWallpaperEngine engine = new Live2DWallpaperEngine();
        return engine;
    }

    class Live2DWallpaperEngine extends GLEngine {
        Live2DGLRenderer renderer;

        public Live2DWallpaperEngine() {
            super();

            JniBridgeJava.SetContext(getApplicationContext());
            JniBridgeJava.nativeOnStart();

            // handle prefs, other initialization
            renderer = new Live2DGLRenderer(getApplicationContext());
            setRenderer(renderer);
            setRenderMode(RENDERMODE_CONTINUOUSLY);
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

            /* そのまま呼び出すと
            * call to OpenGL ES API with no current context (logged once per thread)
            * が発生する。
            * 公式リファレンス↓
            * https://developer.android.com/reference/android/opengl/GLSurfaceView.html#handling-events
            */
             queueEvent(new Runnable() {
                public void run() {
                    if (renderer != null) {
                        renderer.release();
                    }
                    renderer = null;
                }
            });
        }
    }
}
