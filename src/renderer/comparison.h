#pragma once

namespace Renderer
{
	enum class ComparisonFunc
	{
		Always,         // comparison always succeeds
		Never,          // comparison always fails
		Less,           // passes if source is less than the destination
		Equal,          // passes if source is equal to the destination
		NotEqual,       // passes if source is not equal to the destination
		LessEqual,      // passes if source is less than or equal to the destination
		Greater,        // passes if source is greater than to the destination
		GreaterEqual,   // passes if source is greater than or equal to the destination
	};
}