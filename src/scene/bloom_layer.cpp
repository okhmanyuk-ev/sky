#include "bloom_layer.h"
#include <fmt/format.h>

using namespace Scene;

std::shared_ptr<skygfx::RenderTarget> BloomLayer::postprocess(std::shared_ptr<skygfx::RenderTarget> render_texture)
{
	render_texture = RenderLayer<Node>::postprocess(render_texture);
	auto result = GRAPHICS->getRenderTarget(fmt::format("bloom_result_{}", (void*)this), render_texture->getWidth(), render_texture->getHeight());
	GRAPHICS->pushCleanState();
	GRAPHICS->pushRenderTarget(result);
	GRAPHICS->clear();
	mBloom.execute(*render_texture, *result);
	GRAPHICS->pop(2);
	return result;
}