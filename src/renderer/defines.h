#pragma once

#include <platform/defines.h>

#if defined(PLATFORM_WINDOWS)
//	#define RENDERER_D3D11
	#define RENDERER_GL44
//	#define RENDERER_VK 
#elif defined(PLATFORM_ANDROID)
	#define RENDERER_GLES3
#elif defined(PLATFORM_IOS)
    #define RENDERER_GLES3
//  #define RENDERER_MTL
#endif

#if defined(RENDERER_D3D11)
	#define RENDERER_NAME "D3D11"
	#define RENDERER_NAME_LONG "Direct3D 11"
#elif defined(RENDERER_GL44)
	#define RENDERER_NAME "OpenGL4"
	#define RENDERER_NAME_LONG "OpenGL 4.4"
#elif defined(RENDERER_GLES3)
	#define RENDERER_NAME "GLES3"
	#define RENDERER_NAME_LONG "OpenGL ES 3.2"
#elif defined(RENDERER_VK)
	#define RENDERER_NAME "Vulkan"
	#define RENDERER_NAME_LONG "Vulkan"
#elif defined(RENDERER_MTL)
    #define RENDERER_NAME "Metal"
    #define RENDERER_NAME_LONG "Metal 1.0"
#endif

/*
 windows:
  - d3d11
  - gl44
  - vulkan

 android:
  - gles3
 
 ios:
  - gles3
  - metal
 
 */