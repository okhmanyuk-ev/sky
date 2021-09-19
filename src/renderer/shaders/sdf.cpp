#include "sdf.h"

using namespace Renderer;
using namespace Renderer::Shaders;

namespace
{
#if defined(RENDERER_GL44) || defined(RENDERER_GLES3)
	const char* srcFields =
		R"(
			vec4 uColor;
			float uMinValue;
			float uMaxValue;
			float uSmoothFactor;
		)";
	
	const char* srcFragment =
		R"(
		vec4 fragment(vec4 result)
		{
			float distance = texture(uTexture, vTexCoord).a;
			float minAlpha = smoothstep(uMinValue - uSmoothFactor, uMinValue + uSmoothFactor, distance);
			float maxAlpha = smoothstep(uMaxValue + uSmoothFactor, uMaxValue - uSmoothFactor, distance);
			result = vec4(0.0, 0.0, 0.0, 0.0);
			if (maxAlpha > 0.0 && minAlpha > 0.0)
			{
				if (maxAlpha > 0.0)
				{
					result = vec4(uColor.rgb, uColor.a * maxAlpha);
				}
				if (minAlpha > 0.0 && minAlpha < maxAlpha)
				{
					result = vec4(uColor.rgb, uColor.a * minAlpha);
				}
			}
			return result;
		})";
#elif defined(RENDERER_D3D11)
	const char* srcFields =
		R"(
			float4 color;
			float minValue;
			float maxValue;
			float smoothFactor;
		)";

	const char* srcFragment = 
		R"(
		float4 fragment(float4 result, PixelInput input)
		{
			float distance = texture0.Sample(sampler0, input.uv).a;
			float minAlpha = smoothstep(minValue - smoothFactor, minValue + smoothFactor, distance);
			float maxAlpha = smoothstep(maxValue + smoothFactor, maxValue - smoothFactor, distance);
			result = float4(0.0, 0.0, 0.0, 0.0);
			if (maxAlpha > 0.0 && minAlpha > 0.0)
			{
				if (maxAlpha > 0.0)
				{
					result = float4(color.rgb, color.a * maxAlpha);
				}
				if (minAlpha > 0.0 && minAlpha < maxAlpha)
				{
					result = float4(color.rgb, color.a * minAlpha);
				}
			}
			return result;
		})";
#endif
}

Sdf::Sdf(const Vertex::Layout& layout) : Default(layout, { Flag::Colored, Flag::Textured }, sizeof(CustomConstantBuffer), CustomCode{ srcFields, srcFragment })
{
	setCustomConstantBuffer(&mCustomConstantBuffer);
}