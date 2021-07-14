/*
 * Copyright(c) Live2D Inc. All rights reserved.
 * <p>
 * Use of this source code is governed by the Live2D Open Software license
 * that can be found at https://www.live2d.com/eula/live2d-open-software-license-agreement_en.html.
 */

package com.live2d.demo;

import android.content.Context;
import android.opengl.GLES20;
import android.opengl.GLSurfaceView;

import javax.microedition.khronos.egl.EGLConfig;
import javax.microedition.khronos.opengles.GL10;
import net.rbgrn.android.glwallpaperservice.*;

import static android.opengl.GLES20.*;

public class Live2DGLRenderer implements GLSurfaceView.Renderer {
    Context con;

    public Live2DGLRenderer(Context context)
    {
        con = context;
    }

    public void onSurfaceCreated(GL10 gl, EGLConfig config) {
        System.out.println("ここ");
        //バーテックスシェーダのコンパイル
        int vertexShaderId = GLES20.glCreateShader(GLES20.GL_VERTEX_SHADER);
        System.out.println("通過");
    String vertexShader =
            "#version 100\n"+
                    "attribute vec3 position;"+
                    "attribute vec2 uv;"+
                    "varying vec2 vuv;"+
                    "void main(void){"+
                    "    gl_Position = vec4(position, 1.0);"+
                    "vuv = uv;"+
                    "}";
        GLES20.glShaderSource(vertexShaderId,vertexShader);
        GLES20.glCompileShader(vertexShaderId);

        //フラグメントシェーダのコンパイル
        int fragmentShaderId = GLES20.glCreateShader(GLES20.GL_FRAGMENT_SHADER);
        String fragmentShader =
                "#version 100\n"+
        "precision mediump float;"+
        "varying vec2 vuv;"+
        "uniform sampler2D texture;"+
        "uniform vec4 baseColor;"+
        "void main(void){"+
        "    gl_FragColor = texture2D(texture, vuv) * baseColor;"+
        "}";
        GLES20.glShaderSource(fragmentShaderId,fragmentShader);
        GLES20.glCompileShader(fragmentShaderId);

        //プログラムオブジェクトの作成
        int programId = GLES20.glCreateProgram();
        GLES20.glAttachShader(programId, vertexShaderId);
        GLES20.glAttachShader(programId, fragmentShaderId);

        // リンク
        GLES20.glLinkProgram(programId);

        GLES20.glUseProgram(programId);
        JniBridgeJava.nativeOnSurfaceCreated(programId);
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
