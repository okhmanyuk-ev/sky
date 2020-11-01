#include <renderer/texture.h>
#include <platform/system.h>

#if defined(RENDERER_GL44) || defined(RENDERER_GLES3)

using namespace Renderer;

Texture::Texture(int width, int height, bool renderTarget) : mWidth(width), mHeight(height)
{
	GLint last_texture;
	glGetIntegerv(GL_TEXTURE_BINDING_2D, &last_texture);

	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

	glBindTexture(GL_TEXTURE_2D, last_texture);
}

Texture::Texture(int width, int height, int channels, void* data) : Texture(width, height)
{
	writePixels(width, height, channels, data);
}

Texture::~Texture()
{
	glDeleteTextures(1, &texture);
}

void Texture::writePixels(int width, int height, int channels, void* data)
{
	GLint last_texture;
	glGetIntegerv(GL_TEXTURE_BINDING_2D, &last_texture);
	glBindTexture(GL_TEXTURE_2D, texture);

	// TODO: this method is bad because it changes texture width and height
	mWidth = width;
	mHeight = height;
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data); 
	
	// TODO: read abound pixel buffer objects (PBO)
	// https://www.roxlu.com/2014/048/fast-pixel-transfers-with-pixel-buffer-objects

	//glPixelStorei(GL_UNPACK_ALIGNMENT, 2);
	//glPixelStorei(GL_UNPACK_ROW_LENGTH, width);
	//glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	//glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, data);

	glBindTexture(GL_TEXTURE_2D, last_texture);
}

void Texture::bindTexture() const
{
	glBindTexture(GL_TEXTURE_2D, texture);
}

/*void Texture::writeFromBackbuffer()
{
	GLint last_texture;
	glGetIntegerv(GL_TEXTURE_BINDING_2D, &last_texture);
	glBindTexture(GL_TEXTURE_2D, texture);
	//glCopyTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, 0, 0, PLATFORM->getWidth(), PLATFORM->getHeight(), 0);
	glBindTexture(GL_TEXTURE_2D, last_texture);
}*/

#endif