#include <renderer/render_target.h>
#include <cassert>
#include <renderer/system.h>

using namespace Renderer;

RenderTarget::RenderTarget(int width, int height) : Texture(width, height)
{
	mRenderTargetHandler = RENDERER->createRenderTarget(mHandler);
}

RenderTarget::~RenderTarget() 
{
	RENDERER->destroyRenderTarget(mRenderTargetHandler);
}
