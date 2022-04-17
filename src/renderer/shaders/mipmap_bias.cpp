#include "mipmap_bias.h"

using namespace Renderer;
using namespace Renderer::Shaders;

namespace
{
#if defined(RENDERER_GL44) || defined(RENDERER_GLES3) || defined(RENDERER_VK)
	const char* srcFields =
		R"(
			float uBias;
		)";

	const char* srcFragment =
		R"(
		vec4 fragment(vec4 result)
		{
			result = texture(uTexture, vTexCoord, uBias);
			return result;
		})";
#elif defined(RENDERER_D3D11)
	const char* srcFields =
		R"(
			float bias;
		)";

	const char* srcFragment =
		R"(
		float4 fragment(float4 result, PixelInput input)
		{
			result = texture0.SampleBias(sampler0, input.uv, bias);
			return result;
		})";
#endif
}

MipmapBias::MipmapBias(const Vertex::Layout& layout) : Default(layout, sizeof(CustomConstantBuffer), CustomCode{ srcFields, srcFragment })
{
	setCustomConstantBuffer(&mCustomConstantBuffer);
}