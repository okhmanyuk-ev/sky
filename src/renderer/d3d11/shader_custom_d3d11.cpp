#include <renderer/shader_custom.h>
#include <stdexcept>

#if defined(RENDERER_D3D11)
#include <renderer/system_d3d11.h>

using namespace Renderer;

struct ShaderCustom::Impl
{
	ID3D11VertexShader* vertexShader = nullptr;
	ID3D11PixelShader* pixelShader = nullptr;
	ID3D11Buffer* constantBuffer = nullptr;
	ID3D11InputLayout* inputLayout = nullptr;
	size_t customConstantBufferSize = 0;

	void* appliedConstantBuffer = nullptr;
	void* appliedCustomConstantBuffer = nullptr;
	bool forceDirty = false;
};

ShaderCustom::ShaderCustom(const Vertex::Layout& layout, const std::set<Vertex::Attribute::Type>& requiredAttribs,
	size_t customConstantBufferSize, const std::string& source)
{
    mImpl = std::make_unique<Impl>();
	mImpl->customConstantBufferSize = customConstantBufferSize;

	checkRequiredAttribs(requiredAttribs, layout);
	
	ID3DBlob* vertexShaderBlob;
	ID3DBlob* pixelShaderBlob;

	ID3DBlob* vertex_shader_error;
	ID3DBlob* pixel_shader_error;

	D3DCompile(source.c_str(), source.size(), NULL, NULL, NULL, "vs_main", "vs_4_0", 0, 0, &vertexShaderBlob, &vertex_shader_error);
	D3DCompile(source.c_str(), source.size(), NULL, NULL, NULL, "ps_main", "ps_4_0", 0, 0, &pixelShaderBlob, &pixel_shader_error);
	
	std::string vertex_shader_error_string = "";
	std::string pixel_shader_error_string = "";

	if (vertex_shader_error != nullptr)
		vertex_shader_error_string = std::string((char*)vertex_shader_error->GetBufferPointer(), vertex_shader_error->GetBufferSize());

	if (pixel_shader_error != nullptr)
		pixel_shader_error_string = std::string((char*)pixel_shader_error->GetBufferPointer(), pixel_shader_error->GetBufferSize());

	if (vertexShaderBlob == nullptr)
		throw std::runtime_error(vertex_shader_error_string);

	if (pixelShaderBlob == nullptr)
		throw std::runtime_error(pixel_shader_error_string);

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
		desc.ByteWidth = sizeof(mConstantBuffer) + customConstantBufferSize;
		desc.Usage = D3D11_USAGE_DYNAMIC;
		desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		desc.MiscFlags = 0;
		SystemD3D11::Device->CreateBuffer(&desc, NULL, &mImpl->constantBuffer);
	}

	mImpl->appliedConstantBuffer = malloc(sizeof(ConstantBuffer));

	if (customConstantBufferSize > 0)
		mImpl->appliedCustomConstantBuffer = malloc(customConstantBufferSize);
}

ShaderCustom::~ShaderCustom()
{
	free(mImpl->appliedConstantBuffer);

	if (mImpl->appliedCustomConstantBuffer != nullptr)
		free(mImpl->appliedCustomConstantBuffer);
}

void ShaderCustom::apply()
{
	SystemD3D11::Context->IASetInputLayout(mImpl->inputLayout);
	SystemD3D11::Context->VSSetShader(mImpl->vertexShader, nullptr, 0);
	SystemD3D11::Context->PSSetShader(mImpl->pixelShader, nullptr, 0);
	SystemD3D11::Context->VSSetConstantBuffers(0, 1, &mImpl->constantBuffer);
	SystemD3D11::Context->PSSetConstantBuffers(0, 1, &mImpl->constantBuffer);
	mImpl->forceDirty = true;
}

void ShaderCustom::update()
{
	bool dirty = mImpl->forceDirty;

	if (!dirty && memcmp(mImpl->appliedConstantBuffer, &mConstantBuffer, sizeof(ConstantBuffer)) != 0)
		dirty = true;

	if (mCustomConstantBuffer && !dirty && memcmp(mImpl->appliedCustomConstantBuffer, mCustomConstantBuffer, mImpl->customConstantBufferSize) != 0)
		dirty = true;

	if (!dirty)
		return;

	mImpl->forceDirty = false;
	memcpy(mImpl->appliedConstantBuffer, &mConstantBuffer, sizeof(ConstantBuffer));

	if (mCustomConstantBuffer)
		memcpy(mImpl->appliedCustomConstantBuffer, mCustomConstantBuffer, mImpl->customConstantBufferSize);

	D3D11_MAPPED_SUBRESOURCE resource;
	SystemD3D11::Context->Map(mImpl->constantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &resource);
	memcpy(resource.pData, &mConstantBuffer, sizeof(ConstantBuffer));
	
	if (mCustomConstantBuffer)
		memcpy((void*)((size_t)resource.pData + sizeof(ConstantBuffer)), mCustomConstantBuffer, mImpl->customConstantBufferSize);
	
	SystemD3D11::Context->Unmap(mImpl->constantBuffer, 0);
}
#endif