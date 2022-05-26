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
		struct RenderTargetImpl;
		std::unique_ptr<RenderTargetImpl> mRenderTargetImpl;
	};
}
