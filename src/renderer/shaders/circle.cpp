#include "circle.h"

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

			vec4 color;

			vec4 inner_color;
			vec4 outer_color;

			float fill;
			float begin;
			float end;
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
			const float Pi = 3.14159265;

			vec2 center = vec2(0.5, 0.5);

			vec2 p = vPosition.xy - center;
			float angle = atan(-p.x, p.y);
			float normalized_angle = (angle + Pi) / 2.0 / Pi;			

			if (normalized_angle < begin || normalized_angle > end)
				discard;

			float maxRadius = 0.5;
			float minRadius = maxRadius * (1.0f - fill);
			
			float radius = distance(vPosition.xy, center);

			if (radius > maxRadius || radius < minRadius)
				discard;

			vec4 result = vec4(1.0, 1.0, 1.0, 1.0);

			float t = (radius - minRadius) / (maxRadius - minRadius);
			result *= mix(inner_color, outer_color, t);

		#ifdef HAS_COLOR_ATTRIB
			result *= vColor;
		#endif
		#ifdef HAS_TEXCOORD_ATTRIB
			result *= texture(uTexture, vTexCoord);
		#endif
			result *= color;
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

			float4 color;

			float4 inner_color;
			float4 outer_color;

			float fill;
			float begin;
			float end;
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

		float4 ps_main(PixelInput input) : SV_TARGET
		{
			static const float Pi = 3.14159265;

			float2 center = float2(0.5, 0.5);

			float2 p = input.pos - center;
			float angle = atan2(-p.x, p.y);
			float normalized_angle = (angle + Pi) / 2.0 / Pi;			

			if (normalized_angle < begin || normalized_angle > end)
				discard;

			float maxRadius = 0.5;
			float minRadius = maxRadius * (1.0f - fill);
			
			float radius = distance(input.pos, center);

			if (radius > maxRadius || radius < minRadius)
				discard;

			float4 result = float4(1.0, 1.0, 1.0, 1.0);

			float t = (radius - minRadius) / (maxRadius - minRadius);
			result *= lerp(inner_color, outer_color, t);

		#ifdef HAS_COLOR_ATTRIB
			result *= input.col;
		#endif
		#ifdef HAS_TEXCOORD_ATTRIB
			result *= texture0.Sample(sampler0, input.uv);
		#endif
			result *= color;
			return result;
		}
		)";
#endif
}

Circle::Circle(const Vertex::Layout& layout, const std::set<Flag>& flags) :
	ShaderCustom(layout, { Vertex::Attribute::Type::Position }, sizeof(CustomConstantBuffer), 
		MakeDefinesFromFlags(layout, shaderSource, flags))
{
	setCustomConstantBuffer(&mCustomConstantBuffer);
}

Circle::Circle(const Vertex::Layout& layout) : Circle(layout, MakeFlagsFromLayout(layout))
{
	//
};

std::set<Circle::Flag> Circle::MakeFlagsFromLayout(const Vertex::Layout& layout)
{
	std::set<Flag> result = { };

	if (layout.hasAttribute(Vertex::Attribute::Type::Color))
		result.insert(Flag::Colored);

	if (layout.hasAttribute(Vertex::Attribute::Type::TexCoord))
		result.insert(Flag::Textured);

	return result;
}

std::string Circle::MakeDefinesFromFlags(const Vertex::Layout& layout, const std::string& source, const std::set<Flag>& flags)
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