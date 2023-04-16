#pragma once

#include <renderer/shaders/generic.h>

namespace Renderer::Shaders
{
	class Grayscale : public Generic
	{
	private:
		struct alignas(16) Settings
		{
			float intensity = 1.0f;
		};

	public:
		Grayscale(const skygfx::VertexLayout& layout);

	protected:
		void update() override;

	public:
		auto getIntensity() const { return mSettings.intensity; }
		void setIntensity(float value) { mSettings.intensity = value; }

	private:
		Settings mSettings;
	};
}