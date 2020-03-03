#pragma once

#include <renderer/shader_custom.h>

namespace Renderer::Shaders
{
	// https://github.com/cansik/processing-bloom-filter

	class BrightFilter : public ShaderCustom
	{
	private:
		struct alignas(16) CustomConstantBuffer
		{
			float threshold = 0.99f;
		};

	public:
		BrightFilter(const Vertex::Layout& layout);
		
	public:
		auto getThreshold() const { return mCustomConstantBuffer.threshold; }
		void setThreshold(float value) { mCustomConstantBuffer.threshold = value; markDirty(); }

	private:
		CustomConstantBuffer mCustomConstantBuffer;
	};
}