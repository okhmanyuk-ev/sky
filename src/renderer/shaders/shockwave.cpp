#include "shockwave.h"
#include <renderer/system.h>

using namespace Renderer;
using namespace Renderer::Shaders;

static std::string src_ubo = R"(
layout(binding = 2) uniform _settings
{
	float size;
	float thickness;
	float force;
} settings;
)";

static std::string src_fragment = R"(
vec4 fragment(vec4 result)
{
	const vec2 center = vec2(0.5, 0.5);
	float d = distance(In.TexCoord, center);
		
	float scaled_size = settings.size * 0.5;
	float scaled_thickness = settings.thickness * 0.25;

	float mask = (1.0 - smoothstep(scaled_size - scaled_thickness, scaled_size, d));
	mask *= smoothstep(scaled_size - scaled_thickness - scaled_thickness, scaled_size - scaled_thickness, d);
			
	vec2 disp = (In.TexCoord - center) * mask * settings.force;

	result = texture(sTexture, In.TexCoord - disp);

	return result;
}
)";

Shockwave::Shockwave(const Vertex::Layout& layout) : Generic(layout, src_ubo, src_fragment)
{
}

void Shockwave::update()
{
	Generic::update();
	RENDERER->setUniformBuffer(2, mSettings);
}
