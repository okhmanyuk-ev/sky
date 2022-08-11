#include "blur.h"
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
	vec2 direction;
	vec2 resolution;
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
	vec2 direction;
	vec2 resolution;
} ubo;

layout(binding = 0) uniform sampler2D sTexture;

layout(location = 0) in struct { vec2 TexCoord; } In;

layout(location = 0) out vec4 result;

void main()
{
	result = vec4(0.0);

	vec2 off1 = vec2(1.3846153846) * ubo.direction / ubo.resolution;
	vec2 off2 = vec2(3.2307692308) * ubo.direction / ubo.resolution;
			
	result += texture(sTexture, In.TexCoord) * 0.2270270270;
	
	result += texture(sTexture, In.TexCoord + off1) * 0.3162162162;
	result += texture(sTexture, In.TexCoord - off1) * 0.3162162162;

	result += texture(sTexture, In.TexCoord + off2) * 0.0702702703;
	result += texture(sTexture, In.TexCoord - off2) * 0.0702702703;
})";

Blur::Blur(const skygfx::Vertex::Layout& layout) : Shader(layout, src_vertex, src_fragment)
{
}

void Blur::update()
{
	Shader::update();
	RENDERER->setUniformBuffer(1, mSettings);
}
