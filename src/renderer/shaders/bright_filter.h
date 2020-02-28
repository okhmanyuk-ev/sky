#pragma once

#include <Renderer/shader_custom.h>

namespace Renderer
{
	// https://github.com/cansik/processing-bloom-filter

	class ShaderBrightFilter : public ShaderCustom
	{
	private:
		struct alignas(16) CustomConstantBuffer
		{
			float threshold = 0.99f;
		};

	public:
		ShaderBrightFilter(const Vertex::Layout& layout);
		~ShaderBrightFilter();

	public:
		auto getThreshold() const { return mCustomConstantBuffer.threshold; }
		void setThreshold(float value) { mCustomConstantBuffer.threshold = value; markDirty(); }

	private:
		CustomConstantBuffer mCustomConstantBuffer;
	};
}