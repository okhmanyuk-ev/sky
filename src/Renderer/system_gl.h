#pragma once

#include <Renderer/system.h>

#if defined(RENDERER_GL44) || defined(RENDERER_GLES3)

#include <Platform/system.h>
#include <Common/event_system.h>
#include <Renderer/defines.h>
#include <Renderer/vertex.h>
#include <Renderer/topology.h>
#include <Renderer/low_level_api.h>

#include <Platform/system_android.h>

namespace Renderer
{
	class SystemGL : public System
	{
	public:
		SystemGL();
		~SystemGL();

	public:
		void setTopology(const Topology& value) override;
		void setViewport(const Viewport& value) override;
		void setScissor(const Scissor& value) override;
		void setScissor(std::nullptr_t value) override;
		void setVertexBuffer(const Buffer& value) override;
		void setIndexBuffer(const Buffer& value) override;
		void setTexture(const Texture& value) override;
		void setRenderTarget(const RenderTarget& value) override;
		void setRenderTarget(std::nullptr_t value) override;
		void setShader(std::shared_ptr<Shader> value) override;
		void setSampler(const Sampler& value) override;
		void setDepthMode(const DepthMode& value) override;
		void setCullMode(const CullMode& value) override;
		void setBlendMode(const BlendMode& value) override;

		void clear(const glm::vec4& color = { 0.0f, 0.0f, 0.0f, 1.0f }) override;

		void draw(size_t vertexCount, size_t vertexOffset = 0) override;
		void drawIndexed(size_t indexCount, size_t indexOffset = 0, size_t vertexOffset = 0) override;

		void present() override;

		bool isVsync() const override;
		void setVsync(bool value) override;

	private:
		bool mVsync = false;

#if defined(RENDERER_GLES3) && defined(PLATFORM_ANDROID)
		Common::EventSystem::Listener<Platform::SystemAndroid::ResumeEvent> mResumeListener;
		Common::EventSystem::Listener<Platform::SystemAndroid::PauseEvent> mPauseListener;
		Common::EventSystem::Listener<Platform::SystemAndroid::InitWindowEvent> mInitWindowListener;
		Common::EventSystem::Listener<Platform::SystemAndroid::TerminateWindowEvent> mTerminateWindowListener;
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
		static std::map<Topology, GLenum> Topology;
		static std::map<Vertex::Attribute::Format, GLint> Size;
		static std::map<Vertex::Attribute::Format, GLenum> Type;
		static std::map<Vertex::Attribute::Format, GLboolean> Normalize;











		static bool IsRenderTargetBound();

	private:
		static bool mRenderTargetBound;




	private:
		void prepareForDrawing();

	private:
		GLenum mGLTopology;
		GLuint mGLVertexBuffer;
		
		Buffer mIndexBuffer;
		//Buffer mVertexBuffer;

		Viewport mViewport;
		Viewport mViewportState;
		Scissor mScissor;
		Scissor mScissorState;
		std::shared_ptr<Shader> mAppliedShader = nullptr;
		std::shared_ptr<Shader> mStateShader = nullptr;
		DepthMode mDepthMode = DepthMode::None;
		DepthMode mStateDepthMode = DepthMode::None;
		CullMode mCullMode = CullMode::None;
		CullMode mStateCullMode = CullMode::None;
		bool mTextureBound = false;
		Sampler mSampler = Sampler::Nearest;
		
		bool mDepthModeApplied = false;
		bool mCullModeApplied = false;
		
		void setGLDepthMode(DepthMode depthMode);
		void setGLCullMode(CullMode cullMode);
		void updateGLSampler();
	};

	using SystemCrossplatform = SystemGL;
}
#endif