#pragma once

#include <Renderer/defines.h>

#if defined(RENDERER_D3D11)
    #include <d3dcompiler.h>
    #include <d3d11.h>
    #pragma comment(lib, "d3d11")
    #pragma comment(lib, "d3dcompiler")
#elif defined(RENDERER_GL44)
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
    #elif defined(PLATFORM_IOS)
        #include <OpenGLES/ES3/gl.h>
        #include <OpenGLES/ES3/glext.h>
    #endif
#endif