#include "circle.h"
#include <renderer/system.h>

using namespace Renderer;
using namespace Renderer::Shaders;

static std::string src_ubo = R"(
layout(binding = 2) uniform _settings
{
	vec4 color;
	vec4 inner_color;
	vec4 outer_color;
	float fill;
	float pie;
} settings;
)";

static std::string src_fragment = R"(
vec4 fragment(vec4 result)
{
	const float Pi = 3.14159265;

	vec2 center = vec2(0.5, 0.5);

	vec2 p = In.Position.xy - center;
	float angle = atan(-p.x, p.y);
	float normalized_angle = (angle + Pi) / 2.0 / Pi;			

	if (normalized_angle > settings.pie)
		discard;

	float maxRadius = 0.5;
	float minRadius = maxRadius * (1.0f - settings.fill);
			
	float radius = distance(In.Position.xy, center);

	if (radius > maxRadius || radius < minRadius)
	{
		return vec4(0.0); // TODO: discard not working here on D3D11 (discard should be)
	}

	float t = (radius - minRadius) / (maxRadius - minRadius);
	result *= mix(settings.inner_color, settings.outer_color, t);

	result *= settings.color;
	return result;
}
)";

Circle::Circle(const Vertex::Layout& layout) : Generic(layout, src_ubo, src_fragment)
{
};

void Circle::update()
{
	Generic::update();
	RENDERER->setUniformBuffer(2, mSettings);
}
