#pragma once

#include <Renderer/shader_custom.h>

namespace Renderer::Shaders
{
	class Blur : public ShaderCustom
	{
	public:
		enum class Direction
		{
			Vertical,
			Horizontal
		};

	private:
		struct alignas(16) CustomConstantBuffer
		{
			glm::vec2 direction = { 0.0f, 0.0f };
			glm::vec2 resolution = { 0.0f, 0.0f };
		};

	public:
		Blur(const Vertex::Layout& layout);

	public:
		void setDirection(Direction value) { mCustomConstantBuffer.direction = (value == Direction::Horizontal ? glm::vec2(1.0f, 0.0f) : glm::vec2(0.0f, 1.0f)); markDirty(); }
		void setResolution(const glm::vec2& value) { mCustomConstantBuffer.resolution = value; markDirty(); }

	private:
		CustomConstantBuffer mCustomConstantBuffer;
	};
}