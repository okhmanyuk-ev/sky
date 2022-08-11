#include "grayscale.h"
#include <renderer/system.h>

using namespace Renderer;
using namespace Renderer::Shaders;

static std::string src_fragment = R"(
layout(binding = 2) uniform _settings
{
	float intensity;
} settings;

vec4 fragment(vec4 result)
{
	float gray = dot(result.rgb, vec3(0.299, 0.587, 0.114));
	result.rgb = mix(result.rgb, vec3(gray), settings.intensity);
	return result;
})";

Grayscale::Grayscale(const skygfx::Vertex::Layout& layout) : Generic(layout, src_fragment)
{
}

void Grayscale::update()
{
	Generic::update();
	RENDERER->setUniformBuffer(2, mSettings);
}
