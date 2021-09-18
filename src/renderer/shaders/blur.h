#pragma once

#include <renderer/shader_custom.h>

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
		struct alignas(16) ConstantBuffer
		{
			glm::vec2 direction = { 0.0f, 0.0f };
			glm::vec2 resolution = { 0.0f, 0.0f };
		};

	public:
		Blur(const Vertex::Layout& layout);

	public:
		void setDirection(Direction value) { mConstantBuffer.direction = (value == Direction::Horizontal ? glm::vec2(1.0f, 0.0f) : glm::vec2(0.0f, 1.0f)); }
		void setResolution(const glm::vec2& value) { mConstantBuffer.resolution = value; }

	private:
		ConstantBuffer mConstantBuffer;
	};

	class Blur2 : public ShaderCustom
	{
	public:
		enum class Direction
		{
			Vertical,
			Horizontal
		};

	private:
		struct alignas(16) ConstantBuffer
		{
			glm::vec2 direction = { 0.0f, 0.0f };
			glm::vec2 resolution = { 0.0f, 0.0f };
			float sigma = 1.0f;
            float iterations = 32;
		};

	public:
		Blur2(const Vertex::Layout& layout);

	public:
		void setDirection(Direction value) { mConstantBuffer.direction = (value == Direction::Horizontal ? glm::vec2(1.0f, 0.0f) : glm::vec2(0.0f, 1.0f)); }
		void setResolution(const glm::vec2& value) { mConstantBuffer.resolution = value; }

		auto getSigma() const { return mConstantBuffer.sigma; }
		void setSigma(float value) { mConstantBuffer.sigma = value; }

		auto getIterations() const { return (int)mConstantBuffer.iterations; }
		void setIterations(int value) { mConstantBuffer.iterations = (float)value; }

	private:
		ConstantBuffer mConstantBuffer;
	};
}
