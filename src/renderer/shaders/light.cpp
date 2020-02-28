#include "light.h"

using namespace Renderer;
using namespace Renderer::Shaders;

namespace
{
	const char* shaderSource =
#if defined(RENDERER_GL44) || defined(RENDERER_GLES3)
		R"(
		struct DirectionalLight
		{
			vec3 direction;
			vec3 ambient;
			vec3 diffuse;
			vec3 specular;
		};

		struct PointLight
		{
			vec3 position;

			float constantAttenuation;
			float linearAttenuation;
			float quadraticAttenuation;

			vec3 ambient;
			vec3 diffuse;
			vec3 specular;
		};

		struct Material
		{
			vec3 ambient;
			vec3 diffuse;
			vec3 specular;
			float shininess;
		};

		layout (std140) uniform ConstantBuffer
		{
			mat4 viewMatrix;
			mat4 projectionMatrix;
			mat4 modelMatrix;

			vec3 eyePosition;
			
			DirectionalLight directionalLight;
			PointLight pointLight;

			Material material;
		};

		uniform sampler2D uTexture;

		#ifdef VERTEX_SHADER
		in vec3 aPosition;
		out vec3 vPosition;
		in vec3 aNormal;
		out vec3 vNormal;
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
			vPosition = vec3(modelMatrix * vec4(aPosition, 1.0));
		//	vNormal = mat3(transpose(inverse(modelMatrix))) * aNormal; // -- this was from tutorial
			vNormal = vec3(modelMatrix * vec4(aNormal, 1.0)); // i use this, because no changes with upper
		#ifdef HAS_COLOR_ATTRIB
			vColor = aColor;
		#endif
		#ifdef HAS_TEXCOORD_ATTRIB
			vTexCoord = aTexCoord;
		#endif
			gl_Position = projectionMatrix * viewMatrix * vec4(vPosition, 1.0);
		}
		#endif

		#ifdef FRAGMENT_SHADER
		in vec3 vPosition;
		in vec3 vNormal;
		#ifdef HAS_COLOR_ATTRIB
		in vec4 vColor;
		#endif
		#ifdef HAS_TEXCOORD_ATTRIB
		in vec2 vTexCoord;
		#endif	
	
		vec3 CalcDirectionalLight(DirectionalLight light, vec3 normal, vec3 viewDirection);
		vec3 CalcPointLight(PointLight light, vec3 normal, vec3 viewDirection);

		out vec4 resultColor;

		void main()
		{
			resultColor = vec4(1.0, 1.0, 1.0, 1.0);
		#ifdef HAS_COLOR_ATTRIB
			resultColor *= vColor;
		#endif
		#ifdef HAS_TEXCOORD_ATTRIB
			resultColor *= texture(uTexture, vTexCoord);
		#endif
			
			vec3 normal = normalize(vNormal);
			vec3 viewDirection = normalize(eyePosition - vPosition);

			vec3 intensity = CalcDirectionalLight(directionalLight, normal, viewDirection);
			intensity += CalcPointLight(pointLight, normal, viewDirection);
		
			resultColor *= vec4(intensity, 1.0);
		}

		vec3 CalcLight(vec3 normal, vec3 viewDirection, vec3 lightDirection, vec3 lightAmbient, vec3 lightDiffuse, vec3 lightSpecular)
		{
			vec3 lightDir = normalize(lightDirection);

			float diff = max(dot(normal, lightDir), 0.0);

			vec3 reflectDir = reflect(-lightDir, normal);
			float spec = pow(max(dot(viewDirection, reflectDir), 0.0), material.shininess);

			vec3 ambient = lightAmbient * material.diffuse;
			vec3 diffuse = lightDiffuse * diff * material.diffuse;
			vec3 specular = lightSpecular * spec * material.specular;

			return ambient + diffuse + specular;
		}

		vec3 CalcDirectionalLight(DirectionalLight light, vec3 normal, vec3 viewDirection)
		{
			return CalcLight(normal, viewDirection, -light.direction, light.ambient, light.diffuse, light.specular);
		}

		vec3 CalcPointLight(PointLight light, vec3 normal, vec3 viewDirection)
		{
			vec3 lightDir = light.position - vPosition;

			float distance = length(lightDir);
			float linearAttn = light.linearAttenuation * distance;
			float quadraticAttn = light.quadraticAttenuation * (distance * distance);
			float attenuation = 1.0 / (light.constantAttenuation + linearAttn + quadraticAttn);

			return CalcLight(normal, viewDirection, lightDir, light.ambient, light.diffuse, light.specular) * attenuation;
		}
		#endif
		)";
#elif defined(RENDERER_D3D11)
		R"(
		struct DirectionalLight
		{
			float3 direction;
			float3 ambient;
			float3 diffuse;
			float3 specular;
		};

		struct PointLight
		{
			float3 position;

			float constantAttenuation;
			float linearAttenuation;
			float quadraticAttenuation;

			float3 ambient;
			float3 diffuse;
			float3 specular;
		};

		struct Material {
			float3 ambient;
			float3 diffuse;
			float3 specular;
			float shininess;
		};

		cbuffer ConstantBuffer : register(b0)
		{
			float4x4 viewMatrix;
			float4x4 projectionMatrix;
			float4x4 modelMatrix;

			float3 eyePosition;
			
			DirectionalLight directionalLight;
			PointLight pointLight;

			Material material;
		};

		struct VertexInput
		{
			float3 pos : POSITION0;
			float3 normal : NORMAL;
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
			float3 normal : NORMAL;
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
			result.pos = mul(modelMatrix, float4(input.pos, 1.0));
			result.normal = mul(modelMatrix, float4(input.normal, 1.0));
		#ifdef HAS_COLOR_ATTRIB
			result.col = input.col;
		#endif
		#ifdef HAS_TEXCOORD_ATTRIB
			result.uv = input.uv;
		#endif
			result.pixelPosition = mul(projectionMatrix, mul(viewMatrix, float4(result.pos, 1.0)));
			return result;
		};

		float3 CalcLight(float3 normal, float3 viewDirection, float3 lightDirection, float3 lightAmbient, float3 lightDiffuse, float3 lightSpecular)
		{
			float3 lightDir = normalize(lightDirection);

			float diff = max(dot(normal, lightDir), 0.0);

			float3 reflectDir = reflect(-lightDir, normal);
			float spec = pow(max(dot(viewDirection, reflectDir), 0.0), material.shininess);

			float3 ambient = lightAmbient * material.diffuse;
			float3 diffuse = lightDiffuse * diff * material.diffuse;
			float3 specular = lightSpecular * spec * material.specular;

			return ambient + diffuse + specular;
		}

		float3 CalcDirectionalLight(DirectionalLight light, float3 normal, float3 viewDirection)
		{
			return CalcLight(normal, viewDirection, -light.direction, light.ambient, light.diffuse, light.specular);
		}

		float3 CalcPointLight(PointLight light, float3 normal, float3 viewDirection, float3 pixelPosition)
		{
			float3 lightDir = light.position - pixelPosition;

			float distance = length(lightDir);
			float linearAttn = light.linearAttenuation * distance;
			float quadraticAttn = light.quadraticAttenuation * (distance * distance);
			float attenuation = 1.0 / (light.constantAttenuation + linearAttn + quadraticAttn);

			return CalcLight(normal, viewDirection, lightDir, light.ambient, light.diffuse, light.specular) * attenuation;
		}

		float4 ps_main(PixelInput input) : SV_TARGET
		{
			float4 result = float4(1.0, 1.0, 1.0, 1.0);
		#ifdef HAS_COLOR_ATTRIB
			result *= input.col;
		#endif
		#ifdef HAS_TEXCOORD_ATTRIB
			result *= texture0.Sample(sampler0, input.uv);
		#endif

			float3 normal = normalize(input.normal);
			float3 viewDirection = normalize(eyePosition - input.pos);

			float3 intensity = CalcDirectionalLight(directionalLight, normal, viewDirection);
			intensity += CalcPointLight(pointLight, normal, viewDirection, input.pos);

			result *= float4(intensity, 1.0);

			return result;
		}
		)";
#endif
}

Light::Light(const Vertex::Layout& layout, const std::set<Flag>& flags) :
	ShaderCustom(layout, { Vertex::Attribute::Type::Position }, sizeof(CustomConstantBuffer),
		MakeDefinesFromFlags(layout, shaderSource, flags))
{
	setCustomConstantBuffer(&mCustomConstantBuffer);
}

Light::Light(const Vertex::Layout& layout) : Light(layout, MakeFlagsFromLayout(layout))
{
	//
};

std::set<Light::Flag> Light::MakeFlagsFromLayout(const Vertex::Layout& layout)
{
	std::set<Flag> result = { };

	if (layout.hasAttribute(Vertex::Attribute::Type::Color))
		result.insert(Flag::Colored);

	if (layout.hasAttribute(Vertex::Attribute::Type::TexCoord))
		result.insert(Flag::Textured);

	return result;
}

std::string Light::MakeDefinesFromFlags(const Vertex::Layout& layout, const std::string& source, const std::set<Flag>& flags)
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