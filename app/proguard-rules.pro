# MaliOpt Plugin - ProGuard Rules
# Mantem todas as classes nativas JNI
-keep class com.hybridcore.maliopt.** { *; }
-keepclasseswithmembernames class * {
    native <methods>;
}
