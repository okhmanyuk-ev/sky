#include <renderer/texture.h>
#include <platform/system.h>

#if defined(RENDERER_GL44) || defined(RENDERER_GLES3)

using namespace Renderer;

Texture::Texture(int width, int height, bool mipmap) : 
	mWidth(width), 
	mHeight(height),
	mMipmap(mipmap)
{
	GLint last_texture;
	glGetIntegerv(GL_TEXTURE_BINDING_2D, &last_texture);

	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

	glBindTexture(GL_TEXTURE_2D, last_texture);
}

Texture::Texture(int width, int height, int channels, void* data, bool mipmap) : Texture(width, height, mipmap)
{
	writePixels(width, height, channels, data);
}

Texture::~Texture()
{
	glDeleteTextures(1, &texture);
}

void Texture::writePixels(int width, int height, int channels, void* data)
{
	assert(mWidth == width);
	assert(mHeight == height);
	assert(data);

	GLint last_texture;
	glGetIntegerv(GL_TEXTURE_BINDING_2D, &last_texture);
	glBindTexture(GL_TEXTURE_2D, texture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);

	if (mMipmap)
		glGenerateMipmap(GL_TEXTURE_2D);

	glBindTexture(GL_TEXTURE_2D, last_texture);

	// TODO: read abound pixel buffer objects (PBO)
	// https://www.roxlu.com/2014/048/fast-pixel-transfers-with-pixel-buffer-objects
}

#endif