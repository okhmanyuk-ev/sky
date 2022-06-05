#include "generic.h"

using namespace Renderer;
using namespace Renderer::Shaders;

Generic::Generic(const Vertex::Layout& layout, const std::set<Flag>& flags,
	std::optional<std::string> additional_ubo, std::optional<std::string> additional_fragment_func) :
	ShaderCross(layout, GenerateVertexCode(layout, flags), GenerateFragmentCode(layout, flags, additional_ubo, additional_fragment_func))
{
}

Generic::Generic(const Vertex::Layout& layout, std::optional<std::string> additional_ubo,
	std::optional<std::string> additional_fragment_func) : Generic(layout, MakeFlagsFromLayout(layout), additional_ubo, additional_fragment_func)
{
}

void Generic::update()
{
	ShaderCross::update();
	ShaderCross::pushConstants(1, mConstantBuffer);
}

std::string Generic::GenerateVertexCode(const Vertex::Layout& layout, const std::set<Flag>& flags)
{
	auto result = MakeShaderHeader(layout, flags);

	result += R"(
layout(location = 0) in vec3 aPosition;

#ifdef HAS_COLOR_ATTRIB
layout(location = 1) in vec4 aColor;
#endif

#ifdef HAS_TEXCOORD_ATTRIB
layout(location = 2) in vec2 aTexCoord;
#endif

layout(binding = 1) uniform constants
{
	mat4 projection;
	mat4 view;
	mat4 model;
	float lod_bias;
} ubo;

layout(location = 0) out struct 
{
	vec3 Position;
#ifdef HAS_COLOR_ATTRIB
	vec4 Color;
#endif
#ifdef HAS_TEXCOORD_ATTRIB
	vec2 TexCoord;
#endif
} Out;

out gl_PerVertex 
{
	vec4 gl_Position;
};

void main()
{
	Out.Position = aPosition;
#ifdef HAS_COLOR_ATTRIB
	Out.Color = aColor;
#endif
#ifdef HAS_TEXCOORD_ATTRIB
	Out.TexCoord = aTexCoord;
#ifdef FLIP_TEXCOORD_Y
	Out.TexCoord.y = 1.0 - Out.TexCoord.y;
#endif
#endif
	gl_Position = ubo.projection * ubo.view * ubo.model * vec4(aPosition, 1.0);
}
)";

	return result;
}

std::string Generic::GenerateFragmentCode(const Vertex::Layout& layout, const std::set<Flag>& flags,
	std::optional<std::string> additional_ubo,
	std::optional<std::string> additional_fragment_func)
{
	auto result = MakeShaderHeader(layout, flags);

	if (additional_fragment_func.has_value())
	{
		result += "#define HAS_ADDITIONAL_FRAGMENT_FUNC\n";
	}

	if (additional_ubo.has_value())
	{
		result += additional_ubo.value();
	}

	result += R"(
layout(location = 0) out vec4 result;

#ifdef HAS_TEXCOORD_ATTRIB
layout(binding = 0) uniform sampler2D sTexture;
#endif

layout(binding = 1) uniform constants
{
	mat4 projection;
	mat4 view;
	mat4 model;
	float lod_bias;
} ubo;

layout(location = 0) in struct 
{
	vec3 Position;
#ifdef HAS_COLOR_ATTRIB
	vec4 Color;
#endif
#ifdef HAS_TEXCOORD_ATTRIB
	vec2 TexCoord;
#endif
} In;

#ifdef HAS_ADDITIONAL_FRAGMENT_FUNC
vec4 fragment(vec4 result);
#endif

void main()
{
	result = vec4(1.0);
#ifdef HAS_COLOR_ATTRIB
	result *= In.Color;
#endif
#ifdef HAS_TEXCOORD_ATTRIB
	result *= texture(sTexture, In.TexCoord, ubo.lod_bias);
#endif
#ifdef HAS_ADDITIONAL_FRAGMENT_FUNC
	result = fragment(result);
#endif
}
)";

	if (additional_fragment_func.has_value())
	{
		result += additional_fragment_func.value();
	}

	return result;
}

std::string Generic::MakeShaderHeader(const Vertex::Layout& layout, const std::set<Flag>& flags)
{
	std::string result;

	result += "#version 450 core\n";

	if (flags.count(Flag::Colored) > 0)
	{
		assert(layout.hasAttribute(Renderer::Vertex::Attribute::Type::Color));
		result += "#define HAS_COLOR_ATTRIB\n";
	}

	if (flags.count(Flag::Textured) > 0)
	{
		assert(layout.hasAttribute(Renderer::Vertex::Attribute::Type::TexCoord));
		result += "#define HAS_TEXCOORD_ATTRIB\n";
	}

	return result;
}

std::set<Generic::Flag> Generic::MakeFlagsFromLayout(const Vertex::Layout& layout)
{
	std::set<Flag> result = { };

	if (layout.hasAttribute(Vertex::Attribute::Type::Color))
		result.insert(Flag::Colored);

	if (layout.hasAttribute(Vertex::Attribute::Type::TexCoord))
		result.insert(Flag::Textured);

	return result;
}