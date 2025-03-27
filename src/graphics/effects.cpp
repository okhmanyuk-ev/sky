#include "effects.h"

using namespace sky::effects;

const std::string Sdf::Effect = R"(
layout(binding = EFFECT_UNIFORM_BINDING) uniform _sdf
{
	vec4 color;
	float min_value;
	float max_value;
	float smooth_factor;
} sdf;

void effect(inout vec4 result)
{
	float distance = texture(sColorTexture, In.tex_coord, settings.mipmap_bias).a;
	float smoothing = fwidth(distance) * sdf.smooth_factor;

	float min_alpha = smoothstep(sdf.min_value - smoothing, sdf.min_value + smoothing, distance);
	float max_alpha = 1.0 - smoothstep(sdf.max_value - smoothing, sdf.max_value + smoothing, distance);

	float alpha = min(min_alpha, max_alpha);
	vec4 color = In.color * sdf.color;
	result = vec4(color.rgb, color.a * alpha);
})";

const std::string Circle::Effect = R"(
layout(binding = EFFECT_UNIFORM_BINDING) uniform _circle
{
	vec4 color;
	vec4 inner_color;
	vec4 outer_color;
	float fill;
	float pie;
} circle;

void effect(inout vec4 result)
{
	result = GetBasicResult();

	const float Pi = 3.14159265;

	vec2 vertex_pos = In.world_position.xy;
	vec2 center = vec2(0.5, 0.5);

	vec2 p = vertex_pos - center;
	float angle = atan(-p.x, p.y);
	float normalized_angle = (angle + Pi) / 2.0 / Pi;

	if (normalized_angle > circle.pie)
	{
		discard;
	}
	else // early returns via discard are not working in d3d11
	{
		float max_radius = 0.5;
		float min_radius = max_radius * (1.0f - circle.fill);

		float radius = distance(vertex_pos, center);

		if (radius > max_radius || radius < min_radius)
		{
			discard;
		}
		else
		{
			float t = (radius - min_radius) / (max_radius - min_radius);
			result *= mix(circle.inner_color, circle.outer_color, t);
			result *= circle.color;
		}
	}
})";

const std::string Rounded::Effect = R"(
layout(binding = EFFECT_UNIFORM_BINDING) uniform _rounded
{
	vec4 color;
	vec2 size;
	float radius;
} rounded;

void effect(inout vec4 result)
{
	result = GetBasicResult();

	vec2 vertex_pos = (inverse(settings.model) * vec4(In.world_position, 1.0)).xy;

	vec2 p = vertex_pos * rounded.size;

	if (length(p - vec2(rounded.radius, rounded.radius)) > rounded.radius && length(p) < rounded.radius)
		discard;

	if (length(p - vec2(rounded.size.x - rounded.radius, rounded.radius)) > rounded.radius && length(p - vec2(rounded.size.x, 0.0)) < rounded.radius)
		discard;

	if (length(p - vec2(rounded.radius, rounded.size.y - rounded.radius)) > rounded.radius && length(p - vec2(0.0, rounded.size.y)) < rounded.radius)
		discard;

	if (length(p - vec2(rounded.size.x - rounded.radius, rounded.size.y - rounded.radius)) > rounded.radius && length(p - vec2(rounded.size.x, rounded.size.y)) < rounded.radius)
		discard;
})";

const std::string Shockwave::Effect = R"(
layout(binding = EFFECT_UNIFORM_BINDING) uniform _shockwave
{
	float size;
	float thickness;
	float force;
} shockwave;

void effect(inout vec4 result)
{
	const vec2 center = vec2(0.5, 0.5);
	float d = distance(In.tex_coord, center);

	float scaled_size = shockwave.size * 0.5;
	float scaled_thickness = shockwave.thickness * 0.25;

	float mask = (1.0 - smoothstep(scaled_size - scaled_thickness, scaled_size, d));
	mask *= smoothstep(scaled_size - scaled_thickness - scaled_thickness, scaled_size - scaled_thickness, d);

	vec2 disp = (In.tex_coord - center) * mask * shockwave.force;

	result = texture(sColorTexture, In.tex_coord - disp);
})";
