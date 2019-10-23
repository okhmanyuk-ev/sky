#pragma once

#include <Core/engine.h>
#include <Renderer/system.h>
#include <Common/event_system.h>

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
		void draw(size_t vertexCount, size_t vertexOffset = 0) override 
		{
			T::draw(vertexCount, vertexOffset);
			
			auto drawcall = RendererDebugDrawCallEvent();
			drawcall.indexed = false;
			drawcall.vertexCount = vertexCount;
			drawcall.vertexOffset = vertexOffset;
		//	drawcall.indexCount = indexCount;
		//	drawcall.indexOffset = indexOffset;
			
			EVENT->emit(drawcall);
		}

		void drawIndexed(size_t indexCount, size_t indexOffset = 0, size_t vertexOffset = 0) override
		{
			T::drawIndexed(indexCount, indexOffset, vertexOffset);
			
			auto drawcall = RendererDebugDrawCallEvent();
			drawcall.indexed = true;
		//	drawcall.vertexCount = vertexCount;
			drawcall.vertexOffset = vertexOffset;
			drawcall.indexCount = indexCount;
			drawcall.indexOffset = indexOffset;

			EVENT->emit(drawcall);
		}
	};
}