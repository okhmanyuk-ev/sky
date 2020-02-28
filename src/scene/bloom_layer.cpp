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
	{
		auto state = GRAPHICS->getCurrentState();
		state.renderTarget = mBlurTarget1;
		state.blendMode = Renderer::BlendStates::AlphaBlend;
		state.viewport = Renderer::Viewport(*mBlurTarget1);
		state.projectionMatrix = glm::orthoLH(0.0f, mTargetWidth, mTargetHeight, 0.0f, -1.0f, 1.0f);
		state.sampler = Renderer::Sampler::Linear;

		auto model = glm::scale(glm::mat4(1.0f), { mTargetWidth, mTargetHeight, 1.0f });

		GRAPHICS->push(state);
		GRAPHICS->clear();
		GRAPHICS->draw(render_texture, model, mBrightFilterShader);

		mBlurShader->setResolution({ mTargetWidth, mTargetHeight });

		for (int i = 0; i < mBlurPasses; i++)
		{
			mBlurShader->setDirection(Renderer::ShaderBlur::Direction::Horizontal);

			GRAPHICS->push(mBlurTarget2);
			GRAPHICS->clear();
			GRAPHICS->draw(mBlurTarget1, model, mBlurShader);
			GRAPHICS->pop();

			mBlurShader->setDirection(Renderer::ShaderBlur::Direction::Vertical);

			GRAPHICS->clear();
			GRAPHICS->draw(mBlurTarget2, model, mBlurShader);
		}

		GRAPHICS->pop();
	}
	{
		auto state = GRAPHICS->getCurrentState();
		state.renderTarget = render_texture;
		state.blendMode = Renderer::BlendStates::AlphaBlend;
		state.viewport = Renderer::Viewport(*render_texture);
		state.projectionMatrix = glm::orthoLH(0.0f, (float)render_texture->getWidth(), (float)render_texture->getHeight(), 0.0f, -1.0f, 1.0f);
		state.sampler = Renderer::Sampler::Linear;

		auto model = glm::scale(glm::mat4(1.0f), { render_texture->getWidth(), render_texture->getHeight(), 1.0f });

		mDefaultShader->setColor(glm::vec4(mGlowIntensity));

		GRAPHICS->push(state);
	//	GRAPHICS->clear(); // uncomment to get only blur effect
		GRAPHICS->draw(mBlurTarget1, model, mDefaultShader);
		GRAPHICS->pop();
	}
}