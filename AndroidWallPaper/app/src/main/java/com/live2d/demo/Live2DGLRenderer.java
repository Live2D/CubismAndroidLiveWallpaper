/*
 * Copyright(c) Live2D Inc. All rights reserved.
 *
 * Use of this source code is governed by the Live2D Open Software license
 * that can be found at https://www.live2d.com/eula/live2d-open-software-license-agreement_en.html.
 */

package com.live2d.demo;

import android.content.Context;
import android.opengl.GLSurfaceView;

import javax.microedition.khronos.egl.EGLConfig;
import javax.microedition.khronos.opengles.GL10;

public class Live2DGLRenderer implements GLSurfaceView.Renderer {
    Context con;

    public Live2DGLRenderer(Context context)
    {
        con = context;
    }

    public void onSurfaceCreated(GL10 gl, EGLConfig config) {
        JniBridgeJava.nativeOnSurfaceCreated();
    }

    public void onSurfaceChanged(GL10 gl, int width, int height) {
        JniBridgeJava.nativeOnSurfaceChanged(width, height);
    }

    public void onDrawFrame(GL10 gl) {
        JniBridgeJava.nativeOnDrawFrame();
    }

    /**
     * Called when the engine is destroyed. Do any necessary clean up because
     * at this point your renderer instance is now done for.
     */
    public void release() {
    }
}
