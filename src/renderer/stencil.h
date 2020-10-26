#pragma once

#include <cstdint>
#include "comparison.h"

namespace Renderer
{
	enum class StencilOp
	{
		Keep, // Does not update the stencil buffer entry.
		Zero, // Sets the stencil buffer entry to 0.
		Replace, // Replaces the stencil buffer entry with a reference value.
		Increment, // Increments the stencil buffer entry, wrapping to 0 if the new value exceeds the maximum value.
		Decrement, // Decrements the stencil buffer entry, wrapping to the maximum value if the new value is less than 0.
		IncrementSaturation, // Increments the stencil buffer entry, clamping to the maximum value.
		DecrementSaturation, // Decrements the stencil buffer entry, clamping to 0.
		Invert // Inverts the bits in the stencil buffer entry.
	};

	struct StencilMode
	{
		StencilMode() { }

		bool enabled = false;

		uint8_t readMask = 255;
		uint8_t writeMask = 255;

		StencilOp depthFailOp = StencilOp::Keep;
		StencilOp failOp = StencilOp::Keep;
		ComparisonFunc func = ComparisonFunc::Always;
		StencilOp passOp = StencilOp::Keep;

		uint8_t reference = 1;
	};

	inline bool operator==(const StencilMode& left, const StencilMode& right)
	{
		return
			left.enabled == right.enabled &&

			left.readMask == right.readMask &&
			left.writeMask == right.writeMask &&

			left.depthFailOp == right.failOp &&
			left.failOp == right.failOp &&
			left.func == right.func &&
			left.passOp == right.passOp &&
			
			left.reference == right.reference;
	}

	inline bool operator!=(const StencilMode& left, const StencilMode& right) 
	{
		return !(left == right);
	}

	namespace StencilStates
	{
		inline const StencilMode Disabled = StencilMode();
	}
}