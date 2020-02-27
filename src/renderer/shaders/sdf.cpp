#include "sdf.h"

using namespace Renderer;

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
			float uMinValue;
			float uMaxValue;
			float uSmoothFactor;
		};
	
		uniform sampler2D uTexture;
		
		#ifdef VERTEX_SHADER
		in vec3 aPosition;
		in vec2 aTexCoord;

		out vec2 vTexCoord;

		void main()
		{
			vTexCoord = aTexCoord;
			gl_Position = uProjectionMatrix * uViewMatrix * uModelMatrix * vec4(aPosition, 1.0);
		}
		#endif

		#ifdef FRAGMENT_SHADER
		in vec2 vTexCoord;

		out vec4 fragColor;

		void main()
		{
			float distance = texture(uTexture, vTexCoord).a;
			float minAlpha = smoothstep(uMinValue - uSmoothFactor, uMinValue + uSmoothFactor, distance);
			float maxAlpha = smoothstep(uMaxValue + uSmoothFactor, uMaxValue - uSmoothFactor, distance);
			vec4 result = vec4(0.0, 0.0, 0.0, 0.0);
			if (maxAlpha > 0.0 && minAlpha > 0.0)
			{
				if (maxAlpha > 0.0)
				{
					result = vec4(uColor.rgb, uColor.a * maxAlpha);
				}
				if (minAlpha > 0.0 && minAlpha < maxAlpha)
				{
					result = vec4(uColor.rgb, uColor.a * minAlpha);
				}
			}
			fragColor = result;
		}
		#endif)";
#elif defined(RENDERER_D3D11)
		R"(
		cbuffer ConstantBuffer : register(b0)
		{
			float4x4 viewMatrix;
			float4x4 projectionMatrix;
			float4x4 modelMatrix;

			float4 color;
			float minValue;
			float maxValue;
			float smoothFactor;
		};

		struct VertexInput
		{
			float3 pos : POSITION0;
			float2 uv : TEXCOORD0;
		};

		struct PixelInput
		{
			float2 uv : TEXCOORD0;
			float4 pixelPosition : SV_POSITION;
		};

		sampler sampler0;
		Texture2D texture0;

		PixelInput vs_main(VertexInput input)
		{
			PixelInput result;
			result.uv = input.uv;
			result.pixelPosition = mul(projectionMatrix, mul(viewMatrix, mul(modelMatrix, float4(input.pos, 1.0))));
			return result;
		};

		float4 ps_main(PixelInput input) : SV_TARGET
		{
			float distance = texture0.Sample(sampler0, input.uv).a;
			float minAlpha = smoothstep(minValue - smoothFactor, minValue + smoothFactor, distance);
			float maxAlpha = smoothstep(maxValue + smoothFactor, maxValue - smoothFactor, distance);
			float4 result = float4(0.0, 0.0, 0.0, 0.0);
			if (maxAlpha > 0.0 && minAlpha > 0.0)
			{
				if (maxAlpha > 0.0)
				{
					result = float4(color.rgb, color.a * maxAlpha);
				}
				if (minAlpha > 0.0 && minAlpha < maxAlpha)
				{
					result = float4(color.rgb, color.a * minAlpha);
				}
			}
			return result;
		})";
#endif
}

ShaderSdf::ShaderSdf(const Vertex::Layout& layout) :
	ShaderCustom(layout, { Vertex::Attribute::Type::Position, Vertex::Attribute::Type::TexCoord, }, sizeof(CustomConstantBuffer), shaderSource)
{
	setCustomConstantBuffer(&mCustomConstantBuffer);
}

ShaderSdf::~ShaderSdf()
{
	//
}