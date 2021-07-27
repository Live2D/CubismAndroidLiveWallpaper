/*
 * Copyright (C) 2008 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

package net.rbgrn.android.glwallpaperservice;

import java.io.Writer;
import java.util.ArrayList;

import javax.microedition.khronos.egl.EGL10;
import javax.microedition.khronos.egl.EGL11;
import javax.microedition.khronos.egl.EGLConfig;
import javax.microedition.khronos.egl.EGLContext;
import javax.microedition.khronos.egl.EGLDisplay;
import javax.microedition.khronos.egl.EGLSurface;
import javax.microedition.khronos.opengles.GL;
import javax.microedition.khronos.opengles.GL10;

import net.rbgrn.android.glwallpaperservice.BaseConfigChooser.ComponentSizeChooser;
import net.rbgrn.android.glwallpaperservice.BaseConfigChooser.SimpleEGLConfigChooser;

import android.content.Context;
import android.opengl.GLSurfaceView;
import android.service.wallpaper.WallpaperService;
import android.util.Log;
import android.view.SurfaceHolder;

// Original code provided by Robert Green
// http://www.rbgrn.net/content/354-glsurfaceview-adapted-3d-live-wallpapers
public class GLWallpaperService extends WallpaperService {
	private static final String TAG = "GLWallpaperService";

	@Override
	public Engine onCreateEngine() {
		return new GLEngine();
	}

	public class GLEngine extends Engine {
		private static final String TAG = "GLEngine";
		public final static int RENDERMODE_WHEN_DIRTY = 0;
		public final static int RENDERMODE_CONTINUOUSLY = 1;

		private GLSurfaceView.EGLConfigChooser mEGLConfigChooser;
		private GLSurfaceView.EGLContextFactory mEGLContextFactory;
		private GLSurfaceView.EGLWindowSurfaceFactory mEGLWindowSurfaceFactory;
		private GLSurfaceView.GLWrapper mGLWrapper;

		private WallpaperGLSurfaceView glSurfaceView;
		private boolean rendererHasBeenSet;

		public GLEngine() {
			super();
		}

		@Override
		public void onVisibilityChanged(boolean visible) {
			super.onVisibilityChanged(visible);
			if (visible) {
				onResume();
			} else {
				onPause();
			}
		}

		@Override
		public void onCreate(SurfaceHolder surfaceHolder) {
			super.onCreate(surfaceHolder);
			glSurfaceView = new WallpaperGLSurfaceView(GLWallpaperService.this);
		}

		@Override
		public void onDestroy() {
			super.onDestroy();
			// Log.d(TAG, "GLEngine.onDestroy()");
			glSurfaceView.onDestroy();
		}

		@Override
		public void onSurfaceChanged(SurfaceHolder holder, int format, int width, int height) {
			// Log.d(TAG, "onSurfaceChanged()");
			super.onSurfaceChanged(holder, format, width, height);
		}

		@Override
		public void onSurfaceCreated(SurfaceHolder holder) {
			Log.d(TAG, "onSurfaceCreated()");
			super.onSurfaceCreated(holder);
		}

		@Override
		public void onSurfaceDestroyed(SurfaceHolder holder) {
			Log.d(TAG, "onSurfaceDestroyed()");
			super.onSurfaceDestroyed(holder);
		}

		/**
		 * An EGL helper class.
		 */
		public void setGLWrapper(GLSurfaceView.GLWrapper glWrapper) {
			mGLWrapper = glWrapper;
		}

		protected void setRenderer(GLSurfaceView.Renderer renderer) {
			glSurfaceView.setRenderer(renderer);
			rendererHasBeenSet = true;
		}

		protected void setEGLContextClientVersion(int version) {
			glSurfaceView.setEGLContextClientVersion(version);
		}

		protected void setPreserveEGLContextOnPause(boolean preserve) {
			glSurfaceView.setPreserveEGLContextOnPause(preserve);
		}

		public void setEGLContextFactory(GLSurfaceView.EGLContextFactory factory) {
			mEGLContextFactory = factory;
		}

		public void setEGLWindowSurfaceFactory(GLSurfaceView.EGLWindowSurfaceFactory factory) {
			mEGLWindowSurfaceFactory = factory;
		}

		public void setEGLConfigChooser(GLSurfaceView.EGLConfigChooser configChooser) {
			mEGLConfigChooser = configChooser;
		}

		public void setEGLConfigChooser(boolean needDepth) {
			setEGLConfigChooser(new SimpleEGLConfigChooser(needDepth));
		}

		public void setEGLConfigChooser(int redSize, int greenSize, int blueSize, int alphaSize, int depthSize,
				int stencilSize) {
			setEGLConfigChooser(new ComponentSizeChooser(redSize, greenSize, blueSize, alphaSize, depthSize,
					stencilSize));
		}

		public void onPause() {
			glSurfaceView.onPause();
		}

		public void onResume() {
			glSurfaceView.onResume();
		}

		class WallpaperGLSurfaceView extends GLSurfaceView {
			private static final String TAG = "WallpaperGLSurfaceView";

			WallpaperGLSurfaceView(Context context) {
				super(context);
			}

			@Override
			public SurfaceHolder getHolder() {
				return getSurfaceHolder();
			}

			public void onDestroy() {
				super.onDetachedFromWindow();
			}
		}
	}
}

class LogWriter extends Writer {
	private StringBuilder mBuilder = new StringBuilder();

	@Override
	public void close() {
		flushBuilder();
	}

	@Override
	public void flush() {
		flushBuilder();
	}

	@Override
	public void write(char[] buf, int offset, int count) {
		for (int i = 0; i < count; i++) {
			char c = buf[offset + i];
			if (c == '\n') {
				flushBuilder();
			} else {
				mBuilder.append(c);
			}
		}
	}

	private void flushBuilder() {
		if (mBuilder.length() > 0) {
			Log.v("GLSurfaceView", mBuilder.toString());
			mBuilder.delete(0, mBuilder.length());
		}
	}
}

class DefaultContextFactory implements GLSurfaceView.EGLContextFactory {

	public EGLContext createContext(EGL10 egl, EGLDisplay display, EGLConfig config) {
		return egl.eglCreateContext(display, config, EGL10.EGL_NO_CONTEXT, null);
	}

	public void destroyContext(EGL10 egl, EGLDisplay display, EGLContext context) {
		egl.eglDestroyContext(display, context);
	}
}

class DefaultWindowSurfaceFactory implements GLSurfaceView.EGLWindowSurfaceFactory {

	public EGLSurface createWindowSurface(EGL10 egl, EGLDisplay
			display, EGLConfig config, Object nativeWindow) {
		// this is a bit of a hack to work around Droid init problems - if you don't have this, it'll get hung up on orientation changes
		EGLSurface eglSurface = null;
		while (eglSurface == null) {
			try {
				eglSurface = egl.eglCreateWindowSurface(display,
						config, nativeWindow, null);
			} catch (Throwable t) {
			} finally {
				if (eglSurface == null) {
					try {
						Thread.sleep(10);
					} catch (InterruptedException t) {
					}
				}
			}
		}
		return eglSurface;
	}

	public void destroySurface(EGL10 egl, EGLDisplay display, EGLSurface surface) {
		egl.eglDestroySurface(display, surface);
	}
}

class EglHelper {

	private EGL10 mEgl;
	private EGLDisplay mEglDisplay;
	private EGLSurface mEglSurface;
	private EGLContext mEglContext;
	EGLConfig mEglConfig;

	private GLSurfaceView.EGLConfigChooser mEGLConfigChooser;
	private GLSurfaceView.EGLContextFactory mEGLContextFactory;
	private GLSurfaceView.EGLWindowSurfaceFactory mEGLWindowSurfaceFactory;
	private GLSurfaceView.GLWrapper mGLWrapper;

	public EglHelper(GLSurfaceView.EGLConfigChooser chooser, GLSurfaceView.EGLContextFactory contextFactory,
			GLSurfaceView.EGLWindowSurfaceFactory surfaceFactory, GLSurfaceView.GLWrapper wrapper) {
		this.mEGLConfigChooser = chooser;
		this.mEGLContextFactory = contextFactory;
		this.mEGLWindowSurfaceFactory = surfaceFactory;
		this.mGLWrapper = wrapper;
	}

	/**
	 * Initialize EGL for a given configuration spec.
	 *
	 *
	 */
	public void start() {
		// Log.d("EglHelper" + instanceId, "start()");
		if (mEgl == null) {
			// Log.d("EglHelper" + instanceId, "getting new EGL");
			/*
			 * Get an EGL instance
			 */
			mEgl = (EGL10) EGLContext.getEGL();
		} else {
			// Log.d("EglHelper" + instanceId, "reusing EGL");
		}

		if (mEglDisplay == null) {
			// Log.d("EglHelper" + instanceId, "getting new display");
			/*
			 * Get to the default display.
			 */
			mEglDisplay = mEgl.eglGetDisplay(EGL10.EGL_DEFAULT_DISPLAY);
		} else {
			// Log.d("EglHelper" + instanceId, "reusing display");
		}

		if (mEglConfig == null) {
			// Log.d("EglHelper" + instanceId, "getting new config");
			/*
			 * We can now initialize EGL for that display
			 */
			int[] version = new int[2];
			mEgl.eglInitialize(mEglDisplay, version);
			mEglConfig = mEGLConfigChooser.chooseConfig(mEgl, mEglDisplay);
		} else {
			// Log.d("EglHelper" + instanceId, "reusing config");
		}

		if (mEglContext == null) {
			// Log.d("EglHelper" + instanceId, "creating new context");
			/*
			 * Create an OpenGL ES context. This must be done only once, an OpenGL context is a somewhat heavy object.
			 */
			mEglContext = mEGLContextFactory.createContext(mEgl, mEglDisplay, mEglConfig);
			if (mEglContext == null || mEglContext == EGL10.EGL_NO_CONTEXT) {
				throw new RuntimeException("createContext failed");
			}
		} else {
			// Log.d("EglHelper" + instanceId, "reusing context");
		}

		mEglSurface = null;
	}

	/*
	 * React to the creation of a new surface by creating and returning an OpenGL interface that renders to that
	 * surface.
	 */
	public GL createSurface(SurfaceHolder holder) {
		/*
		 * The window size has changed, so we need to create a new surface.
		 */
		if (mEglSurface != null && mEglSurface != EGL10.EGL_NO_SURFACE) {

			/*
			 * Unbind and destroy the old EGL surface, if there is one.
			 */
			mEgl.eglMakeCurrent(mEglDisplay, EGL10.EGL_NO_SURFACE, EGL10.EGL_NO_SURFACE, EGL10.EGL_NO_CONTEXT);
			mEGLWindowSurfaceFactory.destroySurface(mEgl, mEglDisplay, mEglSurface);
		}

		/*
		 * Create an EGL surface we can render into.
		 */
		mEglSurface = mEGLWindowSurfaceFactory.createWindowSurface(mEgl, mEglDisplay, mEglConfig, holder);

		if (mEglSurface == null || mEglSurface == EGL10.EGL_NO_SURFACE) {
			throw new RuntimeException("createWindowSurface failed");
		}

		/*
		 * Before we can issue GL commands, we need to make sure the context is current and bound to a surface.
		 */
		if (!mEgl.eglMakeCurrent(mEglDisplay, mEglSurface, mEglSurface, mEglContext)) {
			throw new RuntimeException("eglMakeCurrent failed.");
		}

		GL gl = mEglContext.getGL();
		if (mGLWrapper != null) {
			gl = mGLWrapper.wrap(gl);
		}

		/*
		 * if ((mDebugFlags & (DEBUG_CHECK_GL_ERROR | DEBUG_LOG_GL_CALLS))!= 0) { int configFlags = 0; Writer log =
		 * null; if ((mDebugFlags & DEBUG_CHECK_GL_ERROR) != 0) { configFlags |= GLDebugHelper.CONFIG_CHECK_GL_ERROR; }
		 * if ((mDebugFlags & DEBUG_LOG_GL_CALLS) != 0) { log = new LogWriter(); } gl = GLDebugHelper.wrap(gl,
		 * configFlags, log); }
		 */
		return gl;
	}

	/**
	 * Display the current render surface.
	 *
	 * @return false if the context has been lost.
	 */
	public boolean swap() {
		mEgl.eglSwapBuffers(mEglDisplay, mEglSurface);

		/*
		 * Always check for EGL_CONTEXT_LOST, which means the context and all associated data were lost (For instance
		 * because the device went to sleep). We need to sleep until we get a new surface.
		 */
		return mEgl.eglGetError() != EGL11.EGL_CONTEXT_LOST;
	}

	public void destroySurface() {
		if (mEglSurface != null && mEglSurface != EGL10.EGL_NO_SURFACE) {
			mEgl.eglMakeCurrent(mEglDisplay, EGL10.EGL_NO_SURFACE, EGL10.EGL_NO_SURFACE, EGL10.EGL_NO_CONTEXT);
			mEGLWindowSurfaceFactory.destroySurface(mEgl, mEglDisplay, mEglSurface);
			mEglSurface = null;
		}
	}

	public void finish() {
		if (mEglContext != null) {
			mEGLContextFactory.destroyContext(mEgl, mEglDisplay, mEglContext);
			mEglContext = null;
		}
		if (mEglDisplay != null) {
			mEgl.eglTerminate(mEglDisplay);
			mEglDisplay = null;
		}
	}
}

abstract class BaseConfigChooser implements GLSurfaceView.EGLConfigChooser {
	public BaseConfigChooser(int[] configSpec) {
		mConfigSpec = configSpec;
	}

	public EGLConfig chooseConfig(EGL10 egl, EGLDisplay display) {
		int[] num_config = new int[1];
		egl.eglChooseConfig(display, mConfigSpec, null, 0, num_config);

		int numConfigs = num_config[0];

		if (numConfigs <= 0) {
			throw new IllegalArgumentException("No configs match configSpec");
		}

		EGLConfig[] configs = new EGLConfig[numConfigs];
		egl.eglChooseConfig(display, mConfigSpec, configs, numConfigs, num_config);
		EGLConfig config = chooseConfig(egl, display, configs);
		if (config == null) {
			throw new IllegalArgumentException("No config chosen");
		}
		return config;
	}

	abstract EGLConfig chooseConfig(EGL10 egl, EGLDisplay display, EGLConfig[] configs);

	protected int[] mConfigSpec;
	public static class ComponentSizeChooser extends BaseConfigChooser {
		public ComponentSizeChooser(int redSize, int greenSize, int blueSize, int alphaSize, int depthSize,
				int stencilSize) {
			super(new int[] { EGL10.EGL_RED_SIZE, redSize, EGL10.EGL_GREEN_SIZE, greenSize, EGL10.EGL_BLUE_SIZE,
					blueSize, EGL10.EGL_ALPHA_SIZE, alphaSize, EGL10.EGL_DEPTH_SIZE, depthSize, EGL10.EGL_STENCIL_SIZE,
					stencilSize, EGL10.EGL_NONE });
			mValue = new int[1];
			mRedSize = redSize;
			mGreenSize = greenSize;
			mBlueSize = blueSize;
			mAlphaSize = alphaSize;
			mDepthSize = depthSize;
			mStencilSize = stencilSize;
		}

		@Override
		public EGLConfig chooseConfig(EGL10 egl, EGLDisplay display, EGLConfig[] configs) {
			EGLConfig closestConfig = null;
			int closestDistance = 1000;
			for (EGLConfig config : configs) {
				int d = findConfigAttrib(egl, display, config, EGL10.EGL_DEPTH_SIZE, 0);
				int s = findConfigAttrib(egl, display, config, EGL10.EGL_STENCIL_SIZE, 0);
				if (d >= mDepthSize && s >= mStencilSize) {
					int r = findConfigAttrib(egl, display, config, EGL10.EGL_RED_SIZE, 0);
					int g = findConfigAttrib(egl, display, config, EGL10.EGL_GREEN_SIZE, 0);
					int b = findConfigAttrib(egl, display, config, EGL10.EGL_BLUE_SIZE, 0);
					int a = findConfigAttrib(egl, display, config, EGL10.EGL_ALPHA_SIZE, 0);
					int distance = Math.abs(r - mRedSize) + Math.abs(g - mGreenSize) + Math.abs(b - mBlueSize)
					+ Math.abs(a - mAlphaSize);
					if (distance < closestDistance) {
						closestDistance = distance;
						closestConfig = config;
					}
				}
			}
			return closestConfig;
		}

		private int findConfigAttrib(EGL10 egl, EGLDisplay display, EGLConfig config, int attribute, int defaultValue) {

			if (egl.eglGetConfigAttrib(display, config, attribute, mValue)) {
				return mValue[0];
			}
			return defaultValue;
		}

		private int[] mValue;
		// Subclasses can adjust these values:
		protected int mRedSize;
		protected int mGreenSize;
		protected int mBlueSize;
		protected int mAlphaSize;
		protected int mDepthSize;
		protected int mStencilSize;
	}

	/**
	 * This class will choose a supported surface as close to RGB565 as possible, with or without a depth buffer.
	 *
	 */
	public static class SimpleEGLConfigChooser extends ComponentSizeChooser {
		public SimpleEGLConfigChooser(boolean withDepthBuffer) {
			super(4, 4, 4, 0, withDepthBuffer ? 16 : 0, 0);
			// Adjust target values. This way we'll accept a 4444 or
			// 555 buffer if there's no 565 buffer available.
			mRedSize = 5;
			mGreenSize = 6;
			mBlueSize = 5;
		}
	}
}