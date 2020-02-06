#pragma once

#include <glm/glm.hpp>

#include <Platform/system.h>
#include <Renderer/render_target.h>

namespace Renderer
{
	struct Viewport
	{
		glm::vec2 position = { 0.0f, 0.0f };
		glm::vec2 size = { 0.0f, 0.0f };
		float minDepth = 0.0f;
		float maxDepth = 0.0f;

		static Viewport FullScreen(float minDepth = 0.0f, float maxDepth = 1.0f)
		{
			Viewport viewport;
			viewport.position = { 0.0f, 0.0f };
			viewport.size = { static_cast<float>(PLATFORM->getWidth()), static_cast<float>(PLATFORM->getHeight()) };
			viewport.minDepth = minDepth;
			viewport.maxDepth = maxDepth;
			return viewport;
		}

		static Viewport FullRenderTarget(const RenderTarget& target, float minDepth = 0.0f, float maxDepth = 1.0f) 
		{
			Viewport viewport;
			viewport.position = { 0.0f, 0.0f };
			viewport.size = { static_cast<float>(target.getWidth()), static_cast<float>(target.getHeight()) };
			viewport.minDepth = minDepth;
			viewport.maxDepth = maxDepth;
			return viewport;
		}
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