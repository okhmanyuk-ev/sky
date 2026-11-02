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

const std::string LiquidGlass::Effect = R"(
layout(binding = EFFECT_UNIFORM_BINDING) uniform _liquid_glass
{
	vec4 tint;
	vec2 size;
	float radius;
	float bezel;
	float refraction;
	float chromatic;
	float highlight;
	float saturation;
} liquid_glass;

// signed distance to a rounded box in x, its outward normal in yz
vec3 RoundedBoxSdf(vec2 p, vec2 half_size, float radius)
{
	vec2 w = abs(p) - half_size + radius;
	float g = max(w.x, w.y);
	vec2 q = max(w, 0.0);
	float l = length(q);
	vec2 normal = sign(p) * (g > 0.0 ? q / l : (w.x > w.y ? vec2(1.0, 0.0) : vec2(0.0, 1.0)));
	return vec3((g > 0.0 ? l : g) - radius, normal);
}

void effect(inout vec4 result)
{
	vec2 local_pos = (inverse(settings.model) * vec4(In.world_position, 1.0)).xy;
	vec2 half_size = liquid_glass.size * 0.5;
	vec2 p = local_pos * liquid_glass.size - half_size;

	float d = RoundedBoxSdf(p, half_size, liquid_glass.radius).x;

	// bezel is shaped by a box rounded at least by bezel width, otherwise sharp corners
	// produce diagonal seams where the normal flips between horizontal and vertical edges
	float bezel_radius = min(max(liquid_glass.radius, liquid_glass.bezel), min(half_size.x, half_size.y));
	vec3 bezel = RoundedBoxSdf(p, half_size, bezel_radius);
	vec2 normal = bezel.yz;

	// 1 on the edge, 0 where the bezel ends and the flat part of the glass begins
	float edge = 1.0 - clamp(-bezel.x / max(liquid_glass.bezel, 0.0001), 0.0, 1.0);
	float lens = edge * edge;

	// bezel acts like a convex lens: pull pixels from the inner area towards the edge
	vec2 texel = 1.0 / vec2(textureSize(sColorTexture, 0));
	vec2 offset = normal * lens * liquid_glass.refraction * liquid_glass.bezel * texel;

	float aberration = 1.0 + liquid_glass.chromatic;

	vec3 color;
	color.r = texture(sColorTexture, In.tex_coord - offset * aberration).r;
	color.g = texture(sColorTexture, In.tex_coord - offset).g;
	color.b = texture(sColorTexture, In.tex_coord - offset / aberration).b;

	float luma = dot(color, vec3(0.299, 0.587, 0.114));
	color = mix(vec3(luma), color, liquid_glass.saturation);
	color = mix(color, liquid_glass.tint.rgb, liquid_glass.tint.a);

	// specular from top-left light and weaker reflection on the opposite side
	const vec2 light_dir = vec2(-0.70710678, -0.70710678);
	float light = dot(normal, light_dir);
	float specular = pow(max(light, 0.0), 3.0) + pow(max(-light, 0.0), 3.0) * 0.4;
	float rim = 1.0 - smoothstep(0.0, 1.5, -d);
	color += (specular * lens + rim * 0.5) * liquid_glass.highlight;

	float alpha = clamp(0.5 - d, 0.0, 1.0);
	vec4 basic_color = In.color * settings.color;
	result = vec4(clamp(color, 0.0, 1.0) * basic_color.rgb, alpha * basic_color.a);
})";
