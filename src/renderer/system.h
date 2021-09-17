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

#define RENDERER ENGINE->getSystem<Renderer::System>()

namespace Renderer
{
	class System
	{
	public:
		virtual void setTopology(const Topology& value) = 0;
		virtual void setViewport(const Viewport& value) = 0;
		virtual void setScissor(const Scissor& value) = 0;
		virtual void setScissor(std::nullptr_t value) = 0;
		virtual void setVertexBuffer(const Buffer& value) = 0;
		virtual void setIndexBuffer(const Buffer& value) = 0;
		virtual void setTexture(std::shared_ptr<Texture> value) = 0;
		virtual void setRenderTarget(std::shared_ptr<RenderTarget> value) = 0;
		virtual void setShader(std::shared_ptr<Shader> value) = 0;
		virtual void setSampler(const Sampler& value) = 0;
		virtual void setDepthMode(const DepthMode& value) = 0;
		virtual void setStencilMode(const StencilMode& value) = 0;
		virtual void setCullMode(const CullMode& value) = 0;
		virtual void setBlendMode(const BlendMode& value) = 0;
		virtual void setTextureAddressMode(const TextureAddress& value) = 0;

		virtual void clear(const glm::vec4& color = { 0.0f, 0.0f, 0.0f, 0.0f }) = 0;
		virtual void clearStencil() = 0;

		virtual void draw(size_t vertexCount, size_t vertexOffset = 0);
		virtual void drawIndexed(size_t indexCount, size_t indexOffset = 0, size_t vertexOffset = 0);

		virtual void readPixels(const glm::ivec2& pos, const glm::ivec2& size, void* memory) = 0;

		virtual void present();

		virtual bool isVsync() const = 0;
		virtual void setVsync(bool value) = 0;

	public:
		int getDrawcalls() const { return mDrawcallsPublic; }

	private:
		int mDrawcalls = 0;
		int mDrawcallsPublic = 0;
	};
}