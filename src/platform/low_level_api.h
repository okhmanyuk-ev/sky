#pragma once

#include <Platform/defines.h>

#if defined(PLATFORM_WINDOWS)
    #include <Windows.h>
    #include <windowsx.h>
#elif defined(PLATFORM_ANDROID)
    #include <android/asset_manager.h>
    #include <android_native_app_glue.h>
    #include <android/window.h>
    #include <android/configuration.h>
    #include <android/keycodes.h>
    #include <jni.h>
#elif defined(PLATFORM_IOS)

#endif