#pragma once

#ifdef _WIN32
	#define PLATFORM_WINDOWS
#elif __ANDROID__
	#define PLATFORM_ANDROID
#elif EMSCRIPTEN
	#define PLATFORM_EMSCRIPTEN
#elif LINUX
	#define PLATFORM_LINUX
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
#elif defined(PLATFORM_MAC)
	#define PLATFORM_NAME "Mac"
#elif defined(PLATFORM_EMSCRIPTEN)
	#define PLATFORM_NAME "Emscripten"
#elif defined(PLATFORM_LINUX)
	#define PLATFORM_NAME "Linux"
#endif
