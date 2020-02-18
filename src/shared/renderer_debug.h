#pragma once

#include <Core/engine.h>
#include <Renderer/system.h>
#include <Common/event_system.h>
#include <Common/profiler_system.h>

namespace Shared
{
	struct RendererDebugDrawCallEvent
	{
		bool indexed;
		size_t vertexCount;
		size_t vertexOffset;
		size_t indexCount;
		size_t indexOffset;
	};

	template <typename T> class RendererDebug : public T
	{
		static_assert(std::is_base_of<Renderer::System, T>::value, "T must be derived from Renderer::System");
	public:
		void setTopology(const Renderer::Topology& value) override
		{
			PROFILER->begin("Renderer::SetTopology");
			T::setTopology(value);
			PROFILER->end();
		}

		void setViewport(const Renderer::Viewport& value) override
		{
			PROFILER->begin("Renderer::SetViewport");
			T::setViewport(value);
			PROFILER->end();
		}

		void setScissor(const Renderer::Scissor& value) override
		{
			PROFILER->begin("Renderer::SetScissor");
			T::setScissor(value);
			PROFILER->end();
		}

		void setScissor(std::nullptr_t value) override
		{
			PROFILER->begin("Renderer::SetScissor");
			T::setScissor(value);
			PROFILER->end();
		}

		void setVertexBuffer(const Renderer::Buffer& value) override
		{
			PROFILER->begin("Renderer::SetVertexBuffer");
			T::setVertexBuffer(value);
			PROFILER->end();
		}

		void setIndexBuffer(const Renderer::Buffer& value) override
		{
			PROFILER->begin("Renderer::SetIndexBuffer");
			T::setIndexBuffer(value);
			PROFILER->end();
		}

		void setTexture(std::shared_ptr<Renderer::Texture> value) override
		{
			PROFILER->begin("Renderer::SetTexture");
			T::setTexture(value);
			PROFILER->end();
		}

		void setRenderTarget(std::shared_ptr<Renderer::RenderTarget> value) override
		{
			PROFILER->begin("Renderer::SetRenderTarget");
			T::setRenderTarget(value);
			PROFILER->end();
		}

		void setShader(std::shared_ptr<Renderer::Shader> value) override
		{
			PROFILER->begin("Renderer::SetShader");
			T::setShader(value);
			PROFILER->end();
		}

		void setSampler(const Renderer::Sampler& value) override
		{
			PROFILER->begin("Renderer::SetSampler");
			T::setSampler(value);
			PROFILER->end();
		}

		void setDepthMode(const Renderer::DepthMode& value) override
		{
			PROFILER->begin("Renderer::SetDepthMode");
			T::setDepthMode(value);
			PROFILER->end();
		}

		void setStencilMode(const Renderer::StencilMode& value) override
		{
			PROFILER->begin("Renderer::SetStencilMode");
			T::setStencilMode(value);
			PROFILER->end();
		}

		void setCullMode(const Renderer::CullMode& value) override
		{
			PROFILER->begin("Renderer::SetCullMode");
			T::setCullMode(value);
			PROFILER->end();
		}

		void setBlendMode(const Renderer::BlendMode& value) override
		{
			PROFILER->begin("Renderer::SetBlendMode");
			T::setBlendMode(value);
			PROFILER->end();
		}

		void clear(const glm::vec4& color = { 0.0f, 0.0f, 0.0f, 0.0f }) override
		{
			PROFILER->begin("Renderer::Clear");
			T::clear(color);
			PROFILER->end();
		}

		void draw(size_t vertexCount, size_t vertexOffset = 0) override
		{
			PROFILER->begin("Renderer::Draw");
			T::draw(vertexCount, vertexOffset);
			PROFILER->end();

			auto drawcall = RendererDebugDrawCallEvent();
			drawcall.indexed = false;
			drawcall.vertexCount = vertexCount;
			drawcall.vertexOffset = vertexOffset;

			EVENT->emit(drawcall);
		}

		void drawIndexed(size_t indexCount, size_t indexOffset = 0, size_t vertexOffset = 0) override
		{
			PROFILER->begin("Renderer::DrawIndexed");
			T::drawIndexed(indexCount, indexOffset, vertexOffset);
			PROFILER->end();

			auto drawcall = RendererDebugDrawCallEvent();
			drawcall.indexed = true;
			drawcall.vertexOffset = vertexOffset;
			drawcall.indexCount = indexCount;
			drawcall.indexOffset = indexOffset;

			EVENT->emit(drawcall);
		}

		void present() override
		{
			PROFILER->begin("Renderer::Present");
			T::present();
			PROFILER->end();
		}
	};
}