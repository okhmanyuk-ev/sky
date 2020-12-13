#include <renderer/render_target.h>
#include <cassert>

#if defined(RENDERER_D3D11)
#include <renderer/system_d3d11.h>

using namespace Renderer;

RenderTarget::RenderTarget(int width, int height) : Texture(width, height, true)
{
	D3D11_RENDER_TARGET_VIEW_DESC render_target_view_desc = { };
	render_target_view_desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	render_target_view_desc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
	render_target_view_desc.Texture2D.MipSlice = 0;
	SystemD3D11::Device->CreateRenderTargetView(texture2d, &render_target_view_desc, &render_target_view);

	D3D11_TEXTURE2D_DESC texture2d_desc = { };
	texture2d_desc.Width = width;
	texture2d_desc.Height = height;
	texture2d_desc.MipLevels = 1;
	texture2d_desc.ArraySize = 1;
	texture2d_desc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	texture2d_desc.SampleDesc.Count = 1;
	texture2d_desc.SampleDesc.Quality = 0;
	texture2d_desc.Usage = D3D11_USAGE_DEFAULT;
	texture2d_desc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	texture2d_desc.CPUAccessFlags = 0;
	texture2d_desc.MiscFlags = 0;
	SystemD3D11::Device->CreateTexture2D(&texture2d_desc, nullptr, &depth_stencil_texture);

	D3D11_DEPTH_STENCIL_VIEW_DESC depth_stencil_view_desc = {};
	depth_stencil_view_desc.Format = texture2d_desc.Format;
	depth_stencil_view_desc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	SystemD3D11::Device->CreateDepthStencilView(depth_stencil_texture, &depth_stencil_view_desc, &depth_stencil_view);
}

RenderTarget::~RenderTarget() 
{
	render_target_view->Release();
	depth_stencil_texture->Release();
	depth_stencil_view->Release();
}

void RenderTarget::bindRenderTarget() const
{
	ID3D11ShaderResourceView* prev_shader_resource_view;
	SystemD3D11::Context->PSGetShaderResources(0, 1, &prev_shader_resource_view);

	if (prev_shader_resource_view == shader_resource_view)
	{
		ID3D11ShaderResourceView* null[] = { nullptr };
		SystemD3D11::Context->PSSetShaderResources(0, 1, null); // remove old shader view
	}
	
	if (prev_shader_resource_view)
		prev_shader_resource_view->Release(); // avoid memory leak

	SystemD3D11::Context->OMSetRenderTargets(1, &render_target_view, depth_stencil_view);
}

void RenderTarget::clearRenderTarget(const glm::vec4& color) const
{
	SystemD3D11::Context->ClearRenderTargetView(render_target_view, (float*)&color);
	SystemD3D11::Context->ClearDepthStencilView(depth_stencil_view, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
}

void RenderTarget::clearRenderTargetStencil() const
{
	SystemD3D11::Context->ClearDepthStencilView(depth_stencil_view, D3D11_CLEAR_STENCIL, 1.0f, 0);
}

#endif