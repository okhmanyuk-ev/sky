#pragma once

#include <renderer/shaders/default.h>

namespace Renderer::Shaders
{
	class MipmapBias : public Default
	{
	private:
		struct alignas(16) CustomConstantBuffer
		{
			float bias = 0.0f;
		};

	public:
		MipmapBias(const Vertex::Layout& layout);

	public:
		auto getBias() const { return mCustomConstantBuffer.bias; }
		void setBias(float value) { mCustomConstantBuffer.bias = value; }

	private:
		CustomConstantBuffer mCustomConstantBuffer;
	};
}