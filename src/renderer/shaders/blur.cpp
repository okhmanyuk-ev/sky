#include "blur.h"

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

			vec2 uDirection;
			vec2 uResolution;
		};

		uniform sampler2D uTexture;

		#ifdef VERTEX_SHADER
		in vec3 aPosition;
		
		void main()
		{
			gl_Position = uProjectionMatrix * uViewMatrix * uModelMatrix * vec4(aPosition, 1.0);
		}
		#endif

		#ifdef FRAGMENT_SHADER
		out vec4 fragColor;

		void main()
		{
			vec4 result = vec4(0.0);

			vec2 off1 = vec2(1.3846153846) * uDirection / uResolution;
			vec2 off2 = vec2(3.2307692308) * uDirection / uResolution;
			
			vec2 uv = vec2(gl_FragCoord.xy / uResolution.xy);

			result += texture(uTexture, uv) * 0.2270270270;
	
			result += texture(uTexture, uv + off1) * 0.3162162162;
			result += texture(uTexture, uv - off1) * 0.3162162162;

			result += texture(uTexture, uv + off2) * 0.0702702703;
			result += texture(uTexture, uv - off2) * 0.0702702703;

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
		};

		struct VertexInput
		{
			float3 pos : POSITION0;
		};

		struct PixelInput
		{
			float4 pixelPosition : SV_POSITION;
		};

		sampler sampler0;
		Texture2D texture0;

		PixelInput vs_main(VertexInput input)
		{
			PixelInput result;
			result.pixelPosition = mul(projectionMatrix, mul(viewMatrix, mul(modelMatrix, float4(input.pos, 1.0))));
			return result;
		};

		float4 ps_main(PixelInput input) : SV_TARGET
		{
			float4 result = 0;
			
			float2 off1 = 1.3846153846 * direction / resolution;
			float2 off2 = 3.2307692308 * direction / resolution;
			
			float2 uv = input.pixelPosition / resolution;

			result += texture0.Sample(sampler0, uv) * 0.2270270270;

			result += texture0.Sample(sampler0, uv + off1) * 0.3162162162;
			result += texture0.Sample(sampler0, uv - off1) * 0.3162162162;

			result += texture0.Sample(sampler0, uv + off2) * 0.0702702703;
			result += texture0.Sample(sampler0, uv - off2) * 0.0702702703;

			return result;
		})";
#endif
}

ShaderBlur::ShaderBlur(const Vertex::Layout& layout) : 
	ShaderCustom(layout, { Vertex::Attribute::Type::Position }, sizeof(CustomConstantBuffer), shaderSource)
{
	setCustomConstantBuffer(&mCustomConstantBuffer);
}

ShaderBlur::~ShaderBlur()
{
	//
}