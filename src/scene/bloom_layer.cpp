#include "bloom_layer.h"
#include <fmt/format.h>

using namespace Scene;

std::shared_ptr<Renderer::RenderTarget> BloomLayer::postprocess(std::shared_ptr<Renderer::RenderTarget> render_texture)
{
	render_texture = RenderLayer<Node>::postprocess(render_texture);

	float factor = mDownscaleFactor * PLATFORM->getScale();

	auto width = glm::floor(render_texture->getWidth() / factor);
	auto height = glm::floor(render_texture->getHeight() / factor);

	auto target1 = GRAPHICS->getRenderTarget(fmt::format("bloom_temp_1_{}", (void*)this), (int)width, (int)height);
	auto target2 = GRAPHICS->getRenderTarget(fmt::format("bloom_temp_2_{}", (void*)this), (int)width, (int)height);

	GRAPHICS->pushSampler(skygfx::Sampler::Linear);
	GRAPHICS->pushBlendMode(skygfx::BlendStates::AlphaBlend);
	GRAPHICS->pushOrthoMatrix(1.0f, 1.0f);

	GRAPHICS->pushRenderTarget(target1);

	GRAPHICS->clear();
	GRAPHICS->drawSprite(render_texture, mBrightFilterShader);

	mBlurShader->setResolution({ width, height});

	for (int i = 0; i < mBlurPasses; i++)
	{
		mBlurShader->setDirection(Renderer::Shaders::Blur::Direction::Horizontal);

		GRAPHICS->pushRenderTarget(target2);
		GRAPHICS->clear();
		GRAPHICS->drawSprite(target1, mBlurShader);
		GRAPHICS->pop();
			
		mBlurShader->setDirection(Renderer::Shaders::Blur::Direction::Vertical);
			
		GRAPHICS->clear();
		GRAPHICS->drawSprite(target2, mBlurShader);
	}

	GRAPHICS->pop();

	auto result = GRAPHICS->getRenderTarget(fmt::format("bloom_result_{}", (void*)this), render_texture->getWidth(), render_texture->getHeight());

	GRAPHICS->pushRenderTarget(result);
	GRAPHICS->clear();
	GRAPHICS->drawSprite(render_texture); // comment to get only blur effect
	GRAPHICS->drawSprite(target1, {}, glm::vec4(mGlowIntensity));
	
	GRAPHICS->pop(4);

	return result;
}