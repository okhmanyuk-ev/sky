#pragma once

#include <renderer/shaders/generic.h>

namespace Renderer::Shaders
{
	class Sdf : public Generic
	{
	private:
		struct alignas(16) Settings
		{
			glm::vec4 color = { 1.0f, 1.0f, 1.0f, 1.0f };
			float minValue = 0.0f;
			float maxValue = 0.0f;
			float smoothFactor = 0.0f;
		};

	public:
		Sdf(const skygfx::Vertex::Layout& layout);

	protected:
		void update() override;

	public:
		auto getMinValue() const { return mSettings.minValue; }
		void setMinValue(float value) { mSettings.minValue = value; }

		auto getMaxValue() const { return mSettings.maxValue; }
		void setMaxValue(float value) { mSettings.maxValue = value; }

		auto getSmoothFactor() const { return mSettings.smoothFactor; }
		void setSmoothFactor(float value) { mSettings.smoothFactor = value; }

		auto getColor() const { return mSettings.color; }
		void setColor(const glm::vec4& value) { mSettings.color = value; }

	private:
		Settings mSettings;
	};
}