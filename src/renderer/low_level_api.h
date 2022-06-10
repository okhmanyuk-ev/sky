#pragma once

#include <renderer/defines.h>

#if defined(RENDERER_GL44)
    #define GLEW_STATIC
    #include <GL/glew.h>
    #include <GL/GL.h>
    #include <GL/wglew.h>
    #pragma comment(lib, "opengl32")
    #pragma comment(lib, "glu32")
#elif defined(RENDERER_GLES3)
    #if defined(PLATFORM_ANDROID)
        #include <EGL/egl.h>
        #include <EGL/eglext.h>
        #include <EGL/eglplatform.h>
        #include <GLES3/gl32.h>
        #include <GLES3/gl3ext.h>
    #elif defined(PLATFORM_IOS)
        #include <OpenGLES/ES3/gl.h>
        #include <OpenGLES/ES3/glext.h>
    #endif
#elif defined(RENDERER_VK)
 //   #include <vulkan/vulkan_raii.hpp> // TODO: very long compile time if uncommented
#endif