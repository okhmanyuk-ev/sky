#include "system_gl.h"

#if defined(RENDERER_GL44) || defined(RENDERER_GLES3)
#include <Platform/system_windows.h>
#include <Platform/system_android.h>
#include <Platform/system_ios.h>

using namespace Renderer;

bool SystemGL::IsRenderTargetBound() 
{
	return mRenderTargetBound;
}

bool SystemGL::mRenderTargetBound = false;

std::map<Topology, GLenum> SystemGL::Topology = {
	{ Topology::PointList, GL_POINTS },
	{ Topology::LineList, GL_LINES },
	{ Topology::LineStrip, GL_LINE_STRIP },
	{ Topology::TriangleList, GL_TRIANGLES },
	{ Topology::TriangleStrip, GL_TRIANGLE_STRIP }
};

std::map<Vertex::Attribute::Format, GLint> SystemGL::Size = {
	{ Vertex::Attribute::Format::R32F, 1 },
	{ Vertex::Attribute::Format::R32G32F, 2 },
	{ Vertex::Attribute::Format::R32G32B32F, 3 },
	{ Vertex::Attribute::Format::R32G32B32A32F, 4 },
	{ Vertex::Attribute::Format::R8UN, 1 },
	{ Vertex::Attribute::Format::R8G8UN, 2 },
	{ Vertex::Attribute::Format::R8G8B8UN, 3 },
	{ Vertex::Attribute::Format::R8G8B8A8UN, 4 }
};

std::map<Vertex::Attribute::Format, GLenum> SystemGL::Type = {
	{ Vertex::Attribute::Format::R32F, GL_FLOAT },
	{ Vertex::Attribute::Format::R32G32F, GL_FLOAT },
	{ Vertex::Attribute::Format::R32G32B32F, GL_FLOAT },
	{ Vertex::Attribute::Format::R32G32B32A32F, GL_FLOAT },
	{ Vertex::Attribute::Format::R8UN, GL_UNSIGNED_BYTE },
	{ Vertex::Attribute::Format::R8G8UN, GL_UNSIGNED_BYTE },
	{ Vertex::Attribute::Format::R8G8B8UN, GL_UNSIGNED_BYTE },
	{ Vertex::Attribute::Format::R8G8B8A8UN, GL_UNSIGNED_BYTE }
};

std::map<Vertex::Attribute::Format, GLboolean> SystemGL::Normalize = {
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
	std::map<DepthMode, GLenum> glcomparisonfunc = {
		{ DepthMode::None, GL_ALWAYS },
		{ DepthMode::Always, GL_ALWAYS },
		{ DepthMode::Never, GL_NEVER },
		{ DepthMode::Less, GL_LESS },
		{ DepthMode::Equal, GL_EQUAL },
		{ DepthMode::NotEqual, GL_NOTEQUAL },
		{ DepthMode::LessEqual, GL_LEQUAL },
		{ DepthMode::Greater, GL_GREATER },
		{ DepthMode::GreaterEqual, GL_GEQUAL }
	};

	std::map<CullMode, GLenum> glcull = {
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

@interface ViewController : GLKViewController

@end

@implementation ViewController

- (id)init
{
    auto result = [super init];
    GLKView* view  = (GLKView*)self.view;

   // view.drawableColorFormat = GLKViewDrawableColorFormatRGBA8888;
   // view.drawableDepthFormat = GLKViewDrawableDepthFormat24;
   // view.drawableStencilFormat = GLKViewDrawableStencilFormat8;

    view.context = [[EAGLContext alloc]initWithAPI:kEAGLRenderingAPIOpenGLES3];
    [EAGLContext setCurrentContext:view.context];
    return result;
}

-(void)glkView:(GLKView *)view drawInRect:(CGRect)rect
{
    //glClear(GL_COLOR_BUFFER_BIT);
}

- (void)dealloc
{
    GLKView* view = (GLKView*)self.view;
    [EAGLContext setCurrentContext:view.context];
    [EAGLContext setCurrentContext:nil];
    [super dealloc];
}


-(void)touchesBegan:(NSSet*)touches withEvent:(__unused ::UIEvent*)event
{
    for (UITouch* touch in touches)
    {
        auto location = [touch locationInView:self.view];
        auto e = Platform::Touch::Event();
        e.type = Platform::Touch::Event::Type::Begin;
        e.x = location.x * PLATFORM->getScale();
        e.y = location.y * PLATFORM->getScale();
        EVENT->emit(e);
        break;
    }
}

- (void)touchesMoved:(NSSet*)touches withEvent:(__unused ::UIEvent*)event
{
    for (UITouch* touch in touches)
    {
        auto location = [touch locationInView:self.view];
        auto e = Platform::Touch::Event();
        e.type = Platform::Touch::Event::Type::Continue;
        e.x = location.x * PLATFORM->getScale();
        e.y = location.y * PLATFORM->getScale();
        EVENT->emit(e);
        break;
    }
}

- (void)touchesCancelled:(NSSet*)touches withEvent:(__unused ::UIEvent*)event
{
    for (UITouch* touch in touches)
    {
        auto location = [touch locationInView:self.view];
        auto e = Platform::Touch::Event();
        e.type = Platform::Touch::Event::Type::End;
        e.x = location.x * PLATFORM->getScale();
        e.y = location.y * PLATFORM->getScale();
        EVENT->emit(e);
        break;
    }
}

- (void)touchesEnded:(NSSet*)touches withEvent:(__unused ::UIEvent*)event
{
    for (UITouch* touch in touches)
    {
        auto location = [touch locationInView:self.view];
        auto e = Platform::Touch::Event();
        e.type = Platform::Touch::Event::Type::End;
        e.x = location.x * PLATFORM->getScale();
        e.y = location.y * PLATFORM->getScale();
        EVENT->emit(e);
        break;
    }
}

- (void)viewWillTransitionToSize:(CGSize)size withTransitionCoordinator:(id<UIViewControllerTransitionCoordinator>)coordinator
{
    static_cast<Platform::SystemIos*>(PLATFORM)->setSize(size.width * PLATFORM->getScale(), size.height * PLATFORM->getScale());
}

-(void)textFieldDidChange:(UITextField *) textField
{
    auto c = textField.text;
    textField.text = @"";
    auto e = Platform::Keyboard::Event();
    
    e.asciiChar = std::string([c UTF8String])[0];
    e.key = Platform::Keyboard::Key::None;

    e.type = Platform::Keyboard::Event::Type::Pressed;
    EVENT->emit(e);
    
    e.type = Platform::Keyboard::Event::Type::Released;
    EVENT->emit(e);
}

@end
#endif

SystemGL::SystemGL()
{
#if defined(RENDERER_GL44)
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

	mHDC = GetDC(Platform::SystemWindows::Window);

	int nPixelFormat = ChoosePixelFormat(mHDC, &pfd);

	SetPixelFormat(mHDC, nPixelFormat, &pfd);

	HGLRC tempContext = wglCreateContext(mHDC);
	wglMakeCurrent(mHDC, tempContext);

	glewInit();
	
	int attribs[] =
	{
		WGL_CONTEXT_MAJOR_VERSION_ARB, 4,
		WGL_CONTEXT_MINOR_VERSION_ARB, 4,
		WGL_CONTEXT_PROFILE_MASK_ARB, WGL_CONTEXT_CORE_PROFILE_BIT_ARB,
		WGL_CONTEXT_FLAGS_ARB, /*WGL_CONTEXT_DEBUG_BIT_ARB*/0,
		0
	};

	if (wglewIsSupported("WGL_ARB_create_context") == 1)
	{
		mWglContext = wglCreateContextAttribsARB(mHDC, 0, attribs);
		wglMakeCurrent(NULL, NULL);
		wglDeleteContext(tempContext);
		wglMakeCurrent(mHDC, mWglContext);
	}
	else
	{	
		mWglContext = tempContext;
	}

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
    auto view = [[ViewController alloc] init];
    [view.view addSubview:Platform::SystemIos::TextField];
    Platform::SystemIos::Window.rootViewController = view;
    [Platform::SystemIos::TextField addTarget:view action:@selector(textFieldDidChange:) forControlEvents:UIControlEventEditingChanged];
    
    #endif
#endif

	setVsync(false);

	glGenBuffers(1, &mGLVertexBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, mGLVertexBuffer);

	setBlendMode(BlendStates::NonPremultiplied);
}

SystemGL::~SystemGL()
{
	glDeleteBuffers(1, &mGLVertexBuffer);
#if defined(RENDERER_GL44)
	wglDeleteContext(mWglContext);
#endif
}

void SystemGL::setTopology(const Renderer::Topology& value) 
{
	mGLTopology = SystemGL::Topology[value];
}

void SystemGL::setViewport(const Viewport& value) 
{
	//assert(value.size.x > 0.0f && value.size.y > 0.0f);
	mViewportState = value;
}

void SystemGL::setScissor(const Scissor& value) 
{
	//assert(value.size.x > 0.0f && value.size.y > 0.0f);
	glEnable(GL_SCISSOR_TEST);
	mScissorState = value;
}

void SystemGL::setScissor(std::nullptr_t value)
{
	glDisable(GL_SCISSOR_TEST);
}

void SystemGL::setVertexBuffer(const Buffer& value) 
{
	glBufferData(GL_ARRAY_BUFFER, value.size, value.data, GL_STATIC_DRAW);
}

void SystemGL::setIndexBuffer(const Buffer& value) 
{
	mIndexBuffer = value;
}

void SystemGL::setTexture(const Texture& value) 
{
	value.bindTexture();
	mTextureBound = true;
	updateGLSampler();
}

void SystemGL::setRenderTarget(std::shared_ptr<RenderTarget> value) 
{
	if (value == nullptr)
	{
		mRenderTargetBound = false;
#if defined(PLATFORM_IOS)
		auto view = (GLKView*)Platform::SystemIos::Window.rootViewController.view;
		[view bindDrawable] ;
#else
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
#endif
	}
	else
	{
		mRenderTargetBound = true;
		value->bindRenderTarget();
	}
}

void SystemGL::setShader(std::shared_ptr<Shader> value)
{
	mStateShader = value;
}

void SystemGL::setSampler(const Sampler& value) 
{
	mSampler = value;
	updateGLSampler();
}

void SystemGL::setDepthMode(const DepthMode& value)
{
	mStateDepthMode = value;
}

void SystemGL::setCullMode(const CullMode& value)
{
	mStateCullMode = value;
}

void SystemGL::setBlendMode(const BlendMode& value)
{
	const std::map<Blend, GLenum> BlendMap = {
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

	const std::map<BlendFunction, GLenum> BlendOpMap = {
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
#if defined(RENDERER_GL44) // TODO: fix
	glDrawElementsBaseVertex(mGLTopology, (GLsizei)indexCount,
		mIndexBuffer.stride == 2 ? GL_UNSIGNED_SHORT : GL_UNSIGNED_INT,
		(uint16_t*)mIndexBuffer.data + (int)indexOffset, (GLint)vertexOffset);
#elif defined(RENDERER_GLES3)
	glDrawElements(mGLTopology, indexCount,
		mIndexBuffer.stride == 2 ? GL_UNSIGNED_SHORT : GL_UNSIGNED_INT,
		(uint16_t*)mIndexBuffer.data + indexOffset);
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
    auto view = (GLKView*)Platform::SystemIos::Window.rootViewController.view;
    [view display];
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
	if (mViewport != mViewportState)
	{
		mViewport = mViewportState;

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

	if (mScissor != mScissorState)
	{
		mScissor = mScissorState;
		glScissor(
			(GLint)mScissor.position.x,
			mRenderTargetBound ? (GLint)mScissor.position.y : (GLint)(PLATFORM->getHeight() - mScissor.position.y - mScissor.size.y),
			(GLint)mScissor.size.x,
			(GLint)mScissor.size.y);
	}

	// shader

	auto shader = mStateShader;
	assert(shader != nullptr);

	if (mAppliedShader != shader)
	{
		shader->apply();
		mAppliedShader = shader;
	}

	shader->update();

	// depth mode

	auto depthMode = mStateDepthMode;
	if (!mDepthModeApplied || mDepthMode != depthMode)
	{
		setGLDepthMode(depthMode);
		mDepthMode = depthMode;
		mDepthModeApplied = true;
	}

	// cull mode

	auto cullMode = mStateCullMode;
	if (!mCullModeApplied || mCullMode != cullMode)
	{
		setGLCullMode(cullMode);
		mCullMode = cullMode;
		mCullModeApplied = true;
	}	
}

void SystemGL::setGLDepthMode(DepthMode depthMode)
{
	if (depthMode != DepthMode::None)
	{
		glEnable(GL_DEPTH_TEST);
		glDepthFunc(glcomparisonfunc.at(depthMode));
	}
	else
	{
		glDisable(GL_DEPTH_TEST);
	}
}

void SystemGL::setGLCullMode(CullMode cullMode)
{
	if (cullMode != CullMode::None)
	{
		glEnable(GL_CULL_FACE);
		glFrontFace(GL_CW);
		glCullFace(glcull.at(cullMode));
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
}

#endif