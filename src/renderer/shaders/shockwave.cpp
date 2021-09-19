#include "shockwave.h"

using namespace Renderer;
using namespace Renderer::Shaders;

namespace
{
#if defined(RENDERER_GL44) || defined(RENDERER_GLES3)
	const char* srcFields =
		R"(
			float size;
			float thickness;
			float force;
		)";

	const char* srcFragment =
		R"(
		vec4 fragment(vec4 result)
		{
			const vec2 center = vec2(0.5, 0.5);
			float d = distance(vTexCoord, center);
		
			float scaled_size = size * 0.5;
			float scaled_thickness = thickness * 0.25;

			float mask = (1.0 - smoothstep(scaled_size - scaled_thickness, scaled_size, d));
			mask *= smoothstep(scaled_size - scaled_thickness - scaled_thickness, scaled_size - scaled_thickness, d);
			
			vec2 disp = (vTexCoord - center) * mask * force;

			result = texture(uTexture, vTexCoord - disp);

			return result;
		})";
#elif defined(RENDERER_D3D11)
	const char* srcFields =
		R"(
			float size;
			float thickness;
			float force;
		)";

	const char* srcFragment =
		R"(
		float4 fragment(float4 result, PixelInput input)
		{
			const float2 center = float2(0.5, 0.5);
			float d = distance(input.uv, center);
		
			float scaled_size = size * 0.5;
			float scaled_thickness = thickness * 0.25;

			float mask = (1.0 - smoothstep(scaled_size - scaled_thickness, scaled_size, d));
			mask *= smoothstep(scaled_size - scaled_thickness - scaled_thickness, scaled_size - scaled_thickness, d);
			
			float2 disp = (input.uv - center) * mask * force;

			result = texture0.Sample(sampler0, input.uv - disp);

			return result;
		})";
#endif
}

Shockwave::Shockwave(const Vertex::Layout& layout) : Default(layout, sizeof(CustomConstantBuffer), CustomCode{ srcFields, srcFragment })
{
	setCustomConstantBuffer(&mCustomConstantBuffer);
}
