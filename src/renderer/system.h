#pragma once

#include <core/engine.h>
#include <renderer/viewport.h>
#include <renderer/scissor.h>
#include <renderer/buffer.h>
#include <renderer/texture.h>
#include <renderer/render_target.h>
#include <renderer/shader.h>
#include <renderer/sampler.h>
#include <renderer/depth.h>
#include <renderer/rasterizer_state.h>
#include <renderer/blend.h>
#include <renderer/stencil.h>
#include <renderer/texture_address.h>

#include <optional>

#include <skygfx/skygfx.h>
#include <common/event_system.h>

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
		void setViewport(std::optional<Viewport> value);
		void setScissor(std::optional<Scissor> value);
		void setVertexBuffer(const Buffer& value);
		void setIndexBuffer(const Buffer& value);
		void setUniformBuffer(int slot, void* memory, size_t size);
		void setTexture(int binding, std::shared_ptr<Texture> value);
		void setTexture(std::shared_ptr<Texture> value);
		void setRenderTarget(std::shared_ptr<RenderTarget> value);
		void setShader(std::shared_ptr<Shader> value);
		void setSampler(const Sampler& value);
		void setDepthMode(const DepthMode& value);
		void setStencilMode(const StencilMode& value);
		void setCullMode(const CullMode& value);
		void setBlendMode(const BlendMode& value);
		void setTextureAddressMode(const TextureAddress& value);

		void clear(std::optional<glm::vec4> color = glm::vec4{ 0.0f, 0.0f, 0.0f, 0.0f }, 
			std::optional<float> depth = 1.0f, std::optional<uint8_t> stencil = 0);

		void draw(size_t vertexCount, size_t vertexOffset = 0);
		void drawIndexed(size_t indexCount, size_t indexOffset = 0, size_t vertexOffset = 0);

		void readPixels(const glm::ivec2& pos, const glm::ivec2& size, std::shared_ptr<Renderer::Texture> dst_texture);

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