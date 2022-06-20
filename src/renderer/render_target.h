#pragma once

#include <renderer/texture.h>
#include <renderer/low_level_api.h>
#include <glm/glm.hpp>

namespace Renderer
{
	class RenderTarget : public Texture
	{
		friend class SystemGL;
		friend class SystemSkygfx;

	public:
		RenderTarget(uint32_t width, uint32_t height);
		~RenderTarget();

	private:
		struct RenderTargetImpl;
		std::unique_ptr<RenderTargetImpl> mRenderTargetImpl;
	};
}
