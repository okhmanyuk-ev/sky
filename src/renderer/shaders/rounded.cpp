#include "rounded.h"

using namespace Renderer;
using namespace Renderer::Shaders;

static std::string src_ubo = R"(
layout(binding = 2) uniform _settings
{
	vec4 color;
	vec2 size;
	float radius;
} settings;
)";

static std::string src_fragment = R"(
vec4 fragment(vec4 result)
{
	vec2 p = vec2(In.Position) * settings.size;

	if (length(p - vec2(settings.radius, settings.radius)) > settings.radius && length(p) < settings.radius)
		discard;

	if (length(p - vec2(settings.size.x - settings.radius, settings.radius)) > settings.radius && length(p - vec2(settings.size.x, 0.0)) < settings.radius)
		discard;

	if (length(p - vec2(settings.radius, settings.size.y - settings.radius)) > settings.radius && length(p - vec2(0.0, settings.size.y)) < settings.radius)
		discard;

	if (length(p - vec2(settings.size.x - settings.radius, settings.size.y - settings.radius)) > settings.radius && length(p - vec2(settings.size.x, settings.size.y)) < settings.radius)
		discard;

	return result;
}
)";

Rounded::Rounded(const Vertex::Layout& layout) : Generic(layout, src_ubo, src_fragment)
{
};

void Rounded::update()
{
	Generic::update();
	RENDERER->setUniformBuffer(2, mSettings);
}
