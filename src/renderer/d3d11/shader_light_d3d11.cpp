#include <Renderer/shader_light.h>

#if defined(RENDERER_D3D11)
#include <Renderer/system_d3d11.h>

using namespace Renderer;

struct ShaderLight::Impl
{
	ID3D11VertexShader* vertexShader = nullptr;
	ID3D11PixelShader* pixelShader = nullptr;
	ID3D11Buffer* constantBuffer = nullptr;
	ID3D11InputLayout* inputLayout = nullptr;
};

namespace
{
	const char* shaderSource = R"(
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
		})";
}

ShaderLight::ShaderLight(const Vertex::Layout& layout)
{
    mImpl = std::make_unique<Impl>();
	checkRequiredAttribs(requiredAttribs, layout);
	
	ID3DBlob* vertexShaderBlob;
	ID3DBlob* pixelShaderBlob;

	ID3DBlob* vertex_shader_error;
	ID3DBlob* pixel_shader_error;

	std::string source = shaderSource;

	if (layout.hasAttribute(Vertex::Attribute::Type::Color))
		source = "#define HAS_COLOR_ATTRIB\n" + source;

	if (layout.hasAttribute(Vertex::Attribute::Type::TexCoord))
		source = "#define HAS_TEXCOORD_ATTRIB\n" + source;

	D3DCompile(source.c_str(), source.size(), NULL, NULL, NULL, "vs_main", "vs_4_0", 0, 0, &vertexShaderBlob, &vertex_shader_error);
	D3DCompile(source.c_str(), source.size(), NULL, NULL, NULL, "ps_main", "ps_4_0", 0, 0, &pixelShaderBlob, &pixel_shader_error);

	std::string vertex_shader_error_string = (char*)vertex_shader_error->GetBufferPointer();
	//std::string pixel_shader_error_string = (char*)pixel_shader_error->GetBufferPointer();

	SystemD3D11::Device->CreateVertexShader(vertexShaderBlob->GetBufferPointer(), vertexShaderBlob->GetBufferSize(), nullptr, &mImpl->vertexShader);
	SystemD3D11::Device->CreatePixelShader(pixelShaderBlob->GetBufferPointer(), pixelShaderBlob->GetBufferSize(), nullptr, &mImpl->pixelShader);

	std::vector<D3D11_INPUT_ELEMENT_DESC> input;

	for (auto& attrib : layout.attributes)
	{
		input.push_back({ SystemD3D11::Semantic.at(attrib.type).c_str(), 0, SystemD3D11::Format.at(attrib.format), 0,
			static_cast<UINT>(attrib.offset), D3D11_INPUT_PER_VERTEX_DATA, 0 });
	}

	SystemD3D11::Device->CreateInputLayout(input.data(), static_cast<UINT>(input.size()), vertexShaderBlob->GetBufferPointer(), vertexShaderBlob->GetBufferSize(), &mImpl->inputLayout);

	{
		D3D11_BUFFER_DESC desc = {};
		desc.ByteWidth = sizeof(mConstantBuffer);
		desc.Usage = D3D11_USAGE_DEFAULT;
		desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		desc.CPUAccessFlags = 0;
		desc.MiscFlags = 0;
		SystemD3D11::Device->CreateBuffer(&desc, NULL, &mImpl->constantBuffer);
	}
}

ShaderLight::~ShaderLight()
{

}

void ShaderLight::apply()
{
	SystemD3D11::Context->IASetInputLayout(mImpl->inputLayout);
	SystemD3D11::Context->VSSetShader(mImpl->vertexShader, nullptr, 0);
	SystemD3D11::Context->PSSetShader(mImpl->pixelShader, nullptr, 0);
	SystemD3D11::Context->VSSetConstantBuffers(0, 1, &mImpl->constantBuffer);
	SystemD3D11::Context->PSSetConstantBuffers(0, 1, &mImpl->constantBuffer);
	mConstantBufferDirty = true;
}

void ShaderLight::update()
{
	if (!mConstantBufferDirty)
		return;

	mConstantBufferDirty = false;
	SystemD3D11::Context->UpdateSubresource(mImpl->constantBuffer, 0, nullptr, &mConstantBuffer, 0, 0);
}

#endif