#pragma once

#include "comparison.h"

namespace Renderer
{
	struct DepthMode
	{
		DepthMode() { };

		DepthMode(ComparisonFunc _func) : DepthMode()
		{
			enabled = true;
			func = _func;
		}

		bool enabled = false;
		ComparisonFunc func = ComparisonFunc::Always;
	};

	inline bool operator==(const DepthMode& left, const DepthMode& right)
	{
		return
			left.enabled == right.enabled &&
			left.func == right.func;
	}

	inline bool operator!=(const DepthMode& left, const DepthMode& right)
	{
		return !(left == right);
	}
}