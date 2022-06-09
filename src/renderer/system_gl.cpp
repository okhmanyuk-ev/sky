#include "system_gl.h"

#if defined(RENDERER_GL44) || defined(RENDERER_GLES3)
#include <platform/system_windows.h>
#include <platform/system_android.h>
#include <platform/system_ios.h>
#include <console/device.h>
#include <skygfx/shader_compiler.h>

using namespace Renderer;

struct Shader::Impl
{
	Vertex::Layout layout;
	GLuint program;
	GLuint vao;
	std::map<Vertex::Attribute::Type, GLint> attribLocations;
};

Shader::Shader(const Vertex::Layout& layout, const std::string& vertex_code, const std::string& fragment_code)
{
	mImpl = std::make_unique<Impl>();
	mImpl->layout = layout;

	std::vector<std::string> defines;
	AddLocationDefines(layout, defines);
	defines.push_back("FLIP_TEXCOORD_Y");

	auto vertex_shader_spirv = skygfx::CompileGlslToSpirv(skygfx::ShaderStage::Vertex, vertex_code, defines);
	auto fragment_shader_spirv = skygfx::CompileGlslToSpirv(skygfx::ShaderStage::Fragment, fragment_code, defines);

	auto glsl_vert = skygfx::CompileSpirvToGlsl(vertex_shader_spirv);
	auto glsl_frag = skygfx::CompileSpirvToGlsl(fragment_shader_spirv);

	auto vertexShader = glCreateShader(GL_VERTEX_SHADER);
	auto v = glsl_vert.c_str();
	glShaderSource(vertexShader, 1, &v, NULL);
	glCompileShader(vertexShader);

	GLint isCompiled = 0;
	glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &isCompiled);
	if (isCompiled == GL_FALSE)
	{
		GLint maxLength = 0;
		glGetShaderiv(vertexShader, GL_INFO_LOG_LENGTH, &maxLength);
		std::string errorLog;
		errorLog.resize(maxLength);
		glGetShaderInfoLog(vertexShader, maxLength, &maxLength, &errorLog[0]);
		throw std::runtime_error(errorLog);
	}

	auto fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	auto f = glsl_frag.c_str();
	glShaderSource(fragmentShader, 1, &f, NULL);
	glCompileShader(fragmentShader);

	isCompiled = 0;
	glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &isCompiled);
	if (isCompiled == GL_FALSE)
	{
		GLint maxLength = 0;
		glGetShaderiv(fragmentShader, GL_INFO_LOG_LENGTH, &maxLength);
		std::string errorLog;
		errorLog.resize(maxLength);
		glGetShaderInfoLog(fragmentShader, maxLength, &maxLength, &errorLog[0]);
		throw std::runtime_error(errorLog);
	}

	mImpl->program = glCreateProgram();
	glAttachShader(mImpl->program, vertexShader);
	glAttachShader(mImpl->program, fragmentShader);
	glLinkProgram(mImpl->program);
	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);

	glGenVertexArrays(1, &mImpl->vao);
	glBindVertexArray(mImpl->vao);

	int i = 0;

	for (auto& attrib : layout.attributes)
	{
		glEnableVertexAttribArray(i);
		mImpl->attribLocations[attrib.type] = i;
		i++;
	}
	glBindVertexArray(0);
}

Shader::~Shader()
{
	glDeleteVertexArrays(1, &mImpl->vao);
	glDeleteProgram(mImpl->program);
}

void Shader::apply()
{
	glUseProgram(mImpl->program);
	glBindVertexArray(mImpl->vao);

	for (auto& attrib : mImpl->layout.attributes)
	{
		if (mImpl->attribLocations.count(attrib.type) == 0)
			continue;

		glVertexAttribPointer(mImpl->attribLocations.at(attrib.type), SystemGL::Size.at(attrib.format),
			SystemGL::Type.at(attrib.format), SystemGL::Normalize.at(attrib.format), (GLsizei)mImpl->layout.stride,
			(void*)attrib.offset);
	}
}

struct Texture::TextureImpl
{
	GLuint texture;
};

Texture::Texture(int width, int height, int channels, void* data, bool mipmap) :
	mWidth(width),
	mHeight(height),
	mMipmap(mipmap)
{
	mTextureImpl = std::make_unique<TextureImpl>();

	GLint last_texture;
	glGetIntegerv(GL_TEXTURE_BINDING_2D, &last_texture);

	glGenTextures(1, &mTextureImpl->texture);
	glBindTexture(GL_TEXTURE_2D, mTextureImpl->texture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

	if (data)
	{
		auto temp_data = malloc(width * height * 4);
		const auto row_size = width * 4;

		for (int i = 0; i < height; i++)
		{
			auto src = (void*)(size_t(data) + size_t(i) * row_size);
			auto dst = (void*)(size_t(temp_data) + size_t(height - 1 - i) * row_size);
			memcpy(dst, src, row_size);
		}

		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, temp_data);
		free(temp_data);

		if (mMipmap)
			glGenerateMipmap(GL_TEXTURE_2D);
	}

	glBindTexture(GL_TEXTURE_2D, last_texture);
}

Texture::~Texture()
{
	glDeleteTextures(1, &mTextureImpl->texture);
}

struct RenderTarget::RenderTargetImpl
{
	GLuint framebuffer;
	GLuint depth_stencil_renderbuffer;
};

RenderTarget::RenderTarget(int width, int height) : Texture(width, height, 4, nullptr)
{
	mRenderTargetImpl = std::make_unique<RenderTargetImpl>();

	GLint last_fbo;
	GLint last_rbo;

	glGetIntegerv(GL_FRAMEBUFFER_BINDING, &last_fbo);
	glGetIntegerv(GL_RENDERBUFFER_BINDING, &last_rbo);

	glGenFramebuffers(1, &mRenderTargetImpl->framebuffer);
	glGenRenderbuffers(1, &mRenderTargetImpl->depth_stencil_renderbuffer);

	glBindFramebuffer(GL_FRAMEBUFFER, mRenderTargetImpl->framebuffer);
	glBindRenderbuffer(GL_RENDERBUFFER, mRenderTargetImpl->depth_stencil_renderbuffer);

	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, mTextureImpl->texture, 0);

	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, mRenderTargetImpl->depth_stencil_renderbuffer);

	assert(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE);

	glBindFramebuffer(GL_FRAMEBUFFER, last_fbo);
	glBindRenderbuffer(GL_RENDERBUFFER, last_rbo);
}

RenderTarget::~RenderTarget()
{
	glDeleteFramebuffers(1, &mRenderTargetImpl->framebuffer);
	glDeleteRenderbuffers(1, &mRenderTargetImpl->depth_stencil_renderbuffer);
}

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
	static const std::unordered_map<GLenum, std::string> SourceMap = {
		{ GL_DEBUG_SOURCE_API, "GL_DEBUG_SOURCE_API" },
		{ GL_DEBUG_SOURCE_WINDOW_SYSTEM, "GL_DEBUG_SOURCE_WINDOW_SYSTEM" },
		{ GL_DEBUG_SOURCE_SHADER_COMPILER, "GL_DEBUG_SOURCE_SHADER_COMPILER" },
		{ GL_DEBUG_SOURCE_THIRD_PARTY, "GL_DEBUG_SOURCE_THIRD_PARTY" },
		{ GL_DEBUG_SOURCE_APPLICATION, "GL_DEBUG_SOURCE_APPLICATION" },
		{ GL_DEBUG_SOURCE_OTHER, "GL_DEBUG_SOURCE_OTHER" },
	};
	
	static const std::unordered_map<GLenum, std::string> TypeMap = {
		{ GL_DEBUG_TYPE_ERROR, "GL_DEBUG_TYPE_ERROR" },
		{ GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR, "GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR" },
		{ GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR, "GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR" },
		{ GL_DEBUG_TYPE_PORTABILITY, "GL_DEBUG_TYPE_PORTABILITY" },
		{ GL_DEBUG_TYPE_PERFORMANCE, "GL_DEBUG_TYPE_PERFORMANCE" },
		{ GL_DEBUG_TYPE_MARKER, "GL_DEBUG_TYPE_MARKER" },
		{ GL_DEBUG_TYPE_PUSH_GROUP, "GL_DEBUG_TYPE_PUSH_GROUP" },
		{ GL_DEBUG_TYPE_POP_GROUP, "GL_DEBUG_TYPE_POP_GROUP" },
		{ GL_DEBUG_TYPE_OTHER, "GL_DEBUG_TYPE_OTHER" },
	};

	static const std::unordered_map<GLenum, std::string> SeverityMap = {
		{ GL_DEBUG_SEVERITY_HIGH, "GL_DEBUG_SEVERITY_HIGH" },
		{ GL_DEBUG_SEVERITY_MEDIUM, "GL_DEBUG_SEVERITY_MEDIUM" },
		{ GL_DEBUG_SEVERITY_LOW, "GL_DEBUG_SEVERITY_LOW" },
		{ GL_DEBUG_SEVERITY_NOTIFICATION, "GL_DEBUG_SEVERITY_NOTIFICATION" },
	};

	std::string source_str = "unknown";
	std::string type_str = "unknown";
	std::string severity_str = "unknown";

	if (SourceMap.contains(source))
		source_str = SourceMap.at(source);

	if (TypeMap.contains(type))
		type_str = TypeMap.at(type);

	if (SeverityMap.contains(severity))
		severity_str = SeverityMap.at(severity);

	if (type == GL_DEBUG_TYPE_OTHER)
		return;

	LOGF("[OpenGL] source: {}, type: {}, id: {}, severity: {}, msg: {}", source_str, type_str, id, severity_str, message);
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

	glEnable(GL_DEBUG_OUTPUT);
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
    auto window = Platform::SystemIos::Window;
    auto rootView = [[window rootViewController] view];
    mGLKView = [[GLKView alloc] initWithFrame:[window frame]];
    [mGLKView setContext:[[EAGLContext alloc]initWithAPI:kEAGLRenderingAPIOpenGLES3]];
    [mGLKView setDrawableColorFormat:GLKViewDrawableColorFormatRGBA8888];
    [mGLKView setDrawableDepthFormat:GLKViewDrawableDepthFormat24];
    [mGLKView setDrawableStencilFormat:GLKViewDrawableStencilFormat8];
    [mGLKView setDrawableMultisample:GLKViewDrawableMultisampleNone];
    [mGLKView setAutoresizingMask:UIViewAutoresizingFlexibleWidth|UIViewAutoresizingFlexibleHeight];
    [EAGLContext setCurrentContext:mGLKView.context];
    [rootView addSubview:mGLKView];
    #endif
#endif

	setVsync(false);

	glGenBuffers(1, &mGLVertexBuffer);
	glGenBuffers(1, &mGLIndexBuffer);
	glGenBuffers(1, &mGLPixelBuffer);
	
	setBlendMode(BlendStates::NonPremultiplied);
}

SystemGL::~SystemGL()
{
	glDeleteBuffers(1, &mGLPixelBuffer);
	glDeleteBuffers(1, &mGLVertexBuffer);
	glDeleteBuffers(1, &mGLIndexBuffer);
	for (auto [slot, ubo] : mGLUniformBuffers)
	{
		glDeleteBuffers(1, &ubo);
	}
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
	glViewport(
		(GLint)value.position.x,
		(GLint)value.position.y,
		(GLint)value.size.x,
		(GLint)value.size.y);

#if defined(RENDERER_GL44)
	glDepthRange((GLclampd)value.minDepth, (GLclampd)value.maxDepth);
#elif defined(RENDERER_GLES3)
	glDepthRangef((GLfloat)value.minDepth, (GLfloat)value.maxDepth);
#endif
}

void SystemGL::setScissor(const Scissor& value) 
{
	glEnable(GL_SCISSOR_TEST);
	glScissor(
		(GLint)glm::round(value.position.x),
		(GLint)glm::round(PLATFORM->getHeight() - value.position.y - value.size.y),
		(GLint)glm::round(value.size.x),
		(GLint)glm::round(value.size.y));
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

void SystemGL::setUniformBuffer(int slot, void* memory, size_t size)
{
	if (!mGLUniformBuffers.contains(slot))
	{
		glGenBuffers(1, &mGLUniformBuffers[slot]);
	}

	glBindBufferBase(GL_UNIFORM_BUFFER, slot, mGLUniformBuffers.at(slot));
	glBufferData(GL_UNIFORM_BUFFER, size, memory, GL_DYNAMIC_DRAW);
}

void SystemGL::setTexture(int binding, std::shared_ptr<Texture> value)
{
	if (value == nullptr)
		return;

	glActiveTexture(GL_TEXTURE0 + binding);
	glBindTexture(GL_TEXTURE_2D, value->mTextureImpl->texture);
	mTextureBound = true;
	updateGLSampler();
}

void SystemGL::setTexture(std::shared_ptr<Texture> value)
{
	setTexture(0, value);
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
		glBindFramebuffer(GL_FRAMEBUFFER, value->mRenderTargetImpl->framebuffer);
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
	glStencilFunc(ComparisonFuncMap.at(value.func), value.reference, value.readMask);
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
	glColorMask(value.colorMask.red, value.colorMask.green, value.colorMask.blue, value.colorMask.alpha);
}

void SystemGL::setTextureAddressMode(const TextureAddress& value)
{
	mTextureAddress = value;
	updateGLSampler();
}

void SystemGL::clear(std::optional<glm::vec4> color, std::optional<float> depth, std::optional<uint8_t> stencil)
{
	auto scissor_was_enabled = glIsEnabled(GL_SCISSOR_TEST);
	
	if (scissor_was_enabled)
	{
		glDisable(GL_SCISSOR_TEST);
	}

	GLbitfield flags = 0;

	if (color.has_value())
	{
		flags |= GL_COLOR_BUFFER_BIT;
		auto _color = color.value();
		glClearColor(_color.r, _color.g, _color.b, _color.a);
	}

	if (depth.has_value())
	{
		flags |= GL_DEPTH_BUFFER_BIT;
		glClearDepthf(depth.value());
	}

	if (stencil.has_value())
	{
		flags |= GL_STENCIL_BUFFER_BIT;
		glClearStencil(stencil.value());
	}

	glClear(flags);

	if (scissor_was_enabled)
	{
		glEnable(GL_SCISSOR_TEST);
	}
}

void SystemGL::draw(size_t vertexCount, size_t vertexOffset)
{
	System::draw(vertexCount, vertexOffset);
	prepareForDrawing();
	glDrawArrays(mGLTopology, (GLint)vertexOffset, (GLsizei)vertexCount);
}

void SystemGL::drawIndexed(size_t indexCount, size_t indexOffset, size_t vertexOffset)
{
	System::drawIndexed(indexCount, indexOffset, vertexOffset);
	prepareForDrawing();
	int indexSize = mGLIndexType == GL_UNSIGNED_INT ? 4 : 2;
#if defined(PLATFORM_ANDROID) | defined(PLATFORM_IOS)
	glDrawElements(mGLTopology, (GLsizei)indexCount, mGLIndexType, (void*)(indexOffset * indexSize));
#else
	glDrawElementsBaseVertex(mGLTopology, (GLsizei)indexCount, mGLIndexType, (void*)(indexOffset * indexSize), (GLint)vertexOffset);
#endif
}

void SystemGL::readPixels(const glm::ivec2& pos, const glm::ivec2& size, std::shared_ptr<Renderer::Texture> dst_texture)
{
	assert(dst_texture->getWidth() == size.x);
	assert(dst_texture->getHeight() == size.y);

	if (size.x <= 0 || size.y <= 0)
		return;

	auto x = (GLint)pos.x;
	auto y = (GLint)(PLATFORM->getHeight() - pos.y - size.y);
	auto w = (GLint)size.x;
	auto h = (GLint)size.y;
	
	glBindBuffer(GL_PIXEL_PACK_BUFFER, mGLPixelBuffer);
	glBufferData(GL_PIXEL_PACK_BUFFER, w * h * 4, nullptr, GL_STATIC_READ);
	glReadPixels(x, y, w, h, GL_RGBA, GL_UNSIGNED_BYTE, 0);
	glBindBuffer(GL_PIXEL_PACK_BUFFER, 0);

	glBindTexture(GL_TEXTURE_2D, dst_texture->mTextureImpl->texture);
	glBindBuffer(GL_PIXEL_UNPACK_BUFFER, mGLPixelBuffer);
	
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);

	if (dst_texture->isMipmap())
		glGenerateMipmap(GL_TEXTURE_2D);

	glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);
}

void SystemGL::present()
{
	System::present();
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
    {
        glBufferData(GL_ARRAY_BUFFER, value.size, value.data, GL_DYNAMIC_DRAW);
    }
	else
    {
        auto ptr = glMapBufferRange(GL_ARRAY_BUFFER, 0, size, GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT);
        memcpy(ptr, value.data, value.size);
        glUnmapBuffer(GL_ARRAY_BUFFER);
    }
}

void SystemGL::setGLIndexBuffer(const Buffer& value)
{
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mGLIndexBuffer);

	GLint size = 0;
	glGetBufferParameteriv(GL_ELEMENT_ARRAY_BUFFER, GL_BUFFER_SIZE, &size);

	if ((size_t)size < value.size)
    {
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, value.size, value.data, GL_DYNAMIC_DRAW);
    }
    else
	{
        auto ptr = glMapBufferRange(GL_ELEMENT_ARRAY_BUFFER, 0, size, GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT);
        memcpy(ptr, value.data, value.size);
        glUnmapBuffer(GL_ELEMENT_ARRAY_BUFFER);

    }

	mGLIndexType = value.stride == 2 ? GL_UNSIGNED_SHORT : GL_UNSIGNED_INT;
}

void SystemGL::setGLCullMode(const CullMode& value)
{
	if (value != CullMode::None)
	{
		glEnable(GL_CULL_FACE);
		glFrontFace(GL_CW);
		glCullFace(CullMap.at(value));
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

	if (mSampler == Sampler::Linear)
	{
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	}
	else if (mSampler == Sampler::Nearest)
	{
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	}
	else if (mSampler == Sampler::LinearMipmapLinear)
	{
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR); // mag parameter support only linear or nearest filters
	}

	if (mTextureAddress == TextureAddress::Clamp)
	{
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	}
	else if (mTextureAddress == TextureAddress::Wrap)
	{
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	}
	else if (mTextureAddress == TextureAddress::MirrorWrap)
	{
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);
	}
}

#endif
