#include "circle.h"

using namespace Renderer;
using namespace Renderer::Shaders;

namespace
{
#if defined(RENDERER_GL44) || defined(RENDERER_GLES3) || defined(RENDERER_VK)
	const char* srcFields =
		R"(
			vec4 color;
			vec4 inner_color;
			vec4 outer_color;
			float fill;
			float pie;
		)";

	const char* srcFragment =
		R"(
		vec4 fragment(vec4 result)
		{
			const float Pi = 3.14159265;

			vec2 center = vec2(0.5, 0.5);

			vec2 p = vPosition.xy - center;
			float angle = atan(-p.x, p.y);
			float normalized_angle = (angle + Pi) / 2.0 / Pi;			

			if (normalized_angle > pie)
				discard;

			float maxRadius = 0.5;
			float minRadius = maxRadius * (1.0f - fill);
			
			float radius = distance(vPosition.xy, center);

			if (radius > maxRadius || radius < minRadius)
				discard;

			float t = (radius - minRadius) / (maxRadius - minRadius);
			result *= mix(inner_color, outer_color, t);

			result *= color;
			return result;

		})";
#elif defined(RENDERER_D3D11)
	const char* srcFields =
		R"(
			float4 color;
			float4 inner_color;
			float4 outer_color;
			float fill;
			float pie;
		)";

	const char* srcFragment =
		R"(
		float4 fragment(float4 result, PixelInput input)
		{
			static const float Pi = 3.14159265;

			float2 center = float2(0.5, 0.5);

			float2 p = input.pos - center;
			float angle = atan2(-p.x, p.y);
			float normalized_angle = (angle + Pi) / 2.0 / Pi;			

			if (normalized_angle > pie)
				discard;

			float maxRadius = 0.5;
			float minRadius = maxRadius * (1.0f - fill);
			
			float radius = distance(input.pos, center);

			if (radius > maxRadius || radius < minRadius)
				discard;

			float t = (radius - minRadius) / (maxRadius - minRadius);
			result *= lerp(inner_color, outer_color, t);
			result *= color;
			return result;
		})";
#endif
}

Circle::Circle(const Vertex::Layout& layout) : Default(layout, sizeof(CustomConstantBuffer), CustomCode{ srcFields, srcFragment })
{
	setCustomConstantBuffer(&mCustomConstantBuffer);
};
