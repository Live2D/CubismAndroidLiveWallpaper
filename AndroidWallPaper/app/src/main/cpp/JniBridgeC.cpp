/**
 * Copyright(c) Live2D Inc. All rights reserved.
 *
 * Use of this source code is governed by the Live2D Open Software license
 * that can be found at https://www.live2d.com/eula/live2d-open-software-license-agreement_en.html.
 */

#include <jni.h>
#include "JniBridgeC.hpp"
#include "LWallpaperDelegate.hpp"
#include "LWallpaperPal.hpp"

using namespace Csm;

static JavaVM* g_JVM; // JavaVM is valid for all threads, so just save it globally
static jclass  g_JniBridgeJavaClass;
static jmethodID g_LoadFileMethodId;

JNIEnv* GetEnv()
{
    JNIEnv* env = NULL;
    g_JVM->GetEnv(reinterpret_cast<void **>(&env), JNI_VERSION_1_6);
    return env;
}

// The VM calls JNI_OnLoad when the native library is loaded
jint JNICALL JNI_OnLoad(JavaVM* vm, void* reserved)
{
    g_JVM = vm;

    JNIEnv *env;
    if (vm->GetEnv(reinterpret_cast<void **>(&env), JNI_VERSION_1_6) != JNI_OK)
    {
        return JNI_ERR;
    }

    jclass clazz = env->FindClass("com/live2d/demo/JniBridgeJava");
    g_JniBridgeJavaClass = reinterpret_cast<jclass>(env->NewGlobalRef(clazz));
    g_LoadFileMethodId = env->GetStaticMethodID(g_JniBridgeJavaClass, "LoadFile", "(Ljava/lang/String;)[B");

    return JNI_VERSION_1_6;
}

void JNICALL JNI_OnUnload(JavaVM *vm, void *reserved)
{
    JNIEnv *env = GetEnv();
    env->DeleteGlobalRef(g_JniBridgeJavaClass);
}

char* JniBridgeC::LoadFileAsBytesFromJava(const char* filePath, unsigned int* outSize)
{
    JNIEnv *env = GetEnv();

    // ファイルロード
    jbyteArray obj = (jbyteArray)env->CallStaticObjectMethod(g_JniBridgeJavaClass, g_LoadFileMethodId, env->NewStringUTF(filePath));
    *outSize = static_cast<unsigned int>(env->GetArrayLength(obj));

    char* buffer = new char[*outSize];
    env->GetByteArrayRegion(obj, 0, *outSize, reinterpret_cast<jbyte *>(buffer));

    return buffer;
}

extern "C"
{
    JNIEXPORT void JNICALL
    Java_com_live2d_demo_JniBridgeJava_nativeOnStart(JNIEnv *env, jclass type)
    {
        LWallpaperDelegate::GetInstance()->OnStart();
    }

    JNIEXPORT void JNICALL
    Java_com_live2d_demo_JniBridgeJava_nativeOnPause(JNIEnv *env, jclass type)
    {
        LWallpaperDelegate::GetInstance()->OnPause();
    }

    JNIEXPORT void JNICALL
    Java_com_live2d_demo_JniBridgeJava_nativeOnStop(JNIEnv *env, jclass type)
    {
        LWallpaperDelegate::GetInstance()->OnStop();
    }

    JNIEXPORT void JNICALL
    Java_com_live2d_demo_JniBridgeJava_nativeOnDestroy(JNIEnv *env, jclass type)
    {
        LWallpaperDelegate::GetInstance()->OnDestroy();
    }

    JNIEXPORT void JNICALL
    Java_com_live2d_demo_JniBridgeJava_nativeOnSurfaceCreated(JNIEnv *env, jclass type)
    {

    }

    JNIEXPORT void JNICALL
    Java_com_live2d_demo_JniBridgeJava_nativeOnSurfaceChanged(JNIEnv *env, jclass type, jint width, jint height)
    {

    }

    JNIEXPORT void JNICALL
    Java_com_live2d_demo_JniBridgeJava_nativeOnDrawFrame(JNIEnv *env, jclass type)
    {

    }

    JNIEXPORT void JNICALL
    Java_com_live2d_demo_JniBridgeJava_nativeOnTouchesBegan(JNIEnv *env, jclass type, jfloat pointX, jfloat pointY)
    {
        LWallpaperDelegate::GetInstance()->OnTouchBegan(pointX, pointY);
    }

    JNIEXPORT void JNICALL
    Java_com_live2d_demo_JniBridgeJava_nativeOnTouchesEnded(JNIEnv *env, jclass type, jfloat pointX, jfloat pointY)
    {
        LWallpaperDelegate::GetInstance()->OnTouchEnded(pointX, pointY);
    }

    JNIEXPORT void JNICALL
    Java_com_live2d_demo_JniBridgeJava_nativeOnTouchesMoved(JNIEnv *env, jclass type, jfloat pointX, jfloat pointY)
    {
        LWallpaperDelegate::GetInstance()->OnTouchMoved(pointX, pointY);
    }

    JNIEXPORT void JNICALL
    Java_com_live2d_demo_JniBridgeJava_nativeStartRandomMotion(JNIEnv *env, jclass type)
    {

    }

    JNIEXPORT void JNICALL
    Java_com_live2d_demo_JniBridgeJava_nativeStartMotion(JNIEnv *env, jclass type, jint index)
    {
        LWallpaperDelegate::GetInstance()->StartMotion(index);
    }

    JNIEXPORT void JNICALL
    Java_com_live2d_demo_JniBridgeJava_nativeSetClearColor(JNIEnv *env, jclass clazz, jfloat r, jfloat g, jfloat b)
    {
        LWallpaperDelegate::GetInstance()->SetClearColor(r, g, b);
    }

    JNIEXPORT void JNICALL
    Java_com_live2d_demo_JniBridgeJava_SetBackGroundSpriteAlpha(JNIEnv *env, jclass clazz, jfloat a)
    {
        LWallpaperDelegate::GetInstance()->SetBackGroundSpriteAlpha(a);
    }

    JNIEXPORT void JNICALL
    Java_com_live2d_demo_JniBridgeJava_SetGravitationalAccelerationX(JNIEnv *env, jclass clazz, jfloat gravity)
    {
        LWallpaperDelegate::GetInstance()->SetGravitationalAccelerationX(gravity);
    }
}
