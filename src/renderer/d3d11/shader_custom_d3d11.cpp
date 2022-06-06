#include <renderer/shader_custom.h>
#include <stdexcept>

#if defined(RENDERER_D3D11)
#include <renderer/system_d3d11.h>
#include <renderer/shader_compiler.h>

using namespace Renderer;

struct ShaderCross::Impl
{
	ID3D11VertexShader* vertexShader = nullptr;
	ID3D11PixelShader* pixelShader = nullptr;
	ID3D11InputLayout* inputLayout = nullptr;
};

ShaderCross::ShaderCross(const Vertex::Layout& layout, const std::string& vertex_code, const std::string& fragment_code)
{
	mImpl = std::make_unique<Impl>();
	
	ID3DBlob* vertexShaderBlob;
	ID3DBlob* pixelShaderBlob;

	ID3DBlob* vertex_shader_error;
	ID3DBlob* pixel_shader_error;

	std::vector<std::string> defines;
	AddLocationDefines(layout, defines);

	auto vertex_shader_spirv = Renderer::CompileGlslToSpirv(Renderer::ShaderStage::Vertex, vertex_code, defines);
	auto fragment_shader_spirv = Renderer::CompileGlslToSpirv(Renderer::ShaderStage::Fragment, fragment_code, defines);

	auto hlsl_vert = Renderer::CompileSpirvToHlsl(vertex_shader_spirv);
	auto hlsl_frag = Renderer::CompileSpirvToHlsl(fragment_shader_spirv);

	D3DCompile(hlsl_vert.c_str(), hlsl_vert.size(), NULL, NULL, NULL, "main", "vs_4_0", 0, 0, &vertexShaderBlob, &vertex_shader_error);
	D3DCompile(hlsl_frag.c_str(), hlsl_frag.size(), NULL, NULL, NULL, "main", "ps_4_0", 0, 0, &pixelShaderBlob, &pixel_shader_error);

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

	static const std::unordered_map<Vertex::Attribute::Format, DXGI_FORMAT> Format = {
		{ Vertex::Attribute::Format::R32F, DXGI_FORMAT_R32_FLOAT },
		{ Vertex::Attribute::Format::R32G32F, DXGI_FORMAT_R32G32_FLOAT },
		{ Vertex::Attribute::Format::R32G32B32F, DXGI_FORMAT_R32G32B32_FLOAT },
		{ Vertex::Attribute::Format::R32G32B32A32F, DXGI_FORMAT_R32G32B32A32_FLOAT },
		{ Vertex::Attribute::Format::R8UN, DXGI_FORMAT_R8_UNORM },
		{ Vertex::Attribute::Format::R8G8UN, DXGI_FORMAT_R8G8_UNORM },
		//	{ Vertex::Attribute::Format::R8G8B8UN, DXGI_FORMAT_R8G8B8_UNORM }, // TODO: fix
		{ Vertex::Attribute::Format::R8G8B8A8UN, DXGI_FORMAT_R8G8B8A8_UNORM }
	};

	std::vector<D3D11_INPUT_ELEMENT_DESC> input;

	UINT i = 0;

	for (auto& attrib : layout.attributes)
	{
		input.push_back({ "TEXCOORD", i, Format.at(attrib.format), 0,
			static_cast<UINT>(attrib.offset), D3D11_INPUT_PER_VERTEX_DATA, 0 });
		i++;
	}

	SystemD3D11::Device->CreateInputLayout(input.data(), static_cast<UINT>(input.size()), vertexShaderBlob->GetBufferPointer(), vertexShaderBlob->GetBufferSize(), &mImpl->inputLayout);
}

ShaderCross::~ShaderCross()
{
	//
}

void ShaderCross::apply()
{
	SystemD3D11::Context->IASetInputLayout(mImpl->inputLayout);
	SystemD3D11::Context->VSSetShader(mImpl->vertexShader, nullptr, 0);
	SystemD3D11::Context->PSSetShader(mImpl->pixelShader, nullptr, 0);
}

#endif