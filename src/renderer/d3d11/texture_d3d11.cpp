#include <Renderer/texture.h>

#if defined(RENDERER_D3D11)
#include <Renderer/system_d3d11.h>

using namespace Renderer;

Texture::Texture(int width, int height, bool renderTarget) : mWidth(width), mHeight(height)
{
	D3D11_TEXTURE2D_DESC texture2d_desc = { };
	texture2d_desc.Width = mWidth;
	texture2d_desc.Height = mHeight;
	texture2d_desc.MipLevels = 1;
	texture2d_desc.ArraySize = 1;
	texture2d_desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	texture2d_desc.SampleDesc.Count = 1;
	texture2d_desc.SampleDesc.Quality = 0;
	texture2d_desc.Usage = D3D11_USAGE_DEFAULT;
	texture2d_desc.BindFlags = D3D11_BIND_SHADER_RESOURCE | (renderTarget ? D3D11_BIND_RENDER_TARGET : 0);
	texture2d_desc.CPUAccessFlags = 0;
	texture2d_desc.MiscFlags = 0;	
	SystemD3D11::Device->CreateTexture2D(&texture2d_desc, nullptr, &texture2d);

	D3D11_SHADER_RESOURCE_VIEW_DESC shader_resource_view_desc = { };
	shader_resource_view_desc.Format = texture2d_desc.Format;
	shader_resource_view_desc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	shader_resource_view_desc.Texture2D.MipLevels = texture2d_desc.MipLevels;
	shader_resource_view_desc.Texture2D.MostDetailedMip = 0;
	SystemD3D11::Device->CreateShaderResourceView(texture2d, &shader_resource_view_desc, &shader_resource_view);
}

Texture::Texture(int width, int height, int channels, void* data) : Texture(width, height)
{
	auto memPitch = width * channels;
	auto memSlicePitch = width * height * channels;
	SystemD3D11::Context->UpdateSubresource(texture2d, 0, nullptr, data, memPitch, memSlicePitch);
}

Texture::~Texture()
{
	shader_resource_view->Release();
	texture2d->Release();
}

void Texture::bindTexture() const
{
	SystemD3D11::Context->PSSetShaderResources(0, 1, &shader_resource_view);
}
#endif