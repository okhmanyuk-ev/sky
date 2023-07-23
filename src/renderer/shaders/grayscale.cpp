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

Grayscale::Grayscale(const skygfx::VertexLayout& layout) : Generic(layout, src_fragment)
{
}

void Grayscale::setupUniforms()
{
	Generic::setupUniforms();
	skygfx::SetUniformBuffer(2, mSettings);
}
