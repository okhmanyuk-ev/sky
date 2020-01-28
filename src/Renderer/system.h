#pragma once

#include <Core/engine.h>
#include <Renderer/defines.h>
#include <Renderer/topology.h>
#include <Renderer/viewport.h>
#include <Renderer/scissor.h>
#include <Renderer/buffer.h>
#include <Renderer/texture.h>
#include <Renderer/render_target.h>
#include <Renderer/shader.h>
#include <Renderer/sampler.h>
#include <Renderer/depth_mode.h>
#include <Renderer/rasterizer_state.h>
#include <Renderer/blend.h>

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
		virtual void setTexture(const Texture& value) = 0;
		virtual void setRenderTarget(const RenderTarget& value) = 0;
		virtual void setRenderTarget(std::nullptr_t value) = 0;
		virtual void setShader(std::shared_ptr<Shader> value) = 0;
		virtual void setSampler(const Sampler& value) = 0;
		virtual void setDepthMode(const DepthMode& value) = 0;
		virtual void setCullMode(const CullMode& value) = 0;
		virtual void setBlendMode(const BlendMode& value) = 0;

		virtual void clear(const glm::vec4& color = { 0.0f, 0.0f, 0.0f, 0.0f }) = 0;

		virtual void draw(size_t vertexCount, size_t vertexOffset = 0) = 0;
		virtual void drawIndexed(size_t indexCount, size_t indexOffset = 0, size_t vertexOffset = 0) = 0;

		virtual void present() = 0;

		virtual bool isVsync() const = 0;
		virtual void setVsync(bool value) = 0;
	};
}