#pragma once

#include <glm/glm.hpp>

namespace Renderer
{
	struct Viewport
	{
		glm::vec2 position = { 0.0f, 0.0f };
		glm::vec2 size = { 0.0f, 0.0f };
		float minDepth = 0.0f;
		float maxDepth = 1.0f;
	};

	inline bool operator==(const Viewport& left, const Viewport& right)
	{
		return left.position == right.position&&
			left.size == right.size &&
			left.minDepth == right.minDepth &&
			left.maxDepth == right.maxDepth;
	}

	inline bool operator!=(const Viewport& left, const Viewport& right)
	{
		return !(left == right);
	}
}