#include "system_d3d11.h"

#if defined(RENDERER_D3D11)
#include <platform/system_windows.h>

using namespace Renderer;

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

	setRenderTarget(nullptr);
	setBlendMode(BlendStates::NonPremultiplied);
}

SystemD3D11::~SystemD3D11()
{
	mSwapChain->Release();
	Context->Release();
	Device->Release();
}

void SystemD3D11::onEvent(const Platform::System::ResizeEvent& e)
{
	destroyRenderTarget();
	mSwapChain->ResizeBuffers(0, e.width, e.height, DXGI_FORMAT_R8G8B8A8_UNORM, 0);
	createRenderTarget();
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

	mRasterizerStateDirty = true;
}

void SystemD3D11::setScissor(std::nullptr_t value)
{
	mRasterizerState.scissorEnabled = false;
	mRasterizerStateDirty = true;
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
	if (value == nullptr)
		return;

	Context->PSSetShaderResources(0, 1, &mTextureDefs.at(value->mHandler).shader_resource_view);
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
		ID3D11ShaderResourceView* prev_shader_resource_view;
		Context->PSGetShaderResources(0, 1, &prev_shader_resource_view);
		
		if (prev_shader_resource_view == mTextureDefs.at(value->mHandler).shader_resource_view)
		{
			ID3D11ShaderResourceView* null[] = { nullptr };
			Context->PSSetShaderResources(0, 1, null); // remove old shader view
		}

		if (prev_shader_resource_view)
			prev_shader_resource_view->Release(); // avoid memory leak

		Context->OMSetRenderTargets(1, &mRenderTargetDefs.at(value->mRenderTargetHandler).render_target_view, mRenderTargetDefs.at(value->mRenderTargetHandler).depth_stencil_view);

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
	mSamplerState.sampler = value;
	mSamplerStateDirty = true;
}

void SystemD3D11::setDepthMode(const DepthMode& value) 
{
	mDepthStencilState.depthMode = value;
	mDepthStencilStateDirty = true;
}

void SystemD3D11::setStencilMode(const StencilMode& value)
{
	mDepthStencilState.stencilMode = value;
	mDepthStencilStateDirty = true;
}

void SystemD3D11::setCullMode(const CullMode& value) 
{
	mRasterizerState.cullMode = value;
	mRasterizerStateDirty = true;
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

		if (value.colorMask.red)
			blend.RenderTargetWriteMask |= D3D11_COLOR_WRITE_ENABLE_RED;

		if (value.colorMask.green)
			blend.RenderTargetWriteMask |= D3D11_COLOR_WRITE_ENABLE_GREEN;
		
		if (value.colorMask.blue)
			blend.RenderTargetWriteMask |= D3D11_COLOR_WRITE_ENABLE_BLUE;
		
		if (value.colorMask.alpha)
			blend.RenderTargetWriteMask |= D3D11_COLOR_WRITE_ENABLE_ALPHA;

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

void SystemD3D11::setTextureAddressMode(const TextureAddress& value)
{
	mSamplerState.textureAddress = value;
	mSamplerStateDirty = true;
}

void SystemD3D11::clear(std::optional<glm::vec4> color, std::optional<float> depth, std::optional<uint8_t> stencil)
{
	auto rtv = renderTargetView;
	auto dsv = depthStencilView;

	if (currentRenderTarget != nullptr)
	{
		auto rtd = mRenderTargetDefs.at(currentRenderTarget->mRenderTargetHandler);
		rtv = rtd.render_target_view;
		dsv = rtd.depth_stencil_view;
	}

	if (color.has_value())
	{
		Context->ClearRenderTargetView(rtv, (float*)&color.value());
	}

	if (depth.has_value() || stencil.has_value())
	{
		UINT flags = 0;

		if (depth.has_value())
			flags |= D3D11_CLEAR_DEPTH;

		if (stencil.has_value())
			flags |= D3D11_CLEAR_STENCIL;

		Context->ClearDepthStencilView(dsv, flags, depth.value_or(1.0f), stencil.value_or(0));
	}
}

void SystemD3D11::draw(size_t vertexCount, size_t vertexOffset)
{
	System::draw(vertexCount, vertexOffset);
	prepareForDrawing();
	Context->Draw((UINT)vertexCount, (UINT)vertexOffset);
}

void SystemD3D11::drawIndexed(size_t indexCount, size_t indexOffset, size_t vertexOffset)
{
	System::drawIndexed(indexCount, indexOffset, vertexOffset);
	prepareForDrawing();
	Context->DrawIndexed((UINT)indexCount, (UINT)indexOffset, (INT)vertexOffset);
}

void SystemD3D11::readPixels(const glm::ivec2& pos, const glm::ivec2& size, void* memory)
{
	if (size.x <= 0 || size.y <= 0)
		return;

	ID3D11Resource* resource = NULL;

	if (currentRenderTarget)
		mRenderTargetDefs.at(currentRenderTarget->mRenderTargetHandler).render_target_view->GetResource(&resource);
	else
		renderTargetView->GetResource(&resource);
	
	ID3D11Texture2D* texture = NULL;
	resource->QueryInterface(__uuidof(ID3D11Texture2D), (void**)&texture);

	D3D11_TEXTURE2D_DESC desc = { 0 };
	texture->GetDesc(&desc);
	desc.Usage = D3D11_USAGE_STAGING;
	desc.BindFlags = 0;
	desc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
	desc.MiscFlags = 0;

	auto back_w = desc.Width;
	auto back_h = desc.Height;

	desc.Width = size.x;
	desc.Height = size.y;

	ID3D11Texture2D* staging_texture = NULL;
	Device->CreateTexture2D(&desc, NULL, &staging_texture);

	auto src_x = (UINT)pos.x;
	auto src_y = (UINT)pos.y;
	auto src_w = (UINT)size.x;
	auto src_h = (UINT)size.y;

	UINT dst_x = 0;
	UINT dst_y = 0;

	if (pos.x < 0)
	{
		src_x = 0;
		if (-pos.x > size.x)
			src_w = 0;
		else
			src_w += pos.x;

		dst_x = -pos.x;
	}

	if (pos.y < 0)
	{
		src_y = 0;
		if (-pos.y > size.y)
			src_h = 0;
		else
			src_h += pos.y;

		dst_y = -pos.y;
	}

	D3D11_BOX box;
	box.left = src_x;
	box.right = src_x + src_w;
	box.top = src_y;
	box.bottom = src_y + src_h;
	box.front = 0;
	box.back = 1;

	if (pos.y < (int)back_h && pos.x < (int)back_w)
	{
		Context->CopySubresourceRegion(staging_texture, 0, dst_x, dst_y, 0, resource, 0, &box);

		D3D11_MAPPED_SUBRESOURCE mapped = { 0 };
		Context->Map(staging_texture, 0, D3D11_MAP_READ, 0, &mapped);
		
		auto dst_row_size = size.x * 4;

		if (mapped.RowPitch == dst_row_size)
		{
			memcpy(memory, mapped.pData, size.x * size.y * 4);
		}
		else
		{
			for (int i = 0; i < size.y; i++)
			{
				auto src_row = (void*)(size_t(mapped.pData) + size_t(i) * mapped.RowPitch);
				auto dst_row = (void*)(size_t(memory) + size_t(i) * dst_row_size);			
				memcpy(dst_row, src_row, dst_row_size);
			}
		}
		Context->Unmap(staging_texture, 0);
	}

	staging_texture->Release();
	resource->Release();
	texture->Release();
}

void SystemD3D11::readPixels(const glm::ivec2& pos, const glm::ivec2& size, std::shared_ptr<Renderer::Texture> dst_texture)
{
	auto dst_texture_handler = dst_texture->mHandler;
	assert(mTextureDefs.contains(dst_texture_handler));

	const auto& texture_def = mTextureDefs.at(dst_texture_handler);

	assert(texture_def.width == size.x);
	assert(texture_def.height == size.y);

	if (size.x <= 0 || size.y <= 0)
		return;

	ID3D11Resource* resource = NULL;

	if (currentRenderTarget)
		mRenderTargetDefs.at(currentRenderTarget->mRenderTargetHandler).render_target_view->GetResource(&resource);
	else
		renderTargetView->GetResource(&resource);

	ID3D11Texture2D* texture = NULL;
	resource->QueryInterface(__uuidof(ID3D11Texture2D), (void**)&texture);

	D3D11_TEXTURE2D_DESC desc = { 0 };
	texture->GetDesc(&desc);
	auto back_w = desc.Width;
	auto back_h = desc.Height;
	texture->Release();

	auto src_x = (UINT)pos.x;
	auto src_y = (UINT)pos.y;
	auto src_w = (UINT)size.x;
	auto src_h = (UINT)size.y;

	UINT dst_x = 0;
	UINT dst_y = 0;

	if (pos.x < 0)
	{
		src_x = 0;
		if (-pos.x > size.x)
			src_w = 0;
		else
			src_w += pos.x;

		dst_x = -pos.x;
	}

	if (pos.y < 0)
	{
		src_y = 0;
		if (-pos.y > size.y)
			src_h = 0;
		else
			src_h += pos.y;

		dst_y = -pos.y;
	}

	D3D11_BOX box;
	box.left = src_x;
	box.right = src_x + src_w;
	box.top = src_y;
	box.bottom = src_y + src_h;
	box.front = 0;
	box.back = 1;

	if (pos.y < (int)back_h && pos.x < (int)back_w)
	{
		Context->CopySubresourceRegion(texture_def.texture2d, 0, dst_x, dst_y, 0, resource, 0, &box);

		if (texture_def.mipmap)
			Context->GenerateMips(texture_def.shader_resource_view);
	}

	resource->Release();
}

void SystemD3D11::present()
{
	System::present();
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

	if (mRasterizerStateDirty)
	{
		setD3D11RasterizerState(mRasterizerState);
		mRasterizerStateDirty = false;
	}

	// depthstencil state

	if (mDepthStencilStateDirty)
	{
		setD3D11DepthStencilState(mDepthStencilState);
		mDepthStencilStateDirty = false;
	}

	// sampler state

	if (mSamplerStateDirty)
	{
		setD3D11SamplerState(mSamplerState);
		mSamplerStateDirty = false;
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

	Context->OMSetDepthStencilState(mD3D11DepthStencilStates.at(value), value.stencilMode.reference);
}

void SystemD3D11::setD3D11SamplerState(const SamplerState& value)
{
	if (mD3D11SamplerStates.count(value) == 0)
	{
		// TODO: see D3D11_ENCODE_BASIC_FILTER

		const static std::unordered_map<Sampler, D3D11_FILTER> SamplerMap = {
			{ Sampler::Linear, D3D11_FILTER_MIN_MAG_MIP_LINEAR  },
			{ Sampler::Nearest, D3D11_FILTER_MIN_MAG_MIP_POINT },
			{ Sampler::LinearMipmapLinear, D3D11_FILTER_MIN_MAG_MIP_LINEAR }
		};

		const static std::unordered_map<TextureAddress, D3D11_TEXTURE_ADDRESS_MODE> TextureAddressMap = {
			{ TextureAddress::Clamp, D3D11_TEXTURE_ADDRESS_CLAMP },
			{ TextureAddress::Wrap, D3D11_TEXTURE_ADDRESS_WRAP },
			{ TextureAddress::MirrorWrap, D3D11_TEXTURE_ADDRESS_MIRROR }
		};

		D3D11_SAMPLER_DESC desc = {};
		desc.Filter = SamplerMap.at(value.sampler);
		desc.AddressU = TextureAddressMap.at(value.textureAddress);
		desc.AddressV = TextureAddressMap.at(value.textureAddress);
		desc.AddressW = TextureAddressMap.at(value.textureAddress);
		desc.MaxAnisotropy = D3D11_MAX_MAXANISOTROPY;
		desc.MipLODBias = 0.0f;
		desc.ComparisonFunc = D3D11_COMPARISON_NEVER;
		desc.MinLOD = 0.0f;
		desc.MaxLOD = FLT_MAX;
		Device->CreateSamplerState(&desc, &mD3D11SamplerStates[value]);
	}

	Context->PSSetSamplers(0, 1, &mD3D11SamplerStates.at(value));
}

Texture::Handler SystemD3D11::createTexture(int width, int height, bool mipmap)
{
	assert(!mTextureDefs.contains(mTextureDefIndex));

	auto result = mTextureDefIndex;
	mTextureDefIndex += 1;

	auto& texture_def = mTextureDefs[result];
	texture_def.mipmap = mipmap;
	texture_def.width = width;
	texture_def.height = height;

	D3D11_TEXTURE2D_DESC texture2d_desc = { };
	texture2d_desc.Width = width;
	texture2d_desc.Height = height;

	if (!mipmap)
		texture2d_desc.MipLevels = 1;

	texture2d_desc.ArraySize = 1;
	texture2d_desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	texture2d_desc.SampleDesc.Count = 1;
	texture2d_desc.SampleDesc.Quality = 0;
	texture2d_desc.Usage = D3D11_USAGE_DEFAULT;
	texture2d_desc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
	texture2d_desc.CPUAccessFlags = 0;
	texture2d_desc.MiscFlags = D3D11_RESOURCE_MISC_GENERATE_MIPS; // TODO: only in mapmap mode ?
	Device->CreateTexture2D(&texture2d_desc, nullptr, &texture_def.texture2d);

	D3D11_SHADER_RESOURCE_VIEW_DESC shader_resource_view_desc = { };
	shader_resource_view_desc.Format = texture2d_desc.Format;
	shader_resource_view_desc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	shader_resource_view_desc.Texture2D.MipLevels = -1;
	shader_resource_view_desc.Texture2D.MostDetailedMip = 0;
	Device->CreateShaderResourceView(texture_def.texture2d, &shader_resource_view_desc, &texture_def.shader_resource_view);

	return result;
}

void SystemD3D11::destroyTexture(Texture::Handler value)
{
	assert(mTextureDefs.contains(value));

	auto& texture_def = mTextureDefs[value];

	if (texture_def.shader_resource_view)
		texture_def.shader_resource_view->Release();

	if (texture_def.texture2d)
		texture_def.texture2d->Release();

	mTextureDefs.erase(value);
}

void SystemD3D11::textureWritePixels(Texture::Handler texture, int width, int height, int channels, void* data)
{
	const auto& texture_def = mTextureDefs.at(texture);

	assert(width == texture_def.width);
	assert(height == texture_def.height);
	assert(data);

	if (!data)
		return;

	auto memPitch = width * channels;
	auto memSlicePitch = width * height * channels;
	Context->UpdateSubresource(texture_def.texture2d, 0, nullptr, data, memPitch, memSlicePitch);

	if (texture_def.mipmap)
		Context->GenerateMips(texture_def.shader_resource_view);
}

RenderTarget::RenderTargetHandler SystemD3D11::createRenderTarget(Texture::Handler texture)
{
	assert(!mRenderTargetDefs.contains(mRenderTargetDefIndex));
	assert(mTextureDefs.contains(texture));

	auto result = mRenderTargetDefIndex;
	mRenderTargetDefIndex += 1;

	const auto& texture_def = mTextureDefs.at(texture);
	auto& render_target_def = mRenderTargetDefs[result];

	D3D11_RENDER_TARGET_VIEW_DESC render_target_view_desc = { };
	render_target_view_desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	render_target_view_desc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
	render_target_view_desc.Texture2D.MipSlice = 0;
	Device->CreateRenderTargetView(texture_def.texture2d, &render_target_view_desc, &render_target_def.render_target_view);
	
	D3D11_TEXTURE2D_DESC texture2d_desc = { };
	texture2d_desc.Width = texture_def.width;
	texture2d_desc.Height = texture_def.height;
	texture2d_desc.MipLevels = 1;
	texture2d_desc.ArraySize = 1;
	texture2d_desc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	texture2d_desc.SampleDesc.Count = 1;
	texture2d_desc.SampleDesc.Quality = 0;
	texture2d_desc.Usage = D3D11_USAGE_DEFAULT;
	texture2d_desc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	texture2d_desc.CPUAccessFlags = 0;
	texture2d_desc.MiscFlags = 0;
	Device->CreateTexture2D(&texture2d_desc, nullptr, &render_target_def.depth_stencil_texture);

	D3D11_DEPTH_STENCIL_VIEW_DESC depth_stencil_view_desc = {};
	depth_stencil_view_desc.Format = texture2d_desc.Format;
	depth_stencil_view_desc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	Device->CreateDepthStencilView(render_target_def.depth_stencil_texture, &depth_stencil_view_desc, &render_target_def.depth_stencil_view);

	return result;
}

void SystemD3D11::destroyRenderTarget(RenderTarget::RenderTargetHandler value)
{
	assert(mRenderTargetDefs.contains(value));

	auto& render_target_def = mRenderTargetDefs[value];
	
	if (render_target_def.render_target_view)
		render_target_def.render_target_view->Release();
	
	if (render_target_def.depth_stencil_texture)
		render_target_def.depth_stencil_texture->Release();
	
	if (render_target_def.depth_stencil_view)
		render_target_def.depth_stencil_view->Release();

	mRenderTargetDefs.erase(value);
}

#endif