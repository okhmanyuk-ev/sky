#include "default.h"

using namespace Renderer;
using namespace Renderer::Shaders;

namespace
{
	const char* shaderSource =
#if defined(RENDERER_GL44) || defined(RENDERER_GLES3) || defined(RENDERER_VK)
		R"(
		layout (std140) uniform ConstantBuffer
		{
			mat4 uViewMatrix;
			mat4 uProjectionMatrix;
			mat4 uModelMatrix;

			//___CUSTOM_FIELDS_HERE
		};

		uniform sampler2D uTexture;

		#ifdef VERTEX_SHADER
		in vec3 aPosition;
		out vec3 vPosition;
		#ifdef HAS_COLOR_ATTRIB
		in vec4 aColor;
		out vec4 vColor;
		#endif
		#ifdef HAS_TEXCOORD_ATTRIB
		in vec2 aTexCoord;
		out vec2 vTexCoord;
		#endif	
	
		void main()
		{
			vPosition = aPosition;
		#ifdef HAS_COLOR_ATTRIB
			vColor = aColor;
		#endif
		#ifdef HAS_TEXCOORD_ATTRIB
			vTexCoord = aTexCoord;
			vTexCoord.y = 1.0 - vTexCoord.y;
		#endif

			gl_Position = uProjectionMatrix * uViewMatrix * uModelMatrix * vec4(aPosition, 1.0);
		}
		#endif

		#ifdef FRAGMENT_SHADER
		in vec3 vPosition;
		#ifdef HAS_COLOR_ATTRIB
		in vec4 vColor;
		#endif
		#ifdef HAS_TEXCOORD_ATTRIB
		in vec2 vTexCoord;
		#endif	
	
		out vec4 fragColor;

		#ifdef HAS_FRAGMENT_FUNC
		vec4 fragment(vec4 result);
		#endif

		void main()
		{
			vec4 result = vec4(1.0, 1.0, 1.0, 1.0);
		#ifdef HAS_COLOR_ATTRIB
			result *= vColor;
		#endif
		#ifdef HAS_TEXCOORD_ATTRIB
			result *= texture(uTexture, vTexCoord);
		#endif
		#ifdef HAS_FRAGMENT_FUNC
			result = fragment(result);
		#endif
			fragColor = result;
		}
		#endif
		)";
#elif defined(RENDERER_D3D11)
		R"(
		cbuffer ConstantBuffer : register(b0)
		{
			float4x4 viewMatrix;
			float4x4 projectionMatrix;
			float4x4 modelMatrix;
			
			//___CUSTOM_FIELDS_HERE
		};

		struct VertexInput
		{
			float3 pos : POSITION0;
		#ifdef HAS_COLOR_ATTRIB
			float4 col : COLOR0;
		#endif
		#ifdef HAS_TEXCOORD_ATTRIB
			float2 uv : TEXCOORD0;
		#endif
		};

		struct PixelInput
		{
			float3 pos : POSITION0;
		#ifdef HAS_COLOR_ATTRIB
			float4 col : COLOR0;
		#endif
		#ifdef HAS_TEXCOORD_ATTRIB
			float2 uv : TEXCOORD0;
		#endif
			float4 pixelPosition : SV_POSITION;
		};

		#ifdef HAS_TEXCOORD_ATTRIB
		sampler sampler0;
		Texture2D texture0;
		#endif

		PixelInput vs_main(VertexInput input)
		{
			PixelInput result;
			result.pos = input.pos;
		#ifdef HAS_COLOR_ATTRIB
			result.col = input.col;
		#endif
		#ifdef HAS_TEXCOORD_ATTRIB
			result.uv = input.uv;
		#endif
			result.pixelPosition = mul(projectionMatrix, mul(viewMatrix, mul(modelMatrix, float4(input.pos, 1.0))));
			return result;
		};

		#ifdef HAS_FRAGMENT_FUNC
		float4 fragment(float4 result, PixelInput input);
		#endif

		float4 ps_main(PixelInput input) : SV_TARGET
		{
			float4 result = float4(1.0, 1.0, 1.0, 1.0);
		#ifdef HAS_COLOR_ATTRIB
			result *= input.col;
		#endif
		#ifdef HAS_TEXCOORD_ATTRIB
			result *= texture0.Sample(sampler0, input.uv);
		#endif
		#ifdef HAS_FRAGMENT_FUNC
			result = fragment(result, input);
		#endif
			return result;
		}
		)";
#endif
}

Default::Default(const Vertex::Layout& layout, const std::set<Flag>& flags, size_t customConstantBufferSize, std::optional<CustomCode> custom_code) :
	ShaderCustom(layout, { Vertex::Attribute::Type::Position }, customConstantBufferSize,
		MakeDefinesFromFlags(layout, shaderSource, flags, custom_code))
{
	//
}

Default::Default(const Vertex::Layout& layout, size_t customConstantBufferSize, std::optional<CustomCode> custom_code) : Default(layout, MakeFlagsFromLayout(layout), customConstantBufferSize, custom_code)
{ 
	//
};

std::set<Default::Flag> Default::MakeFlagsFromLayout(const Vertex::Layout& layout)
{
	std::set<Flag> result = { };

	if (layout.hasAttribute(Vertex::Attribute::Type::Color))
		result.insert(Flag::Colored);

	if (layout.hasAttribute(Vertex::Attribute::Type::TexCoord))
		result.insert(Flag::Textured);

	return result;
}

std::string Default::MakeDefinesFromFlags(const Vertex::Layout& layout, const std::string& source, const std::set<Flag>& flags, std::optional<CustomCode> custom_code)
{
	auto result = source;

	if (flags.count(Flag::Colored) > 0)
	{
		assert(layout.hasAttribute(Renderer::Vertex::Attribute::Type::Color));
		result = "#define HAS_COLOR_ATTRIB\n" + result;
	}

	if (flags.count(Flag::Textured) > 0)
	{
		assert(layout.hasAttribute(Renderer::Vertex::Attribute::Type::TexCoord));
		result = "#define HAS_TEXCOORD_ATTRIB\n" + result;
	}

	if (custom_code.has_value())
	{
		result = "#define HAS_FRAGMENT_FUNC\n" + result;

		std::string replace = "//___CUSTOM_FIELDS_HERE";
		auto index = result.find(replace);
		assert(index != std::string::npos);
		result.replace(index, replace.length(), custom_code->constant_buffer_fields);

#if defined(RENDERER_GL44) || defined(RENDERER_GLES3)
		result += "\n#ifdef FRAGMENT_SHADER\n";
#endif
		result += custom_code->fragment_func;
#if defined(RENDERER_GL44) || defined(RENDERER_GLES3)
		result += "\n#endif\n";
#endif
	}

	return result;
}