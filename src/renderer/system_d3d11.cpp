#include "system_d3d11.h"

#if defined(RENDERER_D3D11)
#include <Platform/system_windows.h>

using namespace Renderer;

ID3D11Device* SystemD3D11::Device = nullptr;
ID3D11DeviceContext* SystemD3D11::Context = nullptr;

const std::unordered_map<Vertex::Attribute::Format, DXGI_FORMAT> SystemD3D11::Format = {
	{ Vertex::Attribute::Format::R32F, DXGI_FORMAT_R32_FLOAT },
	{ Vertex::Attribute::Format::R32G32F, DXGI_FORMAT_R32G32_FLOAT },
	{ Vertex::Attribute::Format::R32G32B32F, DXGI_FORMAT_R32G32B32_FLOAT },
	{ Vertex::Attribute::Format::R32G32B32A32F, DXGI_FORMAT_R32G32B32A32_FLOAT },
	{ Vertex::Attribute::Format::R8UN, DXGI_FORMAT_R8_UNORM },
	{ Vertex::Attribute::Format::R8G8UN, DXGI_FORMAT_R8G8_UNORM },
//	{ Vertex::Attribute::Format::R8G8B8UN, DXGI_FORMAT_R8G8B8_UNORM }, // TODO: fix
	{ Vertex::Attribute::Format::R8G8B8A8UN, DXGI_FORMAT_R8G8B8A8_UNORM }
};

const std::unordered_map<Vertex::Attribute::Type, std::string> SystemD3D11::Semantic = {
	{ Vertex::Attribute::Type::Position, "POSITION" },
	{ Vertex::Attribute::Type::Color, "COLOR" },
	{ Vertex::Attribute::Type::TexCoord, "TEXCOORD" },
	{ Vertex::Attribute::Type::Normal, "NORMAL" }
};

SystemD3D11::SystemD3D11()
{
	{
		DXGI_SWAP_CHAIN_DESC sd = {};
		sd.BufferCount = 2;
		sd.BufferDesc.Width = 0;
		sd.BufferDesc.Height = 0;
		sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		sd.BufferDesc.RefreshRate.Numerator = 60;
		sd.BufferDesc.RefreshRate.Denominator = 5;
		sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
		sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		sd.OutputWindow = Platform::SystemWindows::Window;
		sd.SampleDesc.Count = 1;
		sd.SampleDesc.Quality = 0;
		sd.Windowed = TRUE; // TODO: make false when fullscreen ?		
		sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

		D3D_FEATURE_LEVEL feature;
		std::vector<D3D_FEATURE_LEVEL> features = { D3D_FEATURE_LEVEL_11_0, };

		D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, NULL, features.data(),
			static_cast<UINT>(features.size()), D3D11_SDK_VERSION, &sd, &mSwapChain, &Device,
			&feature, &Context);
	}

	createRenderTarget();

	mResizeListener.setCallback([this] (const auto& e) {
		destroyRenderTarget();
		mSwapChain->ResizeBuffers(0, e.width, e.height, DXGI_FORMAT_R8G8B8A8_UNORM, 0);
		createRenderTarget();
	});

	setRenderTarget(nullptr);
	setBlendMode(BlendStates::NonPremultiplied);
}

SystemD3D11::~SystemD3D11()
{
	mSwapChain->Release();
	Context->Release();
	Device->Release();
}

void SystemD3D11::createRenderTarget() 
{
	D3D11_TEXTURE2D_DESC desc = {};
	desc.Width = PLATFORM->getWidth();
	desc.Height = PLATFORM->getHeight();
	desc.MipLevels = 1;
	desc.ArraySize = 1;
	desc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	desc.SampleDesc.Count = 1;
	desc.SampleDesc.Quality = 0;
	desc.Usage = D3D11_USAGE_DEFAULT;
	desc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	desc.CPUAccessFlags = 0;
	desc.MiscFlags = 0;
	Device->CreateTexture2D(&desc, nullptr, &depthStencilTexture);

	D3D11_DEPTH_STENCIL_VIEW_DESC descDSV = {};
	descDSV.Format = desc.Format;
	descDSV.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;

	Device->CreateDepthStencilView(depthStencilTexture, &descDSV, &depthStencilView);

	ID3D11Texture2D* pBackBuffer;
	mSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&pBackBuffer);
	Device->CreateRenderTargetView(pBackBuffer, nullptr, &renderTargetView);
	pBackBuffer->Release();
}

void SystemD3D11::destroyRenderTarget() 
{
	renderTargetView->Release();	
	depthStencilTexture->Release();
	depthStencilView->Release();
}

void SystemD3D11::setTopology(const Renderer::Topology& value) 
{
	const static std::unordered_map<Topology, D3D11_PRIMITIVE_TOPOLOGY> TopologyMap = {
		{ Topology::PointList, D3D11_PRIMITIVE_TOPOLOGY_POINTLIST },
		{ Topology::LineList, D3D11_PRIMITIVE_TOPOLOGY_LINELIST },
		{ Topology::LineStrip, D3D11_PRIMITIVE_TOPOLOGY_LINESTRIP },
		{ Topology::TriangleList, D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST },
		{ Topology::TriangleStrip, D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP }
	};

	Context->IASetPrimitiveTopology(TopologyMap.at(value));
}

void SystemD3D11::setViewport(const Viewport& value) 
{
	D3D11_VIEWPORT vp;
	vp.Width = value.size.x;
	vp.Height = value.size.y;
	vp.MinDepth = value.minDepth;
	vp.MaxDepth = value.maxDepth;
	vp.TopLeftX = value.position.x;
	vp.TopLeftY = value.position.y;
	Context->RSSetViewports(1, &vp);
	mViewport = value;
}

void SystemD3D11::setScissor(const Scissor& value) 
{
	mRasterizerState.scissorEnabled = true;

	D3D11_RECT ss;
	ss.left = static_cast<LONG>(value.position.x);
	ss.top = static_cast<LONG>(value.position.y);
	ss.right = static_cast<LONG>(value.position.x + value.size.x);
	ss.bottom = static_cast<LONG>(value.position.y + value.size.y);
	Context->RSSetScissorRects(1, &ss);
	mScissor = value;
}

void SystemD3D11::setScissor(std::nullptr_t value)
{
	mRasterizerState.scissorEnabled = false;
}

void SystemD3D11::setVertexBuffer(const Buffer& value) 
{
	D3D11_BUFFER_DESC desc = {};

	if (mD3D11VertexBuffer)
		mD3D11VertexBuffer->GetDesc(&desc);

	if (desc.ByteWidth < value.size)
	{
		if (mD3D11VertexBuffer)
			mD3D11VertexBuffer->Release();

		desc.Usage = D3D11_USAGE_DYNAMIC;
		desc.ByteWidth = static_cast<UINT>(value.size);
		desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		Device->CreateBuffer(&desc, nullptr, &mD3D11VertexBuffer);
	}

	D3D11_MAPPED_SUBRESOURCE resource;
	Context->Map(mD3D11VertexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &resource);
	memcpy(resource.pData, value.data, value.size);
	Context->Unmap(mD3D11VertexBuffer, 0);

	UINT stride = static_cast<UINT>(value.stride);
	UINT offset = static_cast<UINT>(0);

	Context->IASetVertexBuffers(0, 1, &mD3D11VertexBuffer, &stride, &offset);
}

void SystemD3D11::setIndexBuffer(const Buffer& value) 
{
	D3D11_BUFFER_DESC desc = {};

	if (mD3D11IndexBuffer)
		mD3D11IndexBuffer->GetDesc(&desc);

	if (desc.ByteWidth < value.size)
	{
		if (mD3D11IndexBuffer)
			mD3D11IndexBuffer->Release();

		desc.Usage = D3D11_USAGE_DYNAMIC;
		desc.ByteWidth = static_cast<UINT>(value.size);
		desc.BindFlags = D3D11_BIND_INDEX_BUFFER;
		desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		Device->CreateBuffer(&desc, nullptr, &mD3D11IndexBuffer);
	}

	D3D11_MAPPED_SUBRESOURCE resource;
	Context->Map(mD3D11IndexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &resource);
	memcpy(resource.pData, value.data, value.size);
	Context->Unmap(mD3D11IndexBuffer, 0);

	Context->IASetIndexBuffer(mD3D11IndexBuffer, value.stride == 2 ? DXGI_FORMAT_R16_UINT : DXGI_FORMAT_R32_UINT, 0);
}

void SystemD3D11::setTexture(std::shared_ptr<Texture> value)
{
	value->bindTexture();
}

void SystemD3D11::setRenderTarget(std::shared_ptr<RenderTarget> value)
{
	if (value == nullptr) 
	{
		Context->OMSetRenderTargets(1, &renderTargetView, depthStencilView);
		currentRenderTarget = nullptr;
	}
	else
	{
		value->bindRenderTarget();
		currentRenderTarget = value;
	}
}

void SystemD3D11::setShader(std::shared_ptr<Shader> value)
{
	mShader = value;
	mShaderDirty = true;
}

void SystemD3D11::setSampler(const Sampler& value) 
{
	if (mD3D11SamplerStates.count(value) == 0)
	{
		D3D11_SAMPLER_DESC desc = {};
		desc.Filter = value == Sampler::Linear ? D3D11_FILTER_MIN_MAG_MIP_LINEAR : D3D11_FILTER_MIN_MAG_MIP_POINT; // see D3D11_ENCODE_BASIC_FILTER
		desc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
		desc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
		desc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
		desc.MaxAnisotropy = D3D11_MAX_MAXANISOTROPY;
		desc.MipLODBias = 0.0f;
		desc.ComparisonFunc = D3D11_COMPARISON_NEVER;
		desc.MinLOD = 0.0f;
		desc.MaxLOD = FLT_MAX;
		Device->CreateSamplerState(&desc, &mD3D11SamplerStates[value]);
	}

	Context->PSSetSamplers(0, 1, &mD3D11SamplerStates.at(value));
}

void SystemD3D11::setDepthMode(const DepthMode& value) 
{
	mDepthStencilState.depthMode = value;
}

void SystemD3D11::setStencilMode(const StencilMode& value)
{
	mDepthStencilState.stencilMode = value;
}

void SystemD3D11::setCullMode(const CullMode& value) 
{
	mRasterizerState.cullMode = value;
}

void SystemD3D11::setBlendMode(const BlendMode& value)
{
	if (mD3D11BlendModes.count(value) == 0)
	{		
		const static std::unordered_map<Blend, D3D11_BLEND> BlendMap = {
			{ Blend::One, D3D11_BLEND_ONE },
			{ Blend::Zero, D3D11_BLEND_ZERO },
			{ Blend::SrcColor, D3D11_BLEND_SRC_COLOR },
			{ Blend::InvSrcColor, D3D11_BLEND_INV_SRC_COLOR },
			{ Blend::SrcAlpha, D3D11_BLEND_SRC_ALPHA },
			{ Blend::InvSrcAlpha, D3D11_BLEND_INV_SRC_ALPHA },
			{ Blend::DstColor, D3D11_BLEND_DEST_COLOR },
			{ Blend::InvDstColor, D3D11_BLEND_INV_DEST_COLOR },
			{ Blend::DstAlpha, D3D11_BLEND_DEST_ALPHA },
			{ Blend::InvDstAlpha, D3D11_BLEND_INV_DEST_ALPHA }
		};

		const static std::unordered_map<BlendFunction, D3D11_BLEND_OP> BlendOpMap = {
			{ BlendFunction::Add, D3D11_BLEND_OP_ADD },
			{ BlendFunction::Subtract, D3D11_BLEND_OP_SUBTRACT },
			{ BlendFunction::ReverseSubtract, D3D11_BLEND_OP_REV_SUBTRACT },
			{ BlendFunction::Min, D3D11_BLEND_OP_MIN },
			{ BlendFunction::Max, D3D11_BLEND_OP_MAX },
		};

		D3D11_BLEND_DESC desc = {};
		desc.AlphaToCoverageEnable = false;
		
		auto& blend = desc.RenderTarget[0];

		blend.RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
		blend.BlendEnable = true;

		blend.SrcBlend = BlendMap.at(value.colorSrcBlend);
		blend.DestBlend = BlendMap.at(value.colorDstBlend);
		blend.BlendOp = BlendOpMap.at(value.colorBlendFunction);

		blend.SrcBlendAlpha = BlendMap.at(value.alphaSrcBlend);
		blend.DestBlendAlpha = BlendMap.at(value.alphaDstBlend);
		blend.BlendOpAlpha = BlendOpMap.at(value.alphaBlendFunction);

		Device->CreateBlendState(&desc, &mD3D11BlendModes[value]);
	}

	const float blend_factor[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
	Context->OMSetBlendState(mD3D11BlendModes.at(value), blend_factor, 0xFFFFFFFF);
}

void SystemD3D11::clear(const glm::vec4& color) 
{
	if (currentRenderTarget == nullptr) 
	{
		Context->ClearRenderTargetView(renderTargetView, (float*)&color);
		Context->ClearDepthStencilView(depthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
	}
	else
	{
		currentRenderTarget->clearRenderTarget(color);
	}
}

void SystemD3D11::draw(size_t vertexCount, size_t vertexOffset)
{
	prepareForDrawing();
	Context->Draw((UINT)vertexCount, (UINT)vertexOffset);
}

void SystemD3D11::drawIndexed(size_t indexCount, size_t indexOffset, size_t vertexOffset)
{
	prepareForDrawing();
	Context->DrawIndexed((UINT)indexCount, (UINT)indexOffset, (INT)vertexOffset);
}

void SystemD3D11::present()
{
	mSwapChain->Present(mVsync ? 1 : 0, 0);
}

void SystemD3D11::prepareForDrawing()
{
	// shader

	assert(mShader != nullptr);

	if (mShaderDirty)
	{
		mShader->apply();
		mShaderDirty = false;
	}

	mShader->update();

	// rasterizer state

	if (!mRasterizerStateApplied || mAppliedRasterizerState != mRasterizerState)
	{
		setD3D11RasterizerState(mRasterizerState);
		mAppliedRasterizerState = mRasterizerState;
		mRasterizerStateApplied = true;
	}

	// depthstencil state

	if (!mDepthStencilStateApplied || mAppliedDepthStencilState != mDepthStencilState)
	{
		setD3D11DepthStencilState(mDepthStencilState);
		mAppliedDepthStencilState = mDepthStencilState;
		mDepthStencilStateApplied = true;
	}
}
void SystemD3D11::setD3D11RasterizerState(const RasterizerState& value)
{
	if (mD3D11RasterizerStates.count(value) == 0)
	{
		const static std::unordered_map<CullMode, D3D11_CULL_MODE> CullMap = {
			{ CullMode::None, D3D11_CULL_NONE },
			{ CullMode::Front, D3D11_CULL_FRONT },
			{ CullMode::Back, D3D11_CULL_BACK }
		};

		D3D11_RASTERIZER_DESC desc = {};
		desc.FillMode = D3D11_FILL_SOLID;
		desc.CullMode = CullMap.at(value.cullMode);
		desc.ScissorEnable = value.scissorEnabled;
		desc.DepthClipEnable = true;
		Device->CreateRasterizerState(&desc, &mD3D11RasterizerStates[value]);
	}

	Context->RSSetState(mD3D11RasterizerStates.at(value));
}

void SystemD3D11::setD3D11DepthStencilState(const DepthStencilState& value)
{
	if (mD3D11DepthStencilStates.count(value) == 0)
	{
		const static std::unordered_map<ComparisonFunc, D3D11_COMPARISON_FUNC> ComparisonFuncMap = {
			{ ComparisonFunc::Always, D3D11_COMPARISON_ALWAYS },
			{ ComparisonFunc::Never, D3D11_COMPARISON_NEVER },
			{ ComparisonFunc::Less, D3D11_COMPARISON_LESS },
			{ ComparisonFunc::Equal, D3D11_COMPARISON_EQUAL },
			{ ComparisonFunc::NotEqual, D3D11_COMPARISON_NOT_EQUAL },
			{ ComparisonFunc::LessEqual, D3D11_COMPARISON_LESS_EQUAL },
			{ ComparisonFunc::Greater, D3D11_COMPARISON_GREATER },
			{ ComparisonFunc::GreaterEqual, D3D11_COMPARISON_GREATER_EQUAL }
		};

		const static std::unordered_map<StencilOp, D3D11_STENCIL_OP> StencilOpMap = {
			{ StencilOp::Keep, D3D11_STENCIL_OP_KEEP },
			{ StencilOp::Zero, D3D11_STENCIL_OP_ZERO },
			{ StencilOp::Replace, D3D11_STENCIL_OP_REPLACE },
			{ StencilOp::IncrementSaturation, D3D11_STENCIL_OP_INCR_SAT },
			{ StencilOp::DecrementSaturation, D3D11_STENCIL_OP_DECR_SAT },
			{ StencilOp::Invert, D3D11_STENCIL_OP_INVERT },
			{ StencilOp::Increment, D3D11_STENCIL_OP_INCR },
			{ StencilOp::Decrement, D3D11_STENCIL_OP_DECR },
		};

		D3D11_DEPTH_STENCIL_DESC desc = {};
		desc.DepthEnable = value.depthMode.enabled;
		desc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
		desc.DepthFunc = ComparisonFuncMap.at(value.depthMode.func);

		desc.StencilEnable = value.stencilMode.enabled;
		desc.StencilReadMask = value.stencilMode.readMask;
		desc.StencilWriteMask = value.stencilMode.writeMask;

		desc.FrontFace.StencilDepthFailOp = StencilOpMap.at(value.stencilMode.depthFailOp);
		desc.FrontFace.StencilFailOp = StencilOpMap.at(value.stencilMode.failOp);
		desc.FrontFace.StencilFunc = ComparisonFuncMap.at(value.stencilMode.func);
		desc.FrontFace.StencilPassOp = StencilOpMap.at(value.stencilMode.passOp);

		desc.BackFace = desc.FrontFace;

		Device->CreateDepthStencilState(&desc, &mD3D11DepthStencilStates[value]);
	}

	Context->OMSetDepthStencilState(mD3D11DepthStencilStates.at(value), 1);
}

#endif