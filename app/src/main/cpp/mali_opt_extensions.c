#include <jni.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <EGL/egl.h>
#include <GLES3/gl3.h>
#include <GLES2/gl2ext.h>
#include <android/log.h>

#define LOG_TAG  "MaliOptPlugin"
#define LOGI(...)  __android_log_print(ANDROID_LOG_INFO,  LOG_TAG, __VA_ARGS__)
#define LOGW(...)  __android_log_print(ANDROID_LOG_WARN,  LOG_TAG, __VA_ARGS__)
#define LOGE(...)  __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)

#ifndef GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT
#define GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT 0x84FF
#endif
#ifndef GL_TEXTURE_MAX_ANISOTROPY_EXT
#define GL_TEXTURE_MAX_ANISOTROPY_EXT 0x84FE
#endif

typedef struct {
    bool FB_FETCH;
    bool FB_FETCH_DEPTH_STENCIL;
    bool PLS;
    bool MALI_BINARY;
    bool ASTC_LDR;
    bool ASTC_HDR;
    bool ETC2;
    bool STANDARD_DERIVATIVES;
    bool TEXTURE_FLOAT;
    bool TEXTURE_FLOAT_LINEAR;
    bool TEXTURE_HALF_FLOAT;
    bool TEXTURE_HALF_FLOAT_LINEAR;
    bool MULTISAMPLED_RT;
    bool MULTISAMPLED_RT2;
    bool SRGB;
    bool SRGB_WRITE_CONTROL;
    bool ANISOTROPIC;
    bool PROGRAM_BINARY;
    bool TEXTURE_STORAGE;
    bool TEXTURE_NPOT;
    bool MAP_BUFFER;
    bool BUFFER_STORAGE;
    bool TIMER_QUERY;
    bool DEBUG_MARKER;
    bool KHR_DEBUG;
    bool ROBUSTNESS;
    GLint MAX_TEXTURE_SIZE;
    GLint MAX_SAMPLES;
    GLint MAX_VERTEX_ATTRIBS;
    GLint MAX_COMBINED_TIU;
    GLint MAX_ANISOTROPY;
    char GPU_RENDERER[256];
    char GPU_VENDOR[128];
    char GLES_VERSION[128];
} MaliCapabilities;

static MaliCapabilities g_caps;
static bool             g_initialized       = false;
static char             g_context[32]       = "UNKNOWN";
static char             g_raw_gl_ext[65536] = "";
static char             g_raw_egl_ext[8192] = "";

static bool check_extension(const char* ext) {
    return (strstr(g_raw_gl_ext, ext) != NULL);
}

static void detect_context(void) {
    FILE* maps = fopen("/proc/self/maps", "r");
    if (!maps) { strcpy(g_context, "UNKNOWN"); return; }
    char line[512];
    while (fgets(line, sizeof(line), maps)) {
        if (strstr(line, "libgl4es")) {
            strcpy(g_context, "GL4ES");
            fclose(maps); return;
        }
        if (strstr(line, "libEGL_angle") || strstr(line, "libGLESv2_angle")) {
            strcpy(g_context, "ANGLE");
            fclose(maps); return;
        }
    }
    fclose(maps);
    strcpy(g_context, "NATIVE");
}

// Força as capacidades conhecidas do Mali-G52 (Bifrost) quando o driver as omite
static void apply_bifrost_hardware_knowledge(void) {
    if (strstr(g_caps.GPU_RENDERER, "Mali-G52") == NULL) return;

    LOGI("Mali-G52 detectado. Aplicando conhecimento de hardware Bifrost.");

    if (!g_caps.FB_FETCH) {
        g_caps.FB_FETCH = true;
        LOGI("  [FORCE] FB_FETCH = true");
    }
    if (!g_caps.FB_FETCH_DEPTH_STENCIL) {
        g_caps.FB_FETCH_DEPTH_STENCIL = true;
        LOGI("  [FORCE] FB_FETCH_DEPTH_STENCIL = true");
    }
    if (!g_caps.PLS) {
        g_caps.PLS = true;
        LOGI("  [FORCE] PLS = true");
    }
    if (!g_caps.MALI_BINARY) {
        g_caps.MALI_BINARY = true;
        LOGI("  [FORCE] MALI_BINARY = true");
    }
    if (!g_caps.PROGRAM_BINARY) {
        g_caps.PROGRAM_BINARY = true;
        LOGI("  [FORCE] PROGRAM_BINARY = true");
    }
    if (!g_caps.TEXTURE_STORAGE) {
        g_caps.TEXTURE_STORAGE = true;
        LOGI("  [FORCE] TEXTURE_STORAGE = true");
    }
    if (!g_caps.BUFFER_STORAGE) {
        g_caps.BUFFER_STORAGE = true;
        LOGI("  [FORCE] BUFFER_STORAGE = true");
    }
}

// ═══════════════════════════════════════════════════════════════════════════
// TODAS AS FUNÇÕES JNI COM PREFIXO CORRETO: Java_com_maliopt_MaliOptNative_
// ═══════════════════════════════════════════════════════════════════════════

JNIEXPORT void JNICALL
Java_com_maliopt_MaliOptNative_detectExtensions(JNIEnv *env, jobject thiz) {
    if (g_initialized) return;

    LOGI("========== MaliOpt Plugin: Iniciando deteccao ==========");

    detect_context();
    LOGI("Contexto: %s", g_context);

    const char* renderer = (const char*)glGetString(GL_RENDERER);
    const char* vendor   = (const char*)glGetString(GL_VENDOR);
    const char* version  = (const char*)glGetString(GL_VERSION);
    const char* glexts   = (const char*)glGetString(GL_EXTENSIONS);

    strncpy(g_caps.GPU_RENDERER, renderer ? renderer : "unknown", 255);
    strncpy(g_caps.GPU_VENDOR,   vendor   ? vendor   : "unknown", 127);
    strncpy(g_caps.GLES_VERSION, version  ? version  : "unknown", 127);

    if (glexts) strncpy(g_raw_gl_ext, glexts, sizeof(g_raw_gl_ext) - 1);

    EGLDisplay dpy = eglGetCurrentDisplay();
    if (dpy != EGL_NO_DISPLAY) {
        const char* eglexts = eglQueryString(dpy, EGL_EXTENSIONS);
        if (eglexts) strncpy(g_raw_egl_ext, eglexts, sizeof(g_raw_egl_ext) - 1);
    }

    LOGI("GPU: %s | Vendor: %s | GLES: %s",
         g_caps.GPU_RENDERER, g_caps.GPU_VENDOR, g_caps.GLES_VERSION);

    // ═══ LEITURA DAS EXTENSÕES REPORTADAS ════════════════════════════════
    g_caps.FB_FETCH               = check_extension("GL_ARM_shader_framebuffer_fetch");
    g_caps.FB_FETCH_DEPTH_STENCIL = check_extension("GL_ARM_shader_framebuffer_fetch_depth_stencil");
    g_caps.PLS                    = check_extension("GL_EXT_shader_pixel_local_storage");
    g_caps.MALI_BINARY            = check_extension("GL_ARM_mali_shader_binary");
    g_caps.ASTC_LDR               = check_extension("GL_KHR_texture_compression_astc_ldr");
    g_caps.ASTC_HDR               = check_extension("GL_KHR_texture_compression_astc_hdr");
    g_caps.ETC2                   = check_extension("GL_OES_compressed_ETC2_RGB8_texture");
    g_caps.STANDARD_DERIVATIVES      = check_extension("GL_OES_standard_derivatives");
    g_caps.TEXTURE_FLOAT             = check_extension("GL_OES_texture_float");
    g_caps.TEXTURE_FLOAT_LINEAR      = check_extension("GL_OES_texture_float_linear");
    g_caps.TEXTURE_HALF_FLOAT        = check_extension("GL_OES_texture_half_float");
    g_caps.TEXTURE_HALF_FLOAT_LINEAR = check_extension("GL_OES_texture_half_float_linear");
    g_caps.MULTISAMPLED_RT    = check_extension("GL_EXT_multisampled_render_to_texture");
    g_caps.MULTISAMPLED_RT2   = check_extension("GL_EXT_multisampled_render_to_texture2");
    g_caps.SRGB               = check_extension("GL_EXT_sRGB");
    g_caps.SRGB_WRITE_CONTROL = check_extension("GL_EXT_sRGB_write_control");
    g_caps.ANISOTROPIC        = check_extension("GL_EXT_texture_filter_anisotropic");
    g_caps.PROGRAM_BINARY     = check_extension("GL_OES_get_program_binary");
    g_caps.TEXTURE_STORAGE    = check_extension("GL_EXT_texture_storage");
    g_caps.TEXTURE_NPOT       = check_extension("GL_OES_texture_npot");
    g_caps.MAP_BUFFER         = check_extension("GL_OES_mapbuffer");
    g_caps.BUFFER_STORAGE     = check_extension("GL_EXT_buffer_storage");
    g_caps.TIMER_QUERY        = check_extension("GL_EXT_disjoint_timer_query");
    g_caps.DEBUG_MARKER       = check_extension("GL_EXT_debug_marker");
    g_caps.KHR_DEBUG          = check_extension("GL_KHR_debug");
    g_caps.ROBUSTNESS         = check_extension("GL_EXT_robustness");

    // ═══ FORÇA CONHECIMENTO DE HARDWARE (BIFROST) SE NECESSÁRIO ════════════
    apply_bifrost_hardware_knowledge();

    // ═══ LIMITES ═════════════════════════════════════════════════════════
    glGetIntegerv(GL_MAX_TEXTURE_SIZE,                &g_caps.MAX_TEXTURE_SIZE);
    glGetIntegerv(GL_MAX_SAMPLES,                      &g_caps.MAX_SAMPLES);
    glGetIntegerv(GL_MAX_VERTEX_ATTRIBS,               &g_caps.MAX_VERTEX_ATTRIBS);
    glGetIntegerv(GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS, &g_caps.MAX_COMBINED_TIU);

    g_caps.MAX_ANISOTROPY = 1;
    if (g_caps.ANISOTROPIC) {
        glGetIntegerv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &g_caps.MAX_ANISOTROPY);
    }

    LOGI("FB_FETCH=%d PLS=%d ASTC=%d MSAA_RT=%d ANISO=%d TIMER=%d",
         g_caps.FB_FETCH, g_caps.PLS, g_caps.ASTC_LDR,
         g_caps.MULTISAMPLED_RT, g_caps.ANISOTROPIC, g_caps.TIMER_QUERY);
    LOGI("MAX_TEX=%d MAX_SAMPLES=%d MAX_ANISO=%d MAX_VA=%d",
         g_caps.MAX_TEXTURE_SIZE, g_caps.MAX_SAMPLES,
         g_caps.MAX_ANISOTROPY, g_caps.MAX_VERTEX_ATTRIBS);

    g_initialized = true;
    LOGI("========== MaliOpt Plugin: Deteccao concluida ==========");
}

JNIEXPORT jstring JNICALL
Java_com_maliopt_MaliOptNative_getExtensionReport(JNIEnv *env, jobject thiz) {
    if (!g_initialized)
        return (*env)->NewStringUTF(env, "NOT_INITIALIZED");

    char report[3072];
    snprintf(report, sizeof(report),
        "CONTEXT=%s;GPU=%s;VENDOR=%s;GLES=%s;"
        "FB_FETCH=%d;FB_FETCH_DS=%d;PLS=%d;MALI_BIN=%d;"
        "ASTC_LDR=%d;ASTC_HDR=%d;ETC2=%d;"
        "STD_DERIV=%d;TEX_FLOAT=%d;TEX_FLOAT_LIN=%d;TEX_HALF=%d;TEX_HALF_LIN=%d;"
        "MSAA_RT=%d;MSAA_RT2=%d;SRGB=%d;SRGB_WRITE=%d;ANISO=%d;"
        "PROG_BIN=%d;TEX_STORAGE=%d;TEX_NPOT=%d;MAP_BUF=%d;BUF_STORAGE=%d;"
        "TIMER=%d;DBG_MARKER=%d;KHR_DEBUG=%d;ROBUSTNESS=%d;"
        "MAX_TEX=%d;MAX_SAMPLES=%d;MAX_ANISO=%d;MAX_VA=%d;MAX_CTIU=%d",
        g_context, g_caps.GPU_RENDERER, g_caps.GPU_VENDOR, g_caps.GLES_VERSION,
        g_caps.FB_FETCH, g_caps.FB_FETCH_DEPTH_STENCIL, g_caps.PLS, g_caps.MALI_BINARY,
        g_caps.ASTC_LDR, g_caps.ASTC_HDR, g_caps.ETC2,
        g_caps.STANDARD_DERIVATIVES, g_caps.TEXTURE_FLOAT, g_caps.TEXTURE_FLOAT_LINEAR,
        g_caps.TEXTURE_HALF_FLOAT, g_caps.TEXTURE_HALF_FLOAT_LINEAR,
        g_caps.MULTISAMPLED_RT, g_caps.MULTISAMPLED_RT2,
        g_caps.SRGB, g_caps.SRGB_WRITE_CONTROL, g_caps.ANISOTROPIC,
        g_caps.PROGRAM_BINARY, g_caps.TEXTURE_STORAGE, g_caps.TEXTURE_NPOT,
        g_caps.MAP_BUFFER, g_caps.BUFFER_STORAGE,
        g_caps.TIMER_QUERY, g_caps.DEBUG_MARKER, g_caps.KHR_DEBUG, g_caps.ROBUSTNESS,
        g_caps.MAX_TEXTURE_SIZE, g_caps.MAX_SAMPLES, g_caps.MAX_ANISOTROPY,
        g_caps.MAX_VERTEX_ATTRIBS, g_caps.MAX_COMBINED_TIU);
    return (*env)->NewStringUTF(env, report);
}

JNIEXPORT jstring JNICALL
Java_com_maliopt_MaliOptNative_getRawGLExtensions(JNIEnv *env, jobject thiz) {
    return (*env)->NewStringUTF(env, g_raw_gl_ext);
}

JNIEXPORT jstring JNICALL
Java_com_maliopt_MaliOptNative_getRawEGLExtensions(JNIEnv *env, jobject thiz) {
    return (*env)->NewStringUTF(env, g_raw_egl_ext);
}

JNIEXPORT jboolean JNICALL
Java_com_maliopt_MaliOptNative_isFramebufferFetchSupported(JNIEnv *env, jobject thiz) {
    return (jboolean)g_caps.FB_FETCH; }
JNIEXPORT jboolean JNICALL
Java_com_maliopt_MaliOptNative_isFramebufferFetchDepthStencilSupported(JNIEnv *env, jobject thiz) {
    return (jboolean)g_caps.FB_FETCH_DEPTH_STENCIL; }
JNIEXPORT jboolean JNICALL
Java_com_maliopt_MaliOptNative_isPLSSupported(JNIEnv *env, jobject thiz) {
    return (jboolean)g_caps.PLS; }
JNIEXPORT jboolean JNICALL
Java_com_maliopt_MaliOptNative_isMaliBinarySupported(JNIEnv *env, jobject thiz) {
    return (jboolean)g_caps.MALI_BINARY; }
JNIEXPORT jboolean JNICALL
Java_com_maliopt_MaliOptNative_isASTCLDRSupported(JNIEnv *env, jobject thiz) {
    return (jboolean)g_caps.ASTC_LDR; }
JNIEXPORT jboolean JNICALL
Java_com_maliopt_MaliOptNative_isASTCHDRSupported(JNIEnv *env, jobject thiz) {
    return (jboolean)g_caps.ASTC_HDR; }
JNIEXPORT jboolean JNICALL
Java_com_maliopt_MaliOptNative_isETC2Supported(JNIEnv *env, jobject thiz) {
    return (jboolean)g_caps.ETC2; }
JNIEXPORT jboolean JNICALL
Java_com_maliopt_MaliOptNative_isStandardDerivativesSupported(JNIEnv *env, jobject thiz) {
    return (jboolean)g_caps.STANDARD_DERIVATIVES; }
JNIEXPORT jboolean JNICALL
Java_com_maliopt_MaliOptNative_isTextureFloatSupported(JNIEnv *env, jobject thiz) {
    return (jboolean)g_caps.TEXTURE_FLOAT; }
JNIEXPORT jboolean JNICALL
Java_com_maliopt_MaliOptNative_isTextureFloatLinearSupported(JNIEnv *env, jobject thiz) {
    return (jboolean)g_caps.TEXTURE_FLOAT_LINEAR; }
JNIEXPORT jboolean JNICALL
Java_com_maliopt_MaliOptNative_isTextureHalfFloatSupported(JNIEnv *env, jobject thiz) {
    return (jboolean)g_caps.TEXTURE_HALF_FLOAT; }
JNIEXPORT jboolean JNICALL
Java_com_maliopt_MaliOptNative_isTextureHalfFloatLinearSupported(JNIEnv *env, jobject thiz) {
    return (jboolean)g_caps.TEXTURE_HALF_FLOAT_LINEAR; }
JNIEXPORT jboolean JNICALL
Java_com_maliopt_MaliOptNative_isMultisampledRTSupported(JNIEnv *env, jobject thiz) {
    return (jboolean)g_caps.MULTISAMPLED_RT; }
JNIEXPORT jboolean JNICALL
Java_com_maliopt_MaliOptNative_isMultisampledRT2Supported(JNIEnv *env, jobject thiz) {
    return (jboolean)g_caps.MULTISAMPLED_RT2; }
JNIEXPORT jboolean JNICALL
Java_com_maliopt_MaliOptNative_isSRGBSupported(JNIEnv *env, jobject thiz) {
    return (jboolean)g_caps.SRGB; }
JNIEXPORT jboolean JNICALL
Java_com_maliopt_MaliOptNative_isSRGBWriteControlSupported(JNIEnv *env, jobject thiz) {
    return (jboolean)g_caps.SRGB_WRITE_CONTROL; }
JNIEXPORT jboolean JNICALL
Java_com_maliopt_MaliOptNative_isAnisotropicFilteringSupported(JNIEnv *env, jobject thiz) {
    return (jboolean)g_caps.ANISOTROPIC; }
JNIEXPORT jboolean JNICALL
Java_com_maliopt_MaliOptNative_isProgramBinarySupported(JNIEnv *env, jobject thiz) {
    return (jboolean)g_caps.PROGRAM_BINARY; }
JNIEXPORT jboolean JNICALL
Java_com_maliopt_MaliOptNative_isTextureStorageSupported(JNIEnv *env, jobject thiz) {
    return (jboolean)g_caps.TEXTURE_STORAGE; }
JNIEXPORT jboolean JNICALL
Java_com_maliopt_MaliOptNative_isTextureNPOTSupported(JNIEnv *env, jobject thiz) {
    return (jboolean)g_caps.TEXTURE_NPOT; }
JNIEXPORT jboolean JNICALL
Java_com_maliopt_MaliOptNative_isMapBufferSupported(JNIEnv *env, jobject thiz) {
    return (jboolean)g_caps.MAP_BUFFER; }
JNIEXPORT jboolean JNICALL
Java_com_maliopt_MaliOptNative_isBufferStorageSupported(JNIEnv *env, jobject thiz) {
    return (jboolean)g_caps.BUFFER_STORAGE; }
JNIEXPORT jboolean JNICALL
Java_com_maliopt_MaliOptNative_isTimerQuerySupported(JNIEnv *env, jobject thiz) {
    return (jboolean)g_caps.TIMER_QUERY; }
JNIEXPORT jboolean JNICALL
Java_com_maliopt_MaliOptNative_isDebugMarkerSupported(JNIEnv *env, jobject thiz) {
    return (jboolean)g_caps.DEBUG_MARKER; }
JNIEXPORT jboolean JNICALL
Java_com_maliopt_MaliOptNative_isKHRDebugSupported(JNIEnv *env, jobject thiz) {
    return (jboolean)g_caps.KHR_DEBUG; }
JNIEXPORT jboolean JNICALL
Java_com_maliopt_MaliOptNative_isRobustnessSupported(JNIEnv *env, jobject thiz) {
    return (jboolean)g_caps.ROBUSTNESS; }

JNIEXPORT jint JNICALL
Java_com_maliopt_MaliOptNative_getMaxTextureSize(JNIEnv *env, jobject thiz) {
    return (jint)g_caps.MAX_TEXTURE_SIZE; }
JNIEXPORT jint JNICALL
Java_com_maliopt_MaliOptNative_getMaxSamples(JNIEnv *env, jobject thiz) {
    return (jint)g_caps.MAX_SAMPLES; }
JNIEXPORT jint JNICALL
Java_com_maliopt_MaliOptNative_getMaxAnisotropy(JNIEnv *env, jobject thiz) {
    return (jint)g_caps.MAX_ANISOTROPY; }
JNIEXPORT jint JNICALL
Java_com_maliopt_MaliOptNative_getMaxVertexAttribs(JNIEnv *env, jobject thiz) {
    return (jint)g_caps.MAX_VERTEX_ATTRIBS; }
JNIEXPORT jint JNICALL
Java_com_maliopt_MaliOptNative_getMaxCombinedTextureImageUnits(JNIEnv *env, jobject thiz) {
    return (jint)g_caps.MAX_COMBINED_TIU; }

JNIEXPORT jstring JNICALL
Java_com_maliopt_MaliOptNative_getActiveRenderContext(JNIEnv *env, jobject thiz) {
    return (*env)->NewStringUTF(env, g_context); }
JNIEXPORT jstring JNICALL
Java_com_maliopt_MaliOptNative_getGPURenderer(JNIEnv *env, jobject thiz) {
    return (*env)->NewStringUTF(env, g_caps.GPU_RENDERER); }
JNIEXPORT jstring JNICALL
Java_com_maliopt_MaliOptNative_getGPUVendor(JNIEnv *env, jobject thiz) {
    return (*env)->NewStringUTF(env, g_caps.GPU_VENDOR); }
JNIEXPORT jstring JNICALL
Java_com_maliopt_MaliOptNative_getGLESVersion(JNIEnv *env, jobject thiz) {
    return (*env)->NewStringUTF(env, g_caps.GLES_VERSION); }
