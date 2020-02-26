#include <Renderer/shader_blur.h>

#if defined(RENDERER_D3D11)
#include <Renderer/system_d3d11.h>

using namespace Renderer;

struct ShaderBlur::Impl
{
	ID3D11VertexShader* vertexShader = nullptr;
	ID3D11PixelShader* pixelShader = nullptr;
	ID3D11Buffer* constantBuffer = nullptr;
	ID3D11InputLayout* inputLayout = nullptr;
};

namespace
{
	const char* shaderSource = R"(
		cbuffer ConstantBuffer : register(b0)
		{
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
			result.pixelPosition = float4(input.pos, 1.0);
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
}

ShaderBlur::ShaderBlur(const Vertex::Layout& layout)
{
    mImpl = std::make_unique<Impl>();
	checkRequiredAttribs(requiredAttribs, layout);
	
	ID3DBlob* vertexShaderBlob;
	ID3DBlob* pixelShaderBlob;

	ID3DBlob* vertex_shader_error;
	ID3DBlob* pixel_shader_error;

	std::string source = shaderSource;

	D3DCompile(source.c_str(), source.size(), NULL, NULL, NULL, "vs_main", "vs_4_0", 0, 0, &vertexShaderBlob, &vertex_shader_error);
	D3DCompile(source.c_str(), source.size(), NULL, NULL, NULL, "ps_main", "ps_4_0", 0, 0, &pixelShaderBlob, &pixel_shader_error);

	//std::string vertex_shader_error_string = (char*)vertex_shader_error->GetBufferPointer();
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

ShaderBlur::~ShaderBlur()
{
	
}

void ShaderBlur::apply()
{
	SystemD3D11::Context->IASetInputLayout(mImpl->inputLayout);
	SystemD3D11::Context->VSSetShader(mImpl->vertexShader, nullptr, 0);
	SystemD3D11::Context->PSSetShader(mImpl->pixelShader, nullptr, 0);
	SystemD3D11::Context->VSSetConstantBuffers(0, 1, &mImpl->constantBuffer);
	SystemD3D11::Context->PSSetConstantBuffers(0, 1, &mImpl->constantBuffer);
	mConstantBufferDirty = true;
}

void ShaderBlur::update()
{
	if (!mConstantBufferDirty)
		return;

	mConstantBufferDirty = false;
	SystemD3D11::Context->UpdateSubresource(mImpl->constantBuffer, 0, NULL, &mConstantBuffer, 0, 0);
}

#endif