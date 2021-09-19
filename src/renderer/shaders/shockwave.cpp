#include "shockwave.h"

using namespace Renderer;
using namespace Renderer::Shaders;

namespace
{
	const char* shaderSource =
#if defined(RENDERER_GL44) || defined(RENDERER_GLES3)
		R"(
		layout (std140) uniform ConstantBuffer
		{
			mat4 uViewMatrix;
			mat4 uProjectionMatrix;
			mat4 uModelMatrix;

			vec4 uColor;
			float uIntensity;
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

		void main()
		{
			vec4 result = vec4(1.0, 1.0, 1.0, 1.0);

		#ifdef HAS_COLOR_ATTRIB
			result *= vColor;
		#endif
		#ifdef HAS_TEXCOORD_ATTRIB
			result *= texture(uTexture, vTexCoord);
		#endif
			result *= uColor;

			float gray = dot(result.rgb, vec3(0.299, 0.587, 0.114)); 

			result.rgb = mix(result.rgb, vec3(gray), uIntensity);

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

			float progress;
		};

		struct VertexInput
		{
			float3 pos : POSITION0;
		#ifdef HAS_COLOR_ATTRIB
			float4 col : COLOR0;
		#endif
			float2 uv : TEXCOORD0;
		};

		struct PixelInput
		{
			float3 pos : POSITION0;
		#ifdef HAS_COLOR_ATTRIB
			float4 col : COLOR0;
		#endif
			float2 uv : TEXCOORD0;
			float4 pixelPosition : SV_POSITION;
		};

		sampler sampler0;
		Texture2D texture0;

		PixelInput vs_main(VertexInput input)
		{
			PixelInput result;
			result.pos = input.pos;
		#ifdef HAS_COLOR_ATTRIB
			result.col = input.col;
		#endif
			result.uv = input.uv;

			result.pixelPosition = mul(projectionMatrix, mul(viewMatrix, mul(modelMatrix, float4(input.pos, 1.0))));
			return result;
		};

		#define CIRCLE_CENTER float2(0.9, 0.5)
		#define MAX_RADIUS 0.5

		float circleFast(float2 st, float2 pos, float r) {
			float2 dist = st - pos;
			return 1.0 - smoothstep(r - (0.09), r + (0.09), dot(dist, dist) * 4.0);
		}

		float4 ps_main(PixelInput input) : SV_TARGET
		{
			//float4 result = float4(1.0, 1.0, 1.0, 1.0);
			
		#ifdef HAS_COLOR_ATTRIB
			//result *= input.col;
		#endif
		
			//result *= texture0.Sample(sampler0, input.uv);
			//result *= color;


			float factor = 1.0 / (progress + 0.001);
			float2 pos = floor(input.uv * factor + 0.5) / factor;
			float4 result = texture0.Sample(sampler0, pos) * input.col;

			return result;
		}
		)";
#endif
}

Shockwave::Shockwave(const Vertex::Layout& layout, const std::set<Flag>& flags) :
	ShaderCustom(layout, { Vertex::Attribute::Type::Position, Vertex::Attribute::Type::TexCoord }, sizeof(CustomConstantBuffer), 
		MakeDefinesFromFlags(layout, shaderSource, flags))
{
	setCustomConstantBuffer(&mCustomConstantBuffer);
}

Shockwave::Shockwave(const Vertex::Layout& layout) : Shockwave(layout, MakeFlagsFromLayout(layout))
{
	//
};

std::set<Shockwave::Flag> Shockwave::MakeFlagsFromLayout(const Vertex::Layout& layout)
{
	std::set<Flag> result = { };

	if (layout.hasAttribute(Vertex::Attribute::Type::Color))
		result.insert(Flag::Colored);

	if (layout.hasAttribute(Vertex::Attribute::Type::TexCoord))
		result.insert(Flag::Textured);

	return result;
}

std::string Shockwave::MakeDefinesFromFlags(const Vertex::Layout& layout, const std::string& source, const std::set<Flag>& flags)
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

	return result;
}