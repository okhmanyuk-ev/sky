#pragma once

#include <renderer/system.h>

#include <platform/system.h>
#include <common/event_system.h>
#include <common/hash.h>
#include <map>
#include <skygfx/skygfx.h>

namespace Renderer
{
	class SystemSkygfx;

	using SystemCrossplatform = SystemSkygfx;

	class SystemSkygfx : public System,
		Common::Event::Listenable<Platform::System::ResizeEvent>
	{
	public:
		SystemSkygfx();
		~SystemSkygfx();

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

		bool isVsync() const override { return mVsync; }
		void setVsync(bool value) override { mVsync = value; }
	
	private:
		bool mVsync = false;

	private:
		std::shared_ptr<skygfx::Device> mDevice;
	};
}
