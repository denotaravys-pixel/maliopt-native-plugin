package com.hybridcore.maliopt;

/**
 * MaliOpt Native Plugin - Ponte JNI
 * Expoe todas as capacidades reais da GPU Mali-G52 para o MaliOpt Mod.
 */
public class MaliOptNative {

    static {
        System.loadLibrary("maliopt");
    }

    // =============================================
    // INICIALIZACAO
    // =============================================

    /** Detecta todas as extensoes e limites. Chamar antes de qualquer getter. */
    public static native void detectExtensions();

    // =============================================
    // RELATORIO COMPLETO
    // =============================================

    /** Retorna string completa: "CHAVE=VALOR;CHAVE=VALOR;..." */
    public static native String getExtensionReport();

    /** Retorna a lista bruta de extensoes OpenGL ES (glGetString). */
    public static native String getRawGLExtensions();

    /** Retorna a lista bruta de extensoes EGL (eglQueryString). */
    public static native String getRawEGLExtensions();

    // =============================================
    // FAST PATHS ARM MALI
    // =============================================

    public static native boolean isFramebufferFetchSupported();
    public static native boolean isFramebufferFetchDepthStencilSupported();
    public static native boolean isPLSSupported();
    public static native boolean isMaliBinarySupported();

    // =============================================
    // COMPRESSAO DE TEXTURA
    // =============================================

    public static native boolean isASTCLDRSupported();
    public static native boolean isASTCHDRSupported();
    public static native boolean isETC2Supported();

    // =============================================
    // PRECISAO E DERIVADAS
    // =============================================

    public static native boolean isStandardDerivativesSupported();
    public static native boolean isTextureFloatSupported();
    public static native boolean isTextureFloatLinearSupported();
    public static native boolean isTextureHalfFloatSupported();
    public static native boolean isTextureHalfFloatLinearSupported();

    // =============================================
    // RENDERIZACAO AVANCADA
    // =============================================

    public static native boolean isMultisampledRTSupported();
    public static native boolean isMultisampledRT2Supported();
    public static native boolean isSRGBSupported();
    public static native boolean isSRGBWriteControlSupported();
    public static native boolean isAnisotropicFilteringSupported();

    // =============================================
    // MEMORIA E CACHE
    // =============================================

    public static native boolean isProgramBinarySupported();
    public static native boolean isTextureStorageSupported();
    public static native boolean isTextureNPOTSupported();
    public static native boolean isMapBufferSupported();
    public static native boolean isBufferStorageSupported();

    // =============================================
    // DEBUG E SINCRONIZACAO
    // =============================================

    public static native boolean isTimerQuerySupported();
    public static native boolean isDebugMarkerSupported();
    public static native boolean isKHRDebugSupported();
    public static native boolean isRobustnessSupported();

    // =============================================
    // LIMITES DE HARDWARE
    // =============================================

    public static native int getMaxTextureSize();
    public static native int getMaxSamples();
    public static native int getMaxAnisotropy();
    public static native int getMaxVertexAttribs();
    public static native int getMaxCombinedTextureImageUnits();

    // =============================================
    // CONTEXTO E INFORMACOES DA GPU
    // =============================================

    /** Retorna: "NATIVE", "GL4ES", "ANGLE" ou "UNKNOWN" */
    public static native String getActiveRenderContext();

    public static native String getGPURenderer();
    public static native String getGPUVendor();
    public static native String getGLESVersion();
}
