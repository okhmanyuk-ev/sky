#include "sdf.h"

using namespace Renderer;
using namespace Renderer::Shaders;

static std::string src_ubo = R"(
layout(binding = 2) uniform _settings
{
	vec4 color;
	float min_value;
	float max_value;
	float smooth_factor;
} settings;
)";

static std::string src_fragment = R"(
vec4 fragment(vec4 result)
{
	float distance = texture(sTexture, In.TexCoord).a;
	float min_alpha = smoothstep(settings.min_value - settings.smooth_factor, settings.min_value + settings.smooth_factor, distance);
	float max_alpha = smoothstep(settings.max_value + settings.smooth_factor, settings.max_value - settings.smooth_factor, distance);
	result = vec4(0.0, 0.0, 0.0, 0.0);
	if (max_alpha > 0.0 && min_alpha > 0.0)
	{
		if (max_alpha > 0.0)
		{
			result = vec4(settings.color.rgb, settings.color.a * max_alpha);
		}
		if (min_alpha > 0.0 && min_alpha < max_alpha)
		{
			result = vec4(settings.color.rgb, settings.color.a * min_alpha);
		}
	}
	return result;
}
)";

Sdf::Sdf(const Vertex::Layout& layout) : Generic(layout, src_ubo, src_fragment)
{
}

void Sdf::update()
{
	Generic::update();
	pushConstants(2, mSettings);
}
