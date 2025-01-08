#pragma once

#include <platform/defines.h>

#if defined(PLATFORM_WINDOWS)
	//#include <Windows.h>
	//#include <windowsx.h>
#elif defined(PLATFORM_ANDROID)
	#include <android/asset_manager.h>
	#include <android_native_app_glue.h>
	#include <android/window.h>
	#include <android/configuration.h>
	#include <android/keycodes.h>
	#include <jni.h>
#elif defined(PLATFORM_IOS)
	#import <Foundation/Foundation.h>
	#import <UIKit/UIKit.h>
	#import <StoreKit/StoreKit.h>
#elif defined(PLATFORM_EMSCRIPTEN)
	#include <emscripten.h>
	#include <emscripten/html5.h>
#endif
