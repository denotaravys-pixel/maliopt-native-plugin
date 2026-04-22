package com.maliopt;

/**
 * MaliOpt Native Plugin — Ponte JNI
 *
 * REGRA CRÍTICA: o bloco static está INTENCIONALMENTE VAZIO.
 * O controlo de carga da biblioteca pertence exclusivamente a
 * MaliOptMod.loadNativePlugin() via System.loadLibrary("maliopt").
 * Nunca adicionar System.loadLibrary() aqui.
 *
 * Pacote: com.maliopt  (os símbolos C usam o prefixo Java_com_maliopt_)
 * Ao renomear o pacote, os nomes JNI em mali_opt_extensions.c e
 * mali_opt_egl.c DEVEM ser actualizados em sincronia.
 */
public class MaliOptNative {

    static {
        // VAZIO — carga da biblioteca é responsabilidade de MaliOptMod
    }

    // =========================================================
    // INICIALIZAÇÃO — chamar ANTES de qualquer getter
    // =========================================================

    /** Detecta todas as extensões e limites via C directo ao driver. */
    public static native void detectExtensions();

    // =========================================================
    // RELATÓRIO COMPLETO
    // =========================================================

    /** Retorna string "CHAVE=VALOR;CHAVE=VALOR;..." com todas as capacidades. */
    public static native String getExtensionReport();

    /** Lista bruta de extensões GL (glGetString(GL_EXTENSIONS)). */
    public static native String getRawGLExtensions();

    /** Lista bruta de extensões EGL (eglQueryString(EGL_EXTENSIONS)). */
    public static native String getRawEGLExtensions();

    // =========================================================
    // FAST PATHS ARM MALI — TBDR core
    // =========================================================

    public static native boolean isFramebufferFetchSupported();
    public static native boolean isFramebufferFetchDepthStencilSupported();
    public static native boolean isPLSSupported();
    public static native boolean isMaliBinarySupported();

    // =========================================================
    // COMPRESSÃO DE TEXTURA
    // =========================================================

    public static native boolean isASTCLDRSupported();
    public static native boolean isASTCHDRSupported();
    public static native boolean isETC2Supported();

    // =========================================================
    // PRECISÃO E DERIVADAS
    // =========================================================

    public static native boolean isStandardDerivativesSupported();
    public static native boolean isTextureFloatSupported();
    public static native boolean isTextureFloatLinearSupported();
    public static native boolean isTextureHalfFloatSupported();
    public static native boolean isTextureHalfFloatLinearSupported();

    // =========================================================
    // RENDERIZAÇÃO AVANÇADA
    // =========================================================

    public static native boolean isMultisampledRTSupported();
    public static native boolean isMultisampledRT2Supported();
    public static native boolean isSRGBSupported();
    public static native boolean isSRGBWriteControlSupported();
    public static native boolean isAnisotropicFilteringSupported();

    // =========================================================
    // MEMÓRIA E CACHE
    // =========================================================

    public static native boolean isProgramBinarySupported();
    public static native boolean isTextureStorageSupported();
    public static native boolean isTextureNPOTSupported();
    public static native boolean isMapBufferSupported();
    public static native boolean isBufferStorageSupported();

    // =========================================================
    // DEBUG E SINCRONIZAÇÃO
    // =========================================================

    public static native boolean isTimerQuerySupported();
    public static native boolean isDebugMarkerSupported();
    public static native boolean isKHRDebugSupported();
    public static native boolean isRobustnessSupported();

    // =========================================================
    // SINCRONIZAÇÃO EGL (implementados em mali_opt_egl.c)
    // =========================================================

    /** EGL_KHR_fence_sync — necessário para sincronização GPU/CPU eficiente. */
    public static native boolean isFenceSyncSupported();

    /** EGL_ANDROID_native_fence_sync — Android-specific fence sync. */
    public static native boolean isNativeFenceSyncSupported();

    /** EGL_KHR_wait_sync — GPU-side wait sem bloqueio da CPU. */
    public static native boolean isWaitSyncSupported();

    // =========================================================
    // LIMITES DE HARDWARE
    // =========================================================

    public static native int getMaxTextureSize();
    public static native int getMaxSamples();
    public static native int getMaxAnisotropy();
    public static native int getMaxVertexAttribs();
    public static native int getMaxCombinedTextureImageUnits();

    // =========================================================
    // CONTEXTO E INFORMAÇÕES DA GPU
    // =========================================================

    /** Retorna: "NATIVE", "ANGLE", "GL4ES" ou "UNKNOWN". */
    public static native String getActiveRenderContext();

    public static native String getGPURenderer();
    public static native String getGPUVendor();
    public static native String getGLESVersion();
}
