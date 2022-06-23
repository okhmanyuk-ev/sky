#pragma once

#include <platform/defines.h>

#if defined(PLATFORM_WINDOWS)
//	#define RENDERER_GL44
	#define RENDERER_SKYGFX
#elif defined(PLATFORM_ANDROID)
	#define RENDERER_GLES3
#elif defined(PLATFORM_IOS)
    #define RENDERER_GLES3
//  #define RENDERER_MTL
#endif

#if defined(RENDERER_GL44)
	#define RENDERER_NAME "OpenGL4"
	#define RENDERER_NAME_LONG "OpenGL 4.4"
#elif defined(RENDERER_GLES3)
	#define RENDERER_NAME "GLES3"
	#define RENDERER_NAME_LONG "OpenGL ES 3.2"
#elif defined(RENDERER_SKYGFX)
	#define RENDERER_NAME "skygfx" 
	#define RENDERER_NAME_LONG "skygfx"
#endif
