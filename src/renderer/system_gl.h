#pragma once

#include <renderer/system.h>

#if defined(RENDERER_GL44) || defined(RENDERER_GLES3)

#include <platform/system.h>
#include <common/event_system.h>
#include <renderer/defines.h>
#include <renderer/vertex.h>
#include <renderer/topology.h>
#include <renderer/low_level_api.h>

#include <platform/system_android.h>
#include <map>

namespace Renderer
{
	class SystemGL : public System,
		Common::Event::Listenable<Platform::System::ResizeEvent>
	{
	public:
		SystemGL();
		~SystemGL();

	private:
		void onEvent(const Platform::System::ResizeEvent& e) override;

    public:
		void setTopology(const Topology& value) override;
		void setViewport(std::optional<Viewport> value) override;
		void setScissor(std::optional<Scissor> value) override;
		void setVertexBuffer(const Buffer& value) override;
		void setIndexBuffer(const Buffer& value) override;
		void setUniformBuffer(int slot, void* memory, size_t size) override;
		void setTexture(int binding, std::shared_ptr<Texture> value) override;
		void setTexture(std::shared_ptr<Texture> value) override;
		void setRenderTarget(std::shared_ptr<RenderTarget> value) override;
		void setShader(std::shared_ptr<Shader> value) override;
		void setSampler(const Sampler& value) override;
		void setDepthMode(const DepthMode& value) override;
		void setStencilMode(const StencilMode& value) override;
		void setCullMode(const CullMode& value) override;
		void setBlendMode(const BlendMode& value) override;
		void setTextureAddressMode(const TextureAddress& value) override;

		void clear(std::optional<glm::vec4> color = glm::vec4{ 0.0f, 0.0f, 0.0f, 0.0f },
			std::optional<float> depth = 1.0f, std::optional<uint8_t> stencil = 0) override;

		void draw(size_t vertexCount, size_t vertexOffset = 0) override;
		void drawIndexed(size_t indexCount, size_t indexOffset = 0, size_t vertexOffset = 0) override;

		void readPixels(const glm::ivec2& pos, const glm::ivec2& size, std::shared_ptr<Renderer::Texture> dst_texture) override;

		void present() override;

		bool isVsync() const override;
		void setVsync(bool value) override;
        
	private:
		bool mVsync = false;

#if defined(RENDERER_GLES3) && defined(PLATFORM_ANDROID)
		Common::Event::Listener<Platform::SystemAndroid::ResumeEvent> mResumeListener;
		Common::Event::Listener<Platform::SystemAndroid::PauseEvent> mPauseListener;
		Common::Event::Listener<Platform::SystemAndroid::InitWindowEvent> mInitWindowListener;
		Common::Event::Listener<Platform::SystemAndroid::TerminateWindowEvent> mTerminateWindowListener;
#endif

#if defined(RENDERER_GL44)
	private:
		HGLRC mWglContext;
		HDC mHDC;
#elif defined(RENDERER_GLES3) && defined(PLATFORM_ANDROID)
	private:
		EGLDisplay mDisplay;
		EGLSurface mSurface;
		EGLContext mContext;
		EGLConfig mConfig;
#endif

	public:
		static const std::unordered_map<Topology, GLenum> Topology;
		static const std::unordered_map<Vertex::Attribute::Format, GLint> Size;
		static const std::unordered_map<Vertex::Attribute::Format, GLenum> Type;
		static const std::unordered_map<Vertex::Attribute::Format, GLboolean> Normalize;

	private:
		void prepareForDrawing();

	private:
		GLenum mGLTopology;
		GLuint mGLVertexBuffer;
		GLuint mGLIndexBuffer;
		GLuint mGLPixelBuffer;
		std::map<int, GLuint> mGLUniformBuffers;
		GLenum mGLIndexType;

		Buffer mIndexBuffer;
		bool mIndexBufferDirty = false;

		Buffer mVertexBuffer;
		bool mVertexBufferDirty = false;

		std::shared_ptr<Shader> mShader = nullptr;
		bool mShaderDirty = false;

		std::unordered_map<uint32_t, std::shared_ptr<Texture>> mCurrentTextures;
		Sampler mSampler = Sampler::Nearest;
		TextureAddress mTextureAddress = TextureAddress::Clamp;
		
		void updateGLSampler();

		void setGLVertexBuffer(const Buffer& value);
		void setGLIndexBuffer(const Buffer& value);

		std::optional<Viewport> mViewport;
		bool mViewportDirty = true;

		std::shared_ptr<RenderTarget> mCurrentRenderTarget = nullptr;

		uint32_t mBackbufferWidth = 0;
		uint32_t mBackbufferHeight = 0;
	};

	using SystemCrossplatform = SystemGL;
}
#endif
