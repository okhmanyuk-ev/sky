#pragma once

#include <Renderer/vertex.h>
#include <set>
#include <assert.h>

namespace Renderer
{
	class Shader
	{
		friend class SystemD3D11;
		friend class SystemGL;

	protected:
		virtual void apply() = 0;
		virtual void update() = 0;

	protected:
		static void checkRequiredAttribs(const std::set<Vertex::Attribute::Type>& requiredAttribs, const Vertex::Layout& layout);
	};
}