#pragma once

#include <renderer/shader_custom.h>

namespace Renderer::Shaders
{
	class Sdf : public ShaderCustom
	{
	private:
		struct alignas(16) CustomConstantBuffer
		{
			glm::vec4 color = { 1.0f, 1.0f, 1.0f, 1.0f };
			float minValue = 0.0f;
			float maxValue = 0.0f;
			float smoothFactor = 0.0f;
		};

	public:
		Sdf(const Vertex::Layout& layout);

	public:
		auto getMinValue() const { return mCustomConstantBuffer.minValue; }
		void setMinValue(float value) { mCustomConstantBuffer.minValue = value; }

		auto getMaxValue() const { return mCustomConstantBuffer.maxValue; }
		void setMaxValue(float value) { mCustomConstantBuffer.maxValue = value; }

		auto getSmoothFactor() const { return mCustomConstantBuffer.smoothFactor; }
		void setSmoothFactor(float value) { mCustomConstantBuffer.smoothFactor = value; }

		auto getColor() const { return mCustomConstantBuffer.color; }
		void setColor(const glm::vec4& value) { mCustomConstantBuffer.color = value; }

	private:
		CustomConstantBuffer mCustomConstantBuffer;
	};
}