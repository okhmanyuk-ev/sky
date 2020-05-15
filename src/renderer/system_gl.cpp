#include "system_gl.h"

#if defined(RENDERER_GL44) || defined(RENDERER_GLES3)
#include <platform/system_windows.h>
#include <platform/system_android.h>
#include <platform/system_ios.h>

using namespace Renderer;

bool SystemGL::IsRenderTargetBound() 
{
	return mRenderTargetBound;
}

bool SystemGL::mRenderTargetBound = false;

const std::unordered_map<Topology, GLenum> SystemGL::Topology = {
	{ Topology::PointList, GL_POINTS },
	{ Topology::LineList, GL_LINES },
	{ Topology::LineStrip, GL_LINE_STRIP },
	{ Topology::TriangleList, GL_TRIANGLES },
	{ Topology::TriangleStrip, GL_TRIANGLE_STRIP }
};

const std::unordered_map<Vertex::Attribute::Format, GLint> SystemGL::Size = {
	{ Vertex::Attribute::Format::R32F, 1 },
	{ Vertex::Attribute::Format::R32G32F, 2 },
	{ Vertex::Attribute::Format::R32G32B32F, 3 },
	{ Vertex::Attribute::Format::R32G32B32A32F, 4 },
	{ Vertex::Attribute::Format::R8UN, 1 },
	{ Vertex::Attribute::Format::R8G8UN, 2 },
	{ Vertex::Attribute::Format::R8G8B8UN, 3 },
	{ Vertex::Attribute::Format::R8G8B8A8UN, 4 }
};

const std::unordered_map<Vertex::Attribute::Format, GLenum> SystemGL::Type = {
	{ Vertex::Attribute::Format::R32F, GL_FLOAT },
	{ Vertex::Attribute::Format::R32G32F, GL_FLOAT },
	{ Vertex::Attribute::Format::R32G32B32F, GL_FLOAT },
	{ Vertex::Attribute::Format::R32G32B32A32F, GL_FLOAT },
	{ Vertex::Attribute::Format::R8UN, GL_UNSIGNED_BYTE },
	{ Vertex::Attribute::Format::R8G8UN, GL_UNSIGNED_BYTE },
	{ Vertex::Attribute::Format::R8G8B8UN, GL_UNSIGNED_BYTE },
	{ Vertex::Attribute::Format::R8G8B8A8UN, GL_UNSIGNED_BYTE }
};

const std::unordered_map<Vertex::Attribute::Format, GLboolean> SystemGL::Normalize = {
	{ Vertex::Attribute::Format::R32F, GL_FALSE },
	{ Vertex::Attribute::Format::R32G32F, GL_FALSE },
	{ Vertex::Attribute::Format::R32G32B32F, GL_FALSE },
	{ Vertex::Attribute::Format::R32G32B32A32F, GL_FALSE },
	{ Vertex::Attribute::Format::R8UN, GL_TRUE },
	{ Vertex::Attribute::Format::R8G8UN, GL_TRUE },
	{ Vertex::Attribute::Format::R8G8B8UN, GL_TRUE },
	{ Vertex::Attribute::Format::R8G8B8A8UN, GL_TRUE }
};

namespace
{
	const std::unordered_map<ComparisonFunc, GLenum> ComparisonFuncMap = {
		{ ComparisonFunc::Always, GL_ALWAYS },
		{ ComparisonFunc::Never, GL_NEVER },
		{ ComparisonFunc::Less, GL_LESS },
		{ ComparisonFunc::Equal, GL_EQUAL },
		{ ComparisonFunc::NotEqual, GL_NOTEQUAL },
		{ ComparisonFunc::LessEqual, GL_LEQUAL },
		{ ComparisonFunc::Greater, GL_GREATER },
		{ ComparisonFunc::GreaterEqual, GL_GEQUAL }
	};

	const std::unordered_map<CullMode, GLenum> CullMap = {
		{ CullMode::None, GL_NONE },
		{ CullMode::Front, GL_FRONT },
		{ CullMode::Back, GL_BACK }
	};
}

#if defined(RENDERER_GL44)
void GLAPIENTRY MessageCallback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length,
	const GLchar* message, const void* userParam)
{
	fprintf(stderr, "GL CALLBACK: %s type = 0x%x, severity = 0x%x, message = %s\n",
		(type == GL_DEBUG_TYPE_ERROR ? "** GL ERROR **" : ""), type, severity, message);
}
#endif

#if defined(PLATFORM_IOS)
#import <GLKit/GLKit.h>

GLKView* mGLKView = nullptr;
#endif

SystemGL::SystemGL()
{
#if defined(RENDERER_GL44)
	// https://mariuszbartosik.com/opengl-4-x-initialization-in-windows-without-a-framework/

	mHDC = GetDC(Platform::SystemWindows::Window);

	PIXELFORMATDESCRIPTOR pfd;
	memset(&pfd, 0, sizeof(PIXELFORMATDESCRIPTOR));
	pfd.nSize = sizeof(PIXELFORMATDESCRIPTOR);
	pfd.nVersion = 1;
	pfd.iLayerType = PFD_MAIN_PLANE;
	pfd.dwFlags = PFD_DOUBLEBUFFER | PFD_SUPPORT_OPENGL | PFD_DRAW_TO_WINDOW;
	pfd.iPixelType = PFD_TYPE_RGBA;
	pfd.cColorBits = 24;
	pfd.cDepthBits = 24;
	pfd.cStencilBits = 8;
	pfd.cAlphaBits = 8;

	int nPixelFormat = ChoosePixelFormat(mHDC, &pfd);
	SetPixelFormat(mHDC, nPixelFormat, &pfd);

	mWglContext = wglCreateContext(mHDC);
	wglMakeCurrent(mHDC, mWglContext);

	glewInit();

	auto win_system = std::dynamic_pointer_cast<Platform::SystemWindows>(PLATFORM);
	win_system->destroyWindow();
	win_system->makeWindow();

	mHDC = GetDC(Platform::SystemWindows::Window);

	const int pixelAttribs[] = {
		WGL_DRAW_TO_WINDOW_ARB, GL_TRUE,
		WGL_SUPPORT_OPENGL_ARB, GL_TRUE,
		WGL_DOUBLE_BUFFER_ARB, GL_TRUE,
		WGL_PIXEL_TYPE_ARB, WGL_TYPE_RGBA_ARB,
		WGL_ACCELERATION_ARB, WGL_FULL_ACCELERATION_ARB,
		WGL_COLOR_BITS_ARB, 32,
		WGL_ALPHA_BITS_ARB, 8,
		WGL_DEPTH_BITS_ARB, 24,
		WGL_STENCIL_BITS_ARB, 8,
		WGL_SAMPLE_BUFFERS_ARB, GL_TRUE,
		WGL_SAMPLES_ARB, 1,
		0
	};

	int pixelFormatID; 
	UINT numFormats;
	wglChoosePixelFormatARB(mHDC, pixelAttribs, NULL, 1, &pixelFormatID, &numFormats);

	memset(&pfd, 0, sizeof(PIXELFORMATDESCRIPTOR));

	DescribePixelFormat(mHDC, pixelFormatID, sizeof(pfd), &pfd);
	SetPixelFormat(mHDC, pixelFormatID, &pfd);

	int attribs[] =
	{
		WGL_CONTEXT_MAJOR_VERSION_ARB, 4,
		WGL_CONTEXT_MINOR_VERSION_ARB, 4,
		WGL_CONTEXT_PROFILE_MASK_ARB, WGL_CONTEXT_CORE_PROFILE_BIT_ARB,
		WGL_CONTEXT_FLAGS_ARB, /*WGL_CONTEXT_DEBUG_BIT_ARB*/0,
		0
	};

	wglMakeCurrent(NULL, NULL);
	wglDeleteContext(mWglContext);
	mWglContext = wglCreateContextAttribsARB(mHDC, 0, attribs);
	wglMakeCurrent(mHDC, mWglContext);

	auto version = glGetString(GL_VERSION);

	//glEnable(GL_DEBUG_OUTPUT);
	glDebugMessageCallback(MessageCallback, 0);
#elif defined(RENDERER_GLES3)
    #if defined(PLATFORM_ANDROID)
	const EGLint attribs[] = {
		EGL_RENDERABLE_TYPE, EGL_OPENGL_ES3_BIT_KHR,
		EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
		EGL_BLUE_SIZE, 8,
		EGL_GREEN_SIZE, 8,
		EGL_RED_SIZE, 8,
		EGL_DEPTH_SIZE, 24,
		EGL_NONE
	};
	const EGLint context_attribs[] = {
		EGL_CONTEXT_CLIENT_VERSION, 3,
		EGL_NONE
	};
	mDisplay = eglGetDisplay(EGL_DEFAULT_DISPLAY);
	eglInitialize(mDisplay, 0, 0);
	EGLint num_configs;
	eglChooseConfig(mDisplay, attribs, &mConfig, 1, &num_configs);
	mSurface = eglCreateWindowSurface(mDisplay, mConfig, Platform::SystemAndroid::Instance->window, NULL);
	mContext = eglCreateContext(mDisplay, mConfig, NULL, context_attribs);
	eglMakeCurrent(mDisplay, mSurface, mSurface, mContext);
	
	mResumeListener.setCallback([this](const auto& e){

	});

	mPauseListener.setCallback([this](const auto& e){

	});

	mInitWindowListener.setCallback([this](const auto& e){
		mSurface = eglCreateWindowSurface(mDisplay, mConfig, Platform::SystemAndroid::Instance->window, NULL);
		eglMakeCurrent(mDisplay, mSurface, mSurface, mContext);
	});

	mTerminateWindowListener.setCallback([this](const auto& e) {
		eglDestroySurface(mDisplay, mSurface);
		mSurface = EGL_NO_SURFACE;
	});
    #elif defined(PLATFORM_IOS)
    auto screen = [UIScreen mainScreen];
    auto bounds = [screen bounds];
    
    mGLKView = [[GLKView alloc] initWithFrame:bounds];
    [mGLKView setContext:[[EAGLContext alloc]initWithAPI:kEAGLRenderingAPIOpenGLES3]];
    [mGLKView setDrawableColorFormat:GLKViewDrawableColorFormatRGBA8888];
    [mGLKView setDrawableDepthFormat:GLKViewDrawableDepthFormat24];
    [mGLKView setDrawableStencilFormat:GLKViewDrawableStencilFormat8];
    [mGLKView setDrawableMultisample:GLKViewDrawableMultisampleNone];
    [EAGLContext setCurrentContext:mGLKView.context];
    [Platform::SystemIos::ViewController addSubview:mGLKView];
    #endif
#endif

	setVsync(false);

	glGenBuffers(1, &mGLVertexBuffer);
	glGenBuffers(1, &mGLIndexBuffer);
	
	setBlendMode(BlendStates::NonPremultiplied);
}

SystemGL::~SystemGL()
{
	glDeleteBuffers(1, &mGLVertexBuffer);
	glDeleteBuffers(1, &mGLIndexBuffer);
#if defined(RENDERER_GL44)
	wglDeleteContext(mWglContext);
#endif
}

void SystemGL::setTopology(const Renderer::Topology& value) 
{
	mGLTopology = SystemGL::Topology.at(value);
}

void SystemGL::setViewport(const Viewport& value) 
{
	mViewport = value;
	mViewportDirty = true;
}

void SystemGL::setScissor(const Scissor& value) 
{
	glEnable(GL_SCISSOR_TEST);
	mScissor = value;
	mScissorDirty = true;
}

void SystemGL::setScissor(std::nullptr_t value)
{
	glDisable(GL_SCISSOR_TEST);
}

void SystemGL::setVertexBuffer(const Buffer& value) 
{
	mVertexBuffer = value;
	mVertexBufferDirty = true;
}

void SystemGL::setIndexBuffer(const Buffer& value) 
{
	mIndexBuffer = value;
	mIndexBufferDirty = true;
}

void SystemGL::setTexture(std::shared_ptr<Texture> value)
{
	value->bindTexture();
	mTextureBound = true;
	updateGLSampler();
}

void SystemGL::setRenderTarget(std::shared_ptr<RenderTarget> value) 
{
	if (value == nullptr)
	{
		mRenderTargetBound = false;
#if defined(PLATFORM_IOS)
		[mGLKView bindDrawable];
#else
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
#endif
	}
	else
	{
		mRenderTargetBound = true;
		value->bindRenderTarget();
	}

	mCullModeDirty = true;  // when render target is active, we using reversed culling,
							// because image is flipped vertically
}

void SystemGL::setShader(std::shared_ptr<Shader> value)
{
	mShader = value;
	mShaderDirty = true;
}

void SystemGL::setSampler(const Sampler& value) 
{
	mSampler = value;
	updateGLSampler();
}

void SystemGL::setDepthMode(const DepthMode& value)
{
	if (value.enabled)
	{
		glEnable(GL_DEPTH_TEST);
		glDepthFunc(ComparisonFuncMap.at(value.func));
	}
	else
	{
		glDisable(GL_DEPTH_TEST);
	}
}

void SystemGL::setStencilMode(const StencilMode& value)
{
	if (!value.enabled)
	{
		glDisable(GL_STENCIL_TEST);
		return;
	}

	const static std::unordered_map<StencilOp, GLenum> StencilOpMap = {
		{ StencilOp::Keep, GL_KEEP },
		{ StencilOp::Zero, GL_ZERO },
		{ StencilOp::Replace, GL_REPLACE },
		{ StencilOp::IncrementSaturation, GL_INCR },
		{ StencilOp::DecrementSaturation, GL_DECR },
		{ StencilOp::Invert, GL_INVERT },
		{ StencilOp::Increment, GL_INCR_WRAP },
		{ StencilOp::Decrement, GL_DECR_WRAP },
	};

	glEnable(GL_STENCIL_TEST);
	glStencilMask(value.writeMask);
	glStencilOp(StencilOpMap.at(value.failOp), StencilOpMap.at(value.depthFailOp), StencilOpMap.at(value.passOp));
	glStencilFunc(ComparisonFuncMap.at(value.func), 1, value.readMask);
}

void SystemGL::setCullMode(const CullMode& value)
{
	mCullMode = value;
	mCullModeDirty = true;
}

void SystemGL::setBlendMode(const BlendMode& value)
{
	const static std::unordered_map<Blend, GLenum> BlendMap = {
		{ Blend::One, GL_ONE },
		{ Blend::Zero, GL_ZERO },
		{ Blend::SrcColor, GL_SRC_COLOR },
		{ Blend::InvSrcColor, GL_ONE_MINUS_SRC_COLOR },
		{ Blend::SrcAlpha, GL_SRC_ALPHA },
		{ Blend::InvSrcAlpha, GL_ONE_MINUS_SRC_ALPHA },
		{ Blend::DstColor, GL_DST_COLOR },
		{ Blend::InvDstColor, GL_ONE_MINUS_DST_COLOR },
		{ Blend::DstAlpha, GL_DST_ALPHA },
		{ Blend::InvDstAlpha, GL_ONE_MINUS_DST_ALPHA }
	};

	const static std::unordered_map<BlendFunction, GLenum> BlendOpMap = {
		{ BlendFunction::Add, GL_FUNC_ADD },
		{ BlendFunction::Subtract, GL_FUNC_SUBTRACT },
		{ BlendFunction::ReverseSubtract, GL_FUNC_REVERSE_SUBTRACT },
		{ BlendFunction::Min, GL_MIN },
		{ BlendFunction::Max, GL_MAX },
	};

	glEnable(GL_BLEND);
	glBlendEquationSeparate(BlendOpMap.at(value.colorBlendFunction), BlendOpMap.at(value.alphaBlendFunction));
	glBlendFuncSeparate(BlendMap.at(value.colorSrcBlend), BlendMap.at(value.colorDstBlend), BlendMap.at(value.alphaSrcBlend), BlendMap.at(value.alphaDstBlend));
}

void SystemGL::setTextureAddressMode(const TextureAddress& value)
{
	mTextureAddress = value;
	updateGLSampler();
}

void SystemGL::clear(const glm::vec4& color) 
{
	GLboolean last_enable_scissor_test = glIsEnabled(GL_SCISSOR_TEST);
	glDisable(GL_SCISSOR_TEST);
	glClearColor(color.r, color.g, color.b , color.a);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
	
	if (last_enable_scissor_test) 
		glEnable(GL_SCISSOR_TEST); 
	else 
		glDisable(GL_SCISSOR_TEST);
}

void SystemGL::draw(size_t vertexCount, size_t vertexOffset)
{
	prepareForDrawing();
	glDrawArrays(mGLTopology, (GLint)vertexOffset, (GLsizei)vertexCount);
}

void SystemGL::drawIndexed(size_t indexCount, size_t indexOffset, size_t vertexOffset)
{
	prepareForDrawing();
	int indexSize = mGLIndexType == GL_UNSIGNED_INT ? 4 : 2;
#if defined(PLATFORM_ANDROID) | defined(PLATFORM_IOS)
	glDrawElements(mGLTopology, (GLsizei)indexCount, mGLIndexType, (void*)(indexOffset * indexSize));
#else
	glDrawElementsBaseVertex(mGLTopology, (GLsizei)indexCount, mGLIndexType, (void*)(indexOffset * indexSize), (GLint)vertexOffset);
#endif
}

void SystemGL::present()
{
#if defined(RENDERER_GL44)
	SwapBuffers(mHDC);
#elif defined(RENDERER_GLES3)
    #if defined(PLATFORM_ANDROID)
	eglSwapBuffers(mDisplay, mSurface);
    #elif defined(PLATFORM_IOS)
    [mGLKView display];
    #endif
#endif
}

bool SystemGL::isVsync() const
{
#if defined(RENDERER_GL44)
	return (bool)wglGetSwapIntervalEXT();
#elif defined(RENDERER_GLES3)
	return mVsync;
#endif
}

void SystemGL::setVsync(bool value)
{
#if defined(RENDERER_GL44)
	wglSwapIntervalEXT(value ? 1 : 0);
#elif defined(RENDERER_GLES3)
    #if defined(PLATFORM_ANDROID)
        eglSwapInterval(mDisplay, value ? 1 : 0);
    #elif defined(PLATFORM_IOS)
    
    #endif
#endif
    mVsync = value;
}

void SystemGL::prepareForDrawing()
{
	glBindBuffer(GL_ARRAY_BUFFER, mGLVertexBuffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mGLIndexBuffer);

	if (mViewportDirty)
	{
		mViewportDirty = false;
		glViewport(
			(GLint)mViewport.position.x,
			mRenderTargetBound ? (GLint)mViewport.position.y : (GLint)(PLATFORM->getHeight() - mViewport.position.y - mViewport.size.y),
			(GLint)mViewport.size.x,
			(GLint)mViewport.size.y);

#if defined(RENDERER_GL44)
		glDepthRange((GLclampd)mViewport.minDepth, (GLclampd)mViewport.maxDepth);
#elif defined(RENDERER_GLES3)
		glDepthRangef((GLfloat)mViewport.minDepth, (GLfloat)mViewport.maxDepth);
#endif
	}

	if (mScissorDirty)
	{
		mScissorDirty = false;
		glScissor(
			(GLint)mScissor.position.x,
			mRenderTargetBound ? (GLint)mScissor.position.y : (GLint)(PLATFORM->getHeight() - mScissor.position.y - mScissor.size.y),
			(GLint)mScissor.size.x,
			(GLint)mScissor.size.y);
	}

	// shader

	if (mShaderDirty)
	{
		mShader->apply();
		mShaderDirty = false;
	}

	mShader->update();

	// opengl crashes when index or vertex buffers are binded before VAO from shader classes 

	if (mIndexBufferDirty) 
	{
		setGLIndexBuffer(mIndexBuffer);
		mIndexBufferDirty = false;
	}

	if (mVertexBufferDirty)
	{
		setGLVertexBuffer(mVertexBuffer);
		mVertexBufferDirty = false;
	}

	if (mCullModeDirty)
	{
		setGLCullMode(mCullMode);
		mCullModeDirty = false;
	}
}

void SystemGL::setGLVertexBuffer(const Buffer& value)
{
	glBindBuffer(GL_ARRAY_BUFFER, mGLVertexBuffer);
	
	GLint size = 0;
	glGetBufferParameteriv(GL_ARRAY_BUFFER, GL_BUFFER_SIZE, &size);

	if ((size_t)size < value.size)
		glBufferData(GL_ARRAY_BUFFER, value.size, value.data, GL_STATIC_DRAW);
	else
		glBufferSubData(GL_ARRAY_BUFFER, 0, value.size, value.data);
	
	//glBufferData(GL_ARRAY_BUFFER, value.size, nullptr, GL_STATIC_DRAW);
	//auto ptr = glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY);
	//memcpy(ptr, value.data, value.size);
	//glUnmapBuffer(GL_ARRAY_BUFFER);
}

void SystemGL::setGLIndexBuffer(const Buffer& value)
{
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mGLIndexBuffer);

	GLint size = 0;
	glGetBufferParameteriv(GL_ELEMENT_ARRAY_BUFFER, GL_BUFFER_SIZE, &size);

	if ((size_t)size < value.size)
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, value.size, value.data, GL_STATIC_DRAW);
	else
		glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, value.size, value.data);

	//glBufferData(GL_ELEMENT_ARRAY_BUFFER, value.size, nullptr, GL_STATIC_DRAW);
	//auto ptr = glMapBuffer(GL_ELEMENT_ARRAY_BUFFER, GL_WRITE_ONLY);
	//memcpy(ptr, value.data, value.size);
	//glUnmapBuffer(GL_ELEMENT_ARRAY_BUFFER);

	mGLIndexType = value.stride == 2 ? GL_UNSIGNED_SHORT : GL_UNSIGNED_INT;
}

void SystemGL::setGLCullMode(const CullMode& value)
{
	if (value != CullMode::None)
	{
		glEnable(GL_CULL_FACE);
		glFrontFace(GL_CW);

		auto cull = value;

		if (IsRenderTargetBound())
		{
			if (cull == CullMode::Back)
				cull = CullMode::Front;
			else
				cull = CullMode::Back;
		}

		glCullFace(CullMap.at(cull));	
	}
	else
	{
		glDisable(GL_CULL_FACE);
	}
}

void SystemGL::updateGLSampler()
{
	if (!mTextureBound)
		return;

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, mSampler == Sampler::Linear ? GL_LINEAR : GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, mSampler == Sampler::Linear ? GL_LINEAR : GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, mTextureAddress == TextureAddress::Clamp ? GL_CLAMP_TO_EDGE : GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, mTextureAddress == TextureAddress::Clamp ? GL_CLAMP_TO_EDGE : GL_REPEAT);
}
#endif
