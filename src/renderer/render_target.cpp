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
	if (!ENGINE) 
		return;
	
	if (!ENGINE->hasSystem<Renderer::System>())
		return;

	RENDERER->destroyRenderTarget(mRenderTargetHandler);
}
