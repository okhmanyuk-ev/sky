#include <Renderer/render_target.h>
#include <cassert>

#if defined(RENDERER_GL44) || defined(RENDERER_GLES3)

using namespace Renderer;

RenderTarget::RenderTarget(int width, int height) : Texture(width, height, true)
{
	GLint last_fbo;
	GLint last_rbo;

	glGetIntegerv(GL_FRAMEBUFFER_BINDING, &last_fbo);
	glGetIntegerv(GL_RENDERBUFFER_BINDING, &last_rbo);

	glGenFramebuffers(1, &framebuffer);
	glGenRenderbuffers(1, &depth_stencil_renderbuffer);
	
	glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
	glBindRenderbuffer(GL_RENDERBUFFER, depth_stencil_renderbuffer);

	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture, 0);	
	
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, depth_stencil_renderbuffer);

	assert(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE);
	
	glBindFramebuffer(GL_FRAMEBUFFER, last_fbo);
	glBindRenderbuffer(GL_RENDERBUFFER, last_rbo);
}

RenderTarget::~RenderTarget() 
{
	glDeleteFramebuffers(1, &framebuffer);
	glDeleteRenderbuffers(1, &depth_stencil_renderbuffer);
}

void RenderTarget::bindRenderTarget() const
{
	glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
}

void RenderTarget::clearRenderTarget(const glm::vec4& color) const
{

}
#endif