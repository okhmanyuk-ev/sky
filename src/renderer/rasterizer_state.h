#pragma once

namespace Renderer
{
	enum class CullMode
	{
		None,   // No culling
		Front,  // Cull front-facing primitives
		Back,   // Cull back-facing primitives
	};
}