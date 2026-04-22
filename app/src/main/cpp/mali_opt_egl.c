#include <jni.h>
#include <string.h>
#include <EGL/egl.h>
#include <android/log.h>

#define LOG_TAG "MaliOptEGL"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)

static int check_egl_extension(const char* ext_name) {
    EGLDisplay dpy = eglGetCurrentDisplay();
    if (dpy == EGL_NO_DISPLAY) return 0;
    const char* exts = eglQueryString(dpy, EGL_EXTENSIONS);
    if (!exts) return 0;
    return (strstr(exts, ext_name) != NULL) ? 1 : 0;
}

JNIEXPORT jboolean JNICALL
Java_com_hybridcore_maliopt_MaliOptNative_isFenceSyncSupported(JNIEnv *env, jobject thiz) {
    return (jboolean)check_egl_extension("EGL_KHR_fence_sync");
}

JNIEXPORT jboolean JNICALL
Java_com_hybridcore_maliopt_MaliOptNative_isNativeFenceSyncSupported(JNIEnv *env, jobject thiz) {
    return (jboolean)check_egl_extension("EGL_ANDROID_native_fence_sync");
}

JNIEXPORT jboolean JNICALL
Java_com_hybridcore_maliopt_MaliOptNative_isWaitSyncSupported(JNIEnv *env, jobject thiz) {
    return (jboolean)check_egl_extension("EGL_KHR_wait_sync");
}
