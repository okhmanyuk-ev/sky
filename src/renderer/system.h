#pragma once

#include <core/engine.h>
#include <renderer/shader.h>

#include <optional>

#include <skygfx/skygfx.h>
#include <common/event_system.h>
#include <platform/all.h>

#define RENDERER ENGINE->getSystem<Renderer::System>()

namespace Renderer
{
	class System : public Common::Event::Listenable<Platform::System::ResizeEvent>
	{
	public:
		System();
		~System();

	private:
		void onEvent(const Platform::System::ResizeEvent& e) override;

	public:
		void setTopology(skygfx::Topology value);
		void setViewport(std::optional<skygfx::Viewport> value);
		void setScissor(std::optional<skygfx::Scissor> value);
		void setVertexBuffer(const skygfx::Buffer& value);
		void setIndexBuffer(const skygfx::Buffer& value);
		void setUniformBuffer(uint32_t slot, void* memory, size_t size);
		void setTexture(uint32_t binding, const skygfx::Texture& value);
		void setTexture(const skygfx::Texture& value);
		void setRenderTarget(std::shared_ptr<skygfx::RenderTarget> value);
		void setShader(std::shared_ptr<Shader> value);
		void setSampler(skygfx::Sampler value);
		void setDepthMode(std::optional<skygfx::DepthMode> value);
		void setStencilMode(std::optional<skygfx::StencilMode> value);
		void setCullMode(skygfx::CullMode value);
		void setBlendMode(const skygfx::BlendMode& value);
		void setTextureAddressMode(skygfx::TextureAddress value);

		void clear(std::optional<glm::vec4> color = glm::vec4{ 0.0f, 0.0f, 0.0f, 0.0f }, 
			std::optional<float> depth = 1.0f, std::optional<uint8_t> stencil = 0);

		void draw(size_t vertexCount, size_t vertexOffset = 0);
		void drawIndexed(size_t indexCount, size_t indexOffset = 0, size_t vertexOffset = 0);

		void readPixels(const glm::ivec2& pos, const glm::ivec2& size, std::shared_ptr<skygfx::Texture> dst_texture);

		void present();

		bool isVsync() const { return mVsync; }
		void setVsync(bool value) { mVsync = value; }

		template <class T>
		void setUniformBuffer(int slot, const T& buffer) { setUniformBuffer(slot, &const_cast<T&>(buffer), sizeof(T)); }

	public:
		int getDrawcalls() const { return mDrawcallsPublic; }

	private:
		bool mVsync = false;
		int mDrawcalls = 0;
		int mDrawcallsPublic = 0;
		std::shared_ptr<skygfx::Device> mDevice;
	};
}