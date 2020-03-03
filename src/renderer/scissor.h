#pragma once

#include <glm/glm.hpp>

#include <platform/system.h>
#include <renderer/render_target.h>

namespace Renderer
{
	struct Scissor
	{
		glm::vec2 position = { 0.0f, 0.0f };
		glm::vec2 size = { 0.0f, 0.0f };
	};

	inline bool operator==(const Scissor& left, const Scissor& right)
	{
		return left.position == right.position &&
			left.size == right.size;
	}

	inline bool operator!=(const Scissor& left, const Scissor& right)
	{
		return !(left == right);
	}
}