#include "generic.h"
#include <renderer/system.h>

using namespace Renderer;
using namespace Renderer::Shaders;

Generic::Generic(const Vertex::Layout& layout, const std::set<Flag>& flags, 
	std::optional<std::string> additional_fragment_func, const std::vector<std::string>& defines) :
	Shader(layout, 
		GenerateVertexCode(layout, flags), 
		GenerateFragmentCode(layout, flags, additional_fragment_func), 
		GenerateDefines(defines, layout, flags, additional_fragment_func.has_value()))
{
}

Generic::Generic(const Vertex::Layout& layout, std::optional<std::string> additional_fragment_func, 
	const std::vector<std::string>& defines) :
	Generic(layout, MakeFlagsFromLayout(layout), additional_fragment_func, defines)
{
}

void Generic::update()
{
	Shader::update();
	RENDERER->setUniformBuffer(1, mConstantBuffer);
}

std::string Generic::GenerateVertexCode(const Vertex::Layout& layout, const std::set<Flag>& flags)
{
	return R"(
#version 450 core

layout(location = POSITION_LOCATION) in vec3 aPosition;

#ifdef HAS_COLOR_ATTRIB
layout(location = COLOR_LOCATION) in vec4 aColor;
#endif

#ifdef HAS_TEXCOORD_ATTRIB
layout(location = TEXCOORD_LOCATION) in vec2 aTexCoord;
#endif

#ifdef HAS_NORMAL_ATTRIB
layout(location = NORMAL_LOCATION) in vec3 aNormal;
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
#ifdef HAS_NORMAL_ATTRIB
	vec3 Normal;
#endif
} Out;

out gl_PerVertex { vec4 gl_Position; };

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
#ifdef HAS_NORMAL_ATTRIB
	Out.Normal = aNormal;
#endif
	gl_Position = ubo.projection * ubo.view * ubo.model * vec4(aPosition, 1.0);
})";
}

std::string Generic::GenerateFragmentCode(const Vertex::Layout& layout, const std::set<Flag>& flags,
	std::optional<std::string> additional_fragment_func)
{
	std::string result = R"(
#version 450 core

layout(location = 0) out vec4 result;

#ifdef HAS_TEXTURE_FEATURE
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
#ifdef HAS_NORMAL_ATTRIB
	vec3 Normal;
#endif
} In;

#ifdef HAS_ADDITIONAL_FRAGMENT_FUNC_FEATURE
vec4 fragment(vec4 result);
#endif

void main()
{
	result = vec4(1.0);
#ifdef HAS_COLOR_FEATURE
	result *= In.Color;
#endif
#ifdef HAS_TEXTURE_FEATURE
	result *= texture(sTexture, In.TexCoord, ubo.lod_bias);
#endif
#ifdef HAS_ADDITIONAL_FRAGMENT_FUNC_FEATURE
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

std::vector<std::string> Generic::GenerateDefines(const std::vector<std::string>& defines, const Vertex::Layout& layout, 
	const std::set<Flag>& flags, bool custom_fragment_func)
{
	auto result = defines;

	const static std::unordered_map<Renderer::Vertex::Attribute::Type, std::string> AttribsDefines = {
		{ Renderer::Vertex::Attribute::Type::Position, "HAS_POSITION_ATTRIB" },
		{ Renderer::Vertex::Attribute::Type::Color, "HAS_COLOR_ATTRIB" },
		{ Renderer::Vertex::Attribute::Type::TexCoord, "HAS_TEXCOORD_ATTRIB" },
		{ Renderer::Vertex::Attribute::Type::Normal, "HAS_NORMAL_ATTRIB" },
	};

	const static std::unordered_map<Flag, std::string> FeatureDefines = {
		{ Flag::Colored, "HAS_COLOR_FEATURE" },
		{ Flag::Textured, "HAS_TEXTURE_FEATURE" }
	};

	for (const auto& attrib : layout.attributes)
	{
		result.push_back(AttribsDefines.at(attrib.type));
	}

	for (auto flag : flags)
	{
		result.push_back(FeatureDefines.at(flag));
	}

	if (custom_fragment_func)
		result.push_back("HAS_ADDITIONAL_FRAGMENT_FUNC_FEATURE");

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