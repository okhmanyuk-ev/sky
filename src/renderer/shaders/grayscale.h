#pragma once

#include <renderer/shaders/default.h>

namespace Renderer::Shaders
{
	class Grayscale : public Default
	{
	private:
		struct alignas(16) CustomConstantBuffer
		{
			float intensity = 1.0f;
		};

	public:
		Grayscale(const Vertex::Layout& layout);

	public:
		auto getIntensity() const { return mCustomConstantBuffer.intensity; }
		void setIntensity(float value) { mCustomConstantBuffer.intensity = value; }

	private:
		CustomConstantBuffer mCustomConstantBuffer;
	};
}