#include "bright_filter.h"
#include <renderer/system.h>

using namespace Renderer;
using namespace Renderer::Shaders;

static std::string src_vertex = R"(
#version 450

layout(binding = 1) uniform constants
{
	mat4 view;
	mat4 projection;
	mat4 model;
	float threshold;
} ubo;

layout(location = POSITION_LOCATION) in vec3 aPosition;
layout(location = TEXCOORD_LOCATION) in vec2 aTexCoord;

layout(location = 0) out struct { vec2 TexCoord; } Out;

out gl_PerVertex { vec4 gl_Position; };

void main()
{
	gl_Position = ubo.projection * ubo.view * ubo.model * vec4(aPosition, 1.0);
	Out.TexCoord = aTexCoord;
#ifdef FLIP_TEXCOORD_Y
	Out.TexCoord.y = 1.0 - Out.TexCoord.y;
#endif
})";

static std::string src_fragment = R"(
#version 450

layout(binding = 1) uniform constants
{
	mat4 view;
	mat4 projection;
	mat4 model;
	float threshold;
} ubo;

layout(binding = 0) uniform sampler2D sTexture;

layout(location = 0) in struct { vec2 TexCoord; } In;

layout(location = 0) out vec4 result;

void main()
{
	result = texture(sTexture, In.TexCoord);

	vec3 luminanceVector = vec3(0.2125, 0.7154, 0.0721);

	float luminance = dot(luminanceVector, result.xyz);
	luminance = max(0.0, luminance - ubo.threshold);
	result *= sign(luminance);
})";

BrightFilter::BrightFilter(const skygfx::Vertex::Layout& layout) : Shader(layout, src_vertex, src_fragment)
{
}

void BrightFilter::update()
{
	Shader::update();
	RENDERER->setUniformBuffer(1, mSettings);
}