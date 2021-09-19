#include "grayscale.h"

using namespace Renderer;
using namespace Renderer::Shaders;

namespace
{
#if defined(RENDERER_GL44) || defined(RENDERER_GLES3)
	const char* srcFields =
		R"(
			float uIntensity;
		)";

	const char* srcFragment =
		R"(
		vec4 fragment(vec4 result)
		{
			float gray = dot(result.rgb, vec3(0.299, 0.587, 0.114));
			result.rgb = mix(result.rgb, vec3(gray), uIntensity);
			return result;
		})";
#elif defined(RENDERER_D3D11)
	const char* srcFields =
		R"(
			float intensity;
		)";

	const char* srcFragment =
		R"(
		float4 fragment(float4 result, PixelInput input)
		{
			float3 gray = dot(result.rgb, float3(0.299, 0.587, 0.114)); 
			result.rgb = lerp(result.rgb, gray, intensity);
			return result;
		})";
#endif
}

Grayscale::Grayscale(const Vertex::Layout& layout) : Default(layout, { Flag::Colored, Flag::Textured }, sizeof(CustomConstantBuffer), CustomCode{ srcFields, srcFragment })
{
	setCustomConstantBuffer(&mCustomConstantBuffer);
};