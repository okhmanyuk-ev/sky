#include "bloom_layer.h"
#include <fmt/format.h>

using namespace Scene;

void BloomLayer::postprocess(std::shared_ptr<Renderer::RenderTarget> render_texture)
{
	float factor = mDownscaleFactor * PLATFORM->getScale();

	auto width = glm::floor(render_texture->getWidth() / factor);
	auto height = glm::floor(render_texture->getHeight() / factor);

	auto target1 = GRAPHICS->getRenderTarget(fmt::format("bloomlayer1_{}", (void*)this), (int)width, (int)height);
	auto target2 = GRAPHICS->getRenderTarget(fmt::format("bloomlayer2_{}", (void*)this), (int)width, (int)height);

	GRAPHICS->pushSampler(Renderer::Sampler::Linear);
	GRAPHICS->pushBlendMode(Renderer::BlendStates::AlphaBlend);
	GRAPHICS->pushOrthoMatrix(1.0f, 1.0f);

	{
		GRAPHICS->pushRenderTarget(target1);
		GRAPHICS->pushViewport(target1);

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

		GRAPHICS->pop(2);
	}
	{
		GRAPHICS->pushRenderTarget(render_texture);
		GRAPHICS->pushViewport(render_texture);
	//	GRAPHICS->clear(); // uncomment to get only blur effect
		GRAPHICS->drawSprite(target1, glm::mat4(1.0f), {}, glm::vec4(mGlowIntensity));
		GRAPHICS->pop(2);
	}

	GRAPHICS->pop(3);
}