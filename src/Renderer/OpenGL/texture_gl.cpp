#include <Renderer/texture.h>

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
	GLint last_texture;
	glGetIntegerv(GL_TEXTURE_BINDING_2D, &last_texture);
	glBindTexture(GL_TEXTURE_2D, texture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
	glBindTexture(GL_TEXTURE_2D, last_texture);
}

Texture::~Texture()
{
	glDeleteTextures(1, &texture);
}

void Texture::bindTexture() const
{
	glBindTexture(GL_TEXTURE_2D, texture);
}
#endif