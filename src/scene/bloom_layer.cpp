#include "bloom_layer.h"

using namespace Scene;

void BloomLayer::event(const Platform::System::ResizeEvent& e)
{
	RenderLayer<Node>::event(e);
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

	{
		GRAPHICS->pushRenderTarget(mBlurTarget1);
		GRAPHICS->pushOrthoMatrix(1.0f, 1.0f);
		GRAPHICS->pushViewport(mBlurTarget1);

		GRAPHICS->clear();
		GRAPHICS->draw(render_texture, glm::mat4(1.0f), mBrightFilterShader);

		mBlurShader->setResolution({ mTargetWidth, mTargetHeight });

		for (int i = 0; i < mBlurPasses; i++)
		{
			mBlurShader->setDirection(Renderer::Shaders::Blur::Direction::Horizontal);

			GRAPHICS->pushRenderTarget(mBlurTarget2);
			GRAPHICS->clear();
			GRAPHICS->draw(mBlurTarget1, glm::mat4(1.0f), mBlurShader);
			GRAPHICS->pop();
			
			mBlurShader->setDirection(Renderer::Shaders::Blur::Direction::Vertical);
			
			GRAPHICS->clear();
			GRAPHICS->draw(mBlurTarget2, glm::mat4(1.0f), mBlurShader);
		}

		GRAPHICS->pop(3);
	}
	{
		mDefaultShader->setColor(glm::vec4(mGlowIntensity));
		
		GRAPHICS->pushRenderTarget(render_texture);
		GRAPHICS->pushOrthoMatrix(1.0f, 1.0f);
		GRAPHICS->pushViewport(render_texture);
	//	GRAPHICS->clear(); // uncomment to get only blur effect
		GRAPHICS->draw(mBlurTarget1, glm::mat4(1.0f), mDefaultShader);
		GRAPHICS->pop(3);
	}

	GRAPHICS->pop(2);
}