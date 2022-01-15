#pragma once

#include <glm/glm.hpp>

#include <platform/system.h>
#include <renderer/render_target.h>

namespace Renderer
{
	struct Viewport
	{
		Viewport(const glm::vec2& _pos, const glm::vec2& _size, float _minDepth = 0.0f, float _maxDepth = 1.0f) :
			position(_pos),
			size(_size),
			minDepth(_minDepth),
			maxDepth(_maxDepth)
		{ }

		Viewport(const glm::vec2& _size, float _minDepth = 0.0f, float _maxDepth = 1.0f) :
			Viewport({ 0.0f, 0.0f }, _size, _minDepth, _maxDepth)
		{ }

		Viewport(std::shared_ptr<RenderTarget> target = nullptr, float _minDepth = 0.0f, float _maxDepth = 1.0f) :
			Viewport(target 
				? glm::vec2({ static_cast<float>(target->getWidth()), static_cast<float>(target->getHeight()) }) 
				: glm::vec2({ static_cast<float>(PLATFORM->getWidth()), static_cast<float>(PLATFORM->getHeight()) }), 
				_minDepth, _maxDepth)
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