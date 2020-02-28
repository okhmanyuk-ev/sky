#pragma once

#include "comparison.h"

namespace Renderer
{
	enum class TextureAddress
	{
		Wrap, // Texels outside range will form the tile at every integer junction.		
		Clamp, // Texels outside range will be set to color of 0.0 or 1.0 texel.
	};
}