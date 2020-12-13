#pragma once

#include <renderer/texture.h>
#include <renderer/low_level_api.h>
#include <glm/glm.hpp>

namespace Renderer
{
	class RenderTarget : public Texture
	{
		friend class SystemD3D11;
		friend class SystemGL;
	public:
		RenderTarget(int width, int height);
		~RenderTarget();

	private:
		void bindRenderTarget() const;
		void clearRenderTarget(const glm::vec4& color) const;
		void clearRenderTargetStencil() const;

#if defined(RENDERER_D3D11)
	private:
		ID3D11RenderTargetView* render_target_view;
		ID3D11Texture2D* depth_stencil_texture;
		ID3D11DepthStencilView* depth_stencil_view;
#elif defined(RENDERER_GL44) || defined(RENDERER_GLES3)
	private:
		GLuint framebuffer;
		GLuint depth_stencil_renderbuffer;
#endif
	};
}
