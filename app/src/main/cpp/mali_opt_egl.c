/*
 * mali_opt_egl.c
 * Responsavel por verificacoes adicionais via EGL.
 * Atualmente as queries EGL sao feitas dentro de mali_opt_extensions.c.
 * Este arquivo existe para expansoes futuras (fence sync, image, etc).
 */

#include <jni.h>
#include <EGL/egl.h>
#include <android/log.h>

#define LOG_TAG "MaliOptEGL"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)

/*
 * Retorna 1 se uma extensao EGL estiver presente, 0 caso contrario.
 */
static int check_egl_extension(const char* ext_name) {
    EGLDisplay dpy = eglGetCurrentDisplay();
    if (dpy == EGL_NO_DISPLAY) return 0;
    const char* exts = eglQueryString(dpy, EGL_EXTENSIONS);
    if (!exts) return 0;
    return (strstr(exts, ext_name) != NULL) ? 1 : 0;
}

/*
 * Verifica suporte a fence sync (importante para evitar texture pop-in).
 */
JNIEXPORT jboolean JNICALL
Java_com_hybridcore_maliopt_MaliOptNative_isFenceSyncSupported(JNIEnv *env, jobject thiz) {
    return (jboolean)check_egl_extension("EGL_KHR_fence_sync");
}

/*
 * Verifica suporte a native fence (integracao com Android fence FD).
 */
JNIEXPORT jboolean JNICALL
Java_com_hybridcore_maliopt_MaliOptNative_isNativeFenceSyncSupported(JNIEnv *env, jobject thiz) {
    return (jboolean)check_egl_extension("EGL_ANDROID_native_fence_sync");
}

/*
 * Verifica suporte a wait sync (CPU pode esperar GPU sem polling).
 */
JNIEXPORT jboolean JNICALL
Java_com_hybridcore_maliopt_MaliOptNative_isWaitSyncSupported(JNIEnv *env, jobject thiz) {
    return (jboolean)check_egl_extension("EGL_KHR_wait_sync");
}
