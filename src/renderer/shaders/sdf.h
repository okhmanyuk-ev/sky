#pragma once

#include <Renderer/shader_custom.h>

namespace Renderer
{
	class ShaderSdf : public ShaderCustom
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
		ShaderSdf(const Vertex::Layout& layout);
		~ShaderSdf();

	public:
		auto getMinValue() const { return mCustomConstantBuffer.minValue; }
		void setMinValue(float value) { mCustomConstantBuffer.minValue = value; markDirty(); }

		auto getMaxValue() const { return mCustomConstantBuffer.maxValue; }
		void setMaxValue(float value) { mCustomConstantBuffer.maxValue = value; markDirty(); }

		auto getSmoothFactor() const { return mCustomConstantBuffer.smoothFactor; }
		void setSmoothFactor(float value) { mCustomConstantBuffer.smoothFactor = value; markDirty(); }

		auto getColor() const { return mCustomConstantBuffer.color; }
		void setColor(const glm::vec4& value) { mCustomConstantBuffer.color = value; markDirty(); }

	private:
		CustomConstantBuffer mCustomConstantBuffer;
	};
}