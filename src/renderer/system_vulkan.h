#pragma once

#include <renderer/system.h>

#if defined(RENDERER_VK)
#include <platform/system.h>
#include <common/event_system.h>
#include <common/hash.h>
#include <renderer/low_level_api.h>
#include <map>

namespace Renderer
{
	class SystemVK;
	
	using SystemCrossplatform = SystemVK;

	class SystemVK : public System,
		Common::Event::Listenable<Platform::System::ResizeEvent>
	{
	public:
		SystemVK();
		~SystemVK();

	private:
		void createSwapchain();

	private:
		void onEvent(const Platform::System::ResizeEvent& e) override;

	public:
		void setTopology(const Topology& value) override;
		void setViewport(const Viewport& value) override;
		void setScissor(const Scissor& value) override;
		void setScissor(std::nullptr_t value) override;
		void setVertexBuffer(const Buffer& value) override;
		void setIndexBuffer(const Buffer& value) override;
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

		void readPixels(const glm::ivec2& pos, const glm::ivec2& size, void* memory) override;

		void present() override;

		bool isVsync() const override { return false; }
		void setVsync(bool value) override { }
	
	protected:
		Texture::Handler createTexture(int width, int height, bool mipmap) override;
		void destroyTexture(Texture::Handler value) override;
		void textureWritePixels(Texture::Handler texture, int width, int height, int channels, void* data) override;

		RenderTarget::RenderTargetHandler createRenderTarget(Texture::Handler texture) override;
		void destroyRenderTarget(RenderTarget::RenderTargetHandler value) override;
	};
}
#endif