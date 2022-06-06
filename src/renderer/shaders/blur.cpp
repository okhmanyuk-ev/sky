#include "blur.h"

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

layout(location = 0) in vec3 aPosition;
layout(location = 2) in vec2 aTexCoord;

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

Blur::Blur(const Vertex::Layout& layout) : ShaderCross(layout, src_vertex, src_fragment)
{
}

void Blur::update()
{
	ShaderCross::update();
	RENDERER->setUniformBuffer(1, mSettings);
}

namespace
{
	const char* shaderSource2 =
#if defined(RENDERER_GL44) || defined(RENDERER_GLES3) || defined(RENDERER_VK)
		R"(
		layout (std140) uniform ConstantBuffer
		{			
			mat4 uViewMatrix;
			mat4 uProjectionMatrix;
			mat4 uModelMatrix;

			vec2 uDirection;
			vec2 uResolution;
			float uSigma;
			float uIterations;
		};

		uniform sampler2D uTexture;

		#ifdef VERTEX_SHADER
		in vec3 aPosition;
		in vec2 aTexCoord;

		out vec2 vTexCoord;
		
		void main()
		{
			gl_Position = uProjectionMatrix * uViewMatrix * uModelMatrix * vec4(aPosition, 1.0);
			vTexCoord = aTexCoord;
		}
		#endif

		#ifdef FRAGMENT_SHADER
		in vec2 vTexCoord;

		out vec4 fragColor;

		float calcGauss(float offset, float sigma)
		{
			if (sigma <= 0.0)
				return 0.0;

			return exp(-pow(offset, 2.0) / sigma) / (sigma * 3.14157);
		}

		vec4 blur()
		{
			vec4 src = texture(uTexture, vTexCoord);
			vec4 result = vec4(src.rgb, 1.0);
			vec2 step = uDirection / uResolution;

			for (int i = 1; i <= int(uIterations); i++)
			{
				float weight = calcGauss(float(i) / uIterations, uSigma);
				
				if (weight < 1.0 / 255.0)
		            break;

				vec2 offset = step * float(i);

				src = texture(uTexture, vTexCoord + offset);
				result += vec4(src.rgb * weight, weight);

				src = texture(uTexture, vTexCoord - offset);
				result += vec4(src.rgb * weight, weight);
			}

			result.rgb = clamp(result.rgb / result.w, 0.0, 1.0);
			return vec4(result.rgb, 1.0);
		}

		void main()
		{
			vec4 result = vec4(0.0);

			result += blur();
		
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

			float2 direction;
			float2 resolution;
			float sigma;
			float iterations;
		};

		struct VertexInput
		{
			float3 pos : POSITION0;
			float2 uv : TEXCOORD0;
		};

		struct PixelInput
		{
			float4 pixelPosition : SV_POSITION;
			float2 uv : TEXCOORD0;
		};

		sampler sampler0;
		Texture2D texture0;

		PixelInput vs_main(VertexInput input)
		{
			PixelInput result;
			result.pixelPosition = mul(projectionMatrix, mul(viewMatrix, mul(modelMatrix, float4(input.pos, 1.0))));
			result.uv = input.uv;
			return result;
		};

		float calcGauss(float offset, float sigma)
		{
			if (sigma <= 0.0)
				return 0.0;

			return exp(-pow(offset, 2.0) / sigma) / (sigma * 3.14157);
		}

		float4 blur(float2 uv)
		{
			float4 src = texture0.Sample(sampler0, uv);
			float4 result = float4(src.rgb, 1.0);
			float2 step = direction / resolution;

			for (int i = 1; i <= int(iterations); i++)
			{
				float weight = calcGauss(float(i) / iterations, sigma);
				
				if (weight < 1.0 / 255.0)
		            break;

				float2 offset = step * float(i);

				src = texture0.Sample(sampler0, uv + offset);
				result += float4(src.rgb * weight, weight);

				src = texture0.Sample(sampler0, uv - offset);
				result += float4(src.rgb * weight, weight);
			}

			result.rgb = clamp(result.rgb / result.w, 0.0, 1.0);
			return float4(result.rgb, 1.0);
		}

		float4 ps_main(PixelInput input) : SV_TARGET
		{
			float4 result = 0;
			
			result += blur(input.uv);

			return result;
		})";
#endif
}

Blur2::Blur2(const Vertex::Layout& layout) :
	ShaderCustom(layout, { Vertex::Attribute::Type::Position, Vertex::Attribute::Type::TexCoord }, sizeof(ConstantBuffer), shaderSource2)
{
	setCustomConstantBuffer(&mConstantBuffer);
}
