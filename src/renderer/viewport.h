#pragma once

#include <glm/glm.hpp>

#include <Platform/system.h>
#include <Renderer/render_target.h>

namespace Renderer
{
	struct Viewport
	{
		Viewport(const glm::vec2& _size, float _minDepth = 0.0f, float _maxDepth = 1.0f)
		{
			position = { 0.0f, 0.0f };
			size = _size;
			minDepth = minDepth;
			maxDepth = maxDepth;
		}

		Viewport(float _minDepth = 0.0f, float _maxDepth = 1.0f) :
			Viewport(glm::vec2({ static_cast<float>(PLATFORM->getWidth()), static_cast<float>(PLATFORM->getHeight()) }), _minDepth, _maxDepth)
		{ }

		Viewport(const RenderTarget& target, float _minDepth = 0.0f, float _maxDepth = 1.0f) :
			Viewport(glm::vec2({ static_cast<float>(target.getWidth()), static_cast<float>(target.getHeight()) }), _minDepth, _maxDepth)
		{ }

		glm::vec2 position = { 0.0f, 0.0f };
		glm::vec2 size = { 0.0f, 0.0f };
		float minDepth = 0.0f;
		float maxDepth = 0.0f;
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