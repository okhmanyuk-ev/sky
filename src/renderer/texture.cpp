#include <renderer/texture.h>
#include <renderer/system.h>

using namespace Renderer;

Texture::Texture(int width, int height, bool mipmap) : 
	mWidth(width), 
	mHeight(height),
	mMipmap(mipmap)
{
	mHandler = RENDERER->createTexture(width, height, mipmap);
}

Texture::Texture(int width, int height, int channels, void* data, bool mipmap) : Texture(width, height, mipmap)
{
	writePixels(width, height, channels, data);
}

Texture::~Texture()
{
	RENDERER->destroyTexture(mHandler);
}

void Texture::writePixels(int width, int height, int channels, void* data)
{
	RENDERER->textureWritePixels(mHandler, width, height, channels, data);
}
