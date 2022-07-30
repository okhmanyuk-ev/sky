#pragma once

#include <core/engine.h>
#include <renderer/defines.h>
#include <renderer/topology.h>
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

#define RENDERER ENGINE->getSystem<Renderer::System>()

namespace Renderer
{
	class System
	{
	public:
		virtual void setTopology(const Topology& value) = 0; // TODO: remove const
		virtual void setViewport(std::optional<Viewport> value) = 0;
		virtual void setScissor(std::optional<Scissor> value) = 0;
		virtual void setVertexBuffer(const Buffer& value) = 0;
		virtual void setIndexBuffer(const Buffer& value) = 0;
		virtual void setUniformBuffer(int slot, void* memory, size_t size) = 0;
		virtual void setTexture(int binding, std::shared_ptr<Texture> value) = 0;
		virtual void setTexture(std::shared_ptr<Texture> value) = 0;
		virtual void setRenderTarget(std::shared_ptr<RenderTarget> value) = 0;
		virtual void setShader(std::shared_ptr<Shader> value) = 0;
		virtual void setSampler(const Sampler& value) = 0;
		virtual void setDepthMode(const DepthMode& value) = 0;
		virtual void setStencilMode(const StencilMode& value) = 0;
		virtual void setCullMode(const CullMode& value) = 0;
		virtual void setBlendMode(const BlendMode& value) = 0;
		virtual void setTextureAddressMode(const TextureAddress& value) = 0;

		virtual void clear(std::optional<glm::vec4> color = glm::vec4{ 0.0f, 0.0f, 0.0f, 0.0f }, 
			std::optional<float> depth = 1.0f, std::optional<uint8_t> stencil = 0) = 0;

		virtual void draw(size_t vertexCount, size_t vertexOffset = 0);
		virtual void drawIndexed(size_t indexCount, size_t indexOffset = 0, size_t vertexOffset = 0);

		virtual void readPixels(const glm::ivec2& pos, const glm::ivec2& size, std::shared_ptr<Renderer::Texture> dst_texture) = 0;

		virtual void present();

		virtual bool isVsync() const = 0;
		virtual void setVsync(bool value) = 0;

		template <class T>
		void setUniformBuffer(int slot, const T& buffer) { setUniformBuffer(slot, &const_cast<T&>(buffer), sizeof(T)); }

	public:
		int getDrawcalls() const { return mDrawcallsPublic; }

	private:
		int mDrawcalls = 0;
		int mDrawcallsPublic = 0;
	};
}