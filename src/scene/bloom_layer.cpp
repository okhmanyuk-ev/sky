#include "bloom_layer.h"

using namespace Scene;

void BloomLayer::onEvent(const Platform::System::ResizeEvent& e)
{
	RenderLayer<Node>::onEvent(e);
	mTargetsDirty = true;
}

void BloomLayer::postprocess(std::shared_ptr<Renderer::RenderTarget> render_texture)
{
	auto scale = PLATFORM->getScale();

	if (mPrevScale != scale)
	{
		mTargetsDirty = true;
		mPrevScale = scale;
	}

	if (mPrevDownscaleFactor != mDownscaleFactor)
	{
		mTargetsDirty = true;
		mPrevDownscaleFactor = mDownscaleFactor;
	}

	if (mTargetsDirty)
	{
		float factor = mDownscaleFactor * scale;

		mTargetWidth = glm::floor(render_texture->getWidth() / factor);
		mTargetHeight = glm::floor(render_texture->getHeight() / factor);

		mBlurTarget1 = std::make_shared<Renderer::RenderTarget>((int)mTargetWidth, (int)mTargetHeight);
		mBlurTarget2 = std::make_shared<Renderer::RenderTarget>((int)mTargetWidth, (int)mTargetHeight);

		mTargetsDirty = false;
	}

	GRAPHICS->pushSampler(Renderer::Sampler::Linear);
	GRAPHICS->pushBlendMode(Renderer::BlendStates::AlphaBlend);
	GRAPHICS->pushOrthoMatrix(1.0f, 1.0f);

	{
		GRAPHICS->pushRenderTarget(mBlurTarget1);
		GRAPHICS->pushViewport(mBlurTarget1);

		GRAPHICS->clear();
		GRAPHICS->drawSprite(render_texture, mBrightFilterShader);

		mBlurShader->setResolution({ mTargetWidth, mTargetHeight });

		for (int i = 0; i < mBlurPasses; i++)
		{
			mBlurShader->setDirection(Renderer::Shaders::Blur::Direction::Horizontal);

			GRAPHICS->pushRenderTarget(mBlurTarget2);
			GRAPHICS->clear();
			GRAPHICS->drawSprite(mBlurTarget1, mBlurShader);
			GRAPHICS->pop();
			
			mBlurShader->setDirection(Renderer::Shaders::Blur::Direction::Vertical);
			
			GRAPHICS->clear();
			GRAPHICS->drawSprite(mBlurTarget2, mBlurShader);
		}

		GRAPHICS->pop(2);
	}
	{
		mDefaultShader->setColor(glm::vec4(mGlowIntensity));
		
		GRAPHICS->pushRenderTarget(render_texture);
		GRAPHICS->pushViewport(render_texture);
	//	GRAPHICS->clear(); // uncomment to get only blur effect
		GRAPHICS->drawSprite(mBlurTarget1, mDefaultShader);
		GRAPHICS->pop(2);
	}

	GRAPHICS->pop(3);
}