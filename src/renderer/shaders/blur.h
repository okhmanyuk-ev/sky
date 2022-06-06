#pragma once

#include <renderer/shader_custom.h>

namespace Renderer::Shaders
{
	class Blur : public ShaderCross, public ShaderMatrices
	{
	public:
		enum class Direction
		{
			Vertical,
			Horizontal
		};

	private:
		struct alignas(16) Settings
		{
			glm::mat4 projection = glm::mat4(1.0f);
			glm::mat4 view = glm::mat4(1.0f);
			glm::mat4 model = glm::mat4(1.0f);
			glm::vec2 direction = { 0.0f, 0.0f };
			glm::vec2 resolution = { 0.0f, 0.0f };
		};

	public:
		Blur(const Vertex::Layout& layout);

	protected:
		void update() override;

	public:
		glm::mat4 getProjectionMatrix() const override { return mSettings.projection; }
		void setProjectionMatrix(const glm::mat4& value) override { mSettings.projection = value; }

		glm::mat4 getViewMatrix() const override { return mSettings.view; }
		void setViewMatrix(const glm::mat4& value) override { mSettings.view = value; }

		glm::mat4 getModelMatrix() const override { return mSettings.model; }
		void setModelMatrix(const glm::mat4& value) override { mSettings.model = value; }

		void setDirection(Direction value) { mSettings.direction = (value == Direction::Horizontal ? glm::vec2(1.0f, 0.0f) : glm::vec2(0.0f, 1.0f)); }
		void setResolution(const glm::vec2& value) { mSettings.resolution = value; }

	private:
		Settings mSettings;
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
