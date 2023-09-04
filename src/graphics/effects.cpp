#include "effects.h"

const std::string sky::effects::Sdf::Shader = R"(
layout(binding = EFFECT_UNIFORM_BINDING) uniform _grayscale
{
	vec4 color;
	float min_value;
	float max_value;
	float smooth_factor;
} sdf;

void effect(inout vec4 result)
{
	float distance = result.a;
	float min_alpha = smoothstep(sdf.min_value - sdf.smooth_factor, sdf.min_value + sdf.smooth_factor, distance);
	float max_alpha = smoothstep(sdf.max_value + sdf.smooth_factor, sdf.max_value - sdf.smooth_factor, distance);
	result = vec4(0.0, 0.0, 0.0, 0.0);
	if (max_alpha > 0.0 && min_alpha > 0.0)
	{
		if (max_alpha > 0.0)
		{
			result = vec4(sdf.color.rgb, sdf.color.a * max_alpha);
		}
		if (min_alpha > 0.0 && min_alpha < max_alpha)
		{
			result = vec4(sdf.color.rgb, sdf.color.a * min_alpha);
		}
	}
})";
