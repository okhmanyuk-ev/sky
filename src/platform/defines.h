#pragma once

#ifdef _WIN32
	#define PLATFORM_WINDOWS
#elif __ANDROID__
	#define PLATFORM_ANDROID
#elif __APPLE__
    #define PLATFORM_IOS
#endif

#if defined(PLATFORM_ANDROID) || defined(PLATFORM_IOS)
	#define PLATFORM_MOBILE
#endif

#if defined(PLATFORM_WINDOWS)
	#define PLATFORM_NAME "Windows"
#elif defined(PLATFORM_ANDROID)
	#define PLATFORM_NAME "Android"
#elif defined(PLATFORM_IOS)
    #define PLATFORM_NAME "iOS"
#endif