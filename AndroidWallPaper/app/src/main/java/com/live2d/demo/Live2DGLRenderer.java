/*
 * Copyright(c) Live2D Inc. All rights reserved.
 * <p>
 * Use of this source code is governed by the Live2D Open Software license
 * that can be found at https://www.live2d.com/eula/live2d-open-software-license-agreement_en.html.
 */

package com.live2d.demo;

import android.content.Context;
import android.opengl.GLSurfaceView;
import javax.microedition.khronos.egl.EGLConfig;
import javax.microedition.khronos.opengles.GL10;
import net.rbgrn.android.glwallpaperservice.*;

public class Live2DGLRenderer implements GLWallpaperService.Renderer {
    Context con;

    public Live2DGLRenderer(Context context)
    {
        con = context;
        JniBridgeJava.SetContext(con);
        JniBridgeJava.nativeOnStart();
    }

    public void onSurfaceCreated(GL10 gl, EGLConfig config) {
        JniBridgeJava.nativeOnSurfaceCreated();
    }

    public void onSurfaceChanged(GL10 gl, int width, int height) {
        JniBridgeJava.nativeOnSurfaceChanged(width, height);
    }

    public void onDrawFrame(GL10 gl) {
        gl.glMatrixMode(GL10.GL_MODELVIEW ) ;
        gl.glLoadIdentity() ;
        gl.glClear( GL10.GL_COLOR_BUFFER_BIT ) ;
        gl.glEnable( GL10.GL_BLEND ) ;
        gl.glBlendFunc( GL10.GL_ONE , GL10.GL_ONE_MINUS_SRC_ALPHA ) ;
        gl.glDisable( GL10.GL_DEPTH_TEST ) ;
        gl.glDisable( GL10.GL_CULL_FACE ) ;

        JniBridgeJava.nativeOnDrawFrame();
    }

    /**
     * Called when the engine is destroyed. Do any necessary clean up because
     * at this point your renderer instance is now done for.
     */
    public void release() {
    }
}
