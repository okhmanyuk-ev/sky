#include "bright_filter.h"

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

			float uThreshold;
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

		void main()
		{
			vec4 result = texture2D(uTexture, vTexCoord);

			vec3 luminanceVector = vec3(0.2125, 0.7154, 0.0721);

			float luminance = dot(luminanceVector, result.xyz);
			luminance = max(0.0, luminance - uThreshold);
			result *= sign(luminance);

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

			float threshold;
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

		float4 ps_main(PixelInput input) : SV_TARGET
		{
			float4 result = texture0.Sample(sampler0, input.uv);

			float3 luminanceVector = float3(0.2125, 0.7154, 0.0721);

			float luminance = dot(luminanceVector, result.rgb);
			luminance = max(0.0, luminance - threshold);
			result *= sign(luminance);

			return result;
		})";
#endif
}

ShaderBrightFilter::ShaderBrightFilter(const Vertex::Layout& layout) :
	ShaderCustom(layout, { Vertex::Attribute::Type::Position, Vertex::Attribute::Type::TexCoord }, sizeof(CustomConstantBuffer), shaderSource)
{
	setCustomConstantBuffer(&mCustomConstantBuffer);
}

ShaderBrightFilter::~ShaderBrightFilter()
{
	//
}