#include "blur_layer.h"

using namespace Scene;

void BlurLayer::event(const Platform::System::ResizeEvent& e)
{
	mTargetsDirty = true;
}

void BlurLayer::beginRender()
{
	if (mPrevScale != PLATFORM->getScale())
		mTargetsDirty = true;

	if (mTargetsDirty)
	{
		mSourceTarget.reset();
		mBlurTarget.reset();
		mBlurTarget2.reset();

		mPrevScale = PLATFORM->getScale();

		float factor = mDownscaleFactor * mPrevScale;

		auto w = PLATFORM->getWidth();
		auto h = PLATFORM->getHeight();

		mSourceTarget = std::make_shared<Renderer::RenderTarget>(w, h);
		mBlurTarget = std::make_shared<Renderer::RenderTarget>(int(w / factor), int(h / factor));
		mBlurTarget2 = std::make_shared<Renderer::RenderTarget>(int(w / factor), int(h / factor));

		mTargetsDirty = false;
	}

	Node::beginRender();

	GRAPHICS->push(mSourceTarget);
	GRAPHICS->push({ Renderer::Blend::SrcAlpha, Renderer::Blend::InvSrcAlpha, Renderer::Blend::One, Renderer::Blend::InvSrcAlpha });
	GRAPHICS->clear();
}

void BlurLayer::endRender()
{
	GRAPHICS->pop(2);

	if (!mPosteffectEnabled)
	{
		Node::endRender();
		auto model = glm::scale(glm::mat4(1.0f), { PLATFORM->getLogicalWidth(), PLATFORM->getLogicalHeight(), 1.0f });
		GRAPHICS->push(Renderer::BlendStates::AlphaBlend);
		GRAPHICS->draw(mSourceTarget, model);
		GRAPHICS->pop();
		return;
	}

	auto prev_batching = GRAPHICS->isBatching();

	GRAPHICS->setBatching(false);

	{
		auto model = glm::scale(glm::mat4(1.0f), { mBlurTarget->getWidth(), mBlurTarget->getHeight(), 1.0f });

		GRAPHICS->push(Renderer::BlendStates::AlphaBlend);
		GRAPHICS->push(Renderer::Viewport::FullRenderTarget(*mBlurTarget));
		GRAPHICS->push(mBlurTarget);
		GRAPHICS->pushProjectionMatrix(glm::orthoLH(0.0f, (float)mBlurTarget->getWidth(), (float)mBlurTarget->getHeight(), 0.0f, -1.0f, 1.0f));
		GRAPHICS->push(Renderer::Sampler::Linear);
		GRAPHICS->clear();
		GRAPHICS->draw(mSourceTarget, model);
		GRAPHICS->pop(5);
	}

	GRAPHICS->push(Graphics::System::State());

	mBlurShader.setResolution({ mBlurTarget->getWidth(), mBlurTarget->getHeight() });

	const std::vector<Renderer::Vertex::PositionTexture> Vertices = {
		{ { -1.0f, -1.0f, 0.0f }, { 0.0f, 1.0f } },
		{ { -1.0f,  1.0f, 0.0f }, { 0.0f, 0.0f } },
		{ {  1.0f,  1.0f, 0.0f }, { 1.0f, 0.0f } },
		{ {  1.0f, -1.0f, 0.0f }, { 1.0f, 1.0f } }
	};

	const std::vector<uint16_t> Indices = { 0, 1, 2, 0, 2, 3 };

	RENDERER->setCullMode(Renderer::CullMode::None);
	RENDERER->setDepthMode(Renderer::DepthMode::None);
	RENDERER->setBlendMode(Renderer::BlendStates::AlphaBlend);

	RENDERER->setTopology(Renderer::Topology::TriangleList);
	RENDERER->setShader(mBlurShader);
	RENDERER->setVertexBuffer(Vertices);
	RENDERER->setIndexBuffer(Indices);
	RENDERER->setViewport(Renderer::Viewport::FullRenderTarget(*mBlurTarget));
	RENDERER->setSampler(Renderer::Sampler::Linear);
	RENDERER->setScissor(nullptr);

	for (int i = 0; i < mBlurPasses; i++)
	{
		mBlurShader.setDirection(Renderer::ShaderBlur::Direction::Horizontal);

		RENDERER->setRenderTarget(*mBlurTarget2);
		RENDERER->setTexture(*mBlurTarget);
		RENDERER->clear();
		RENDERER->drawIndexed(Indices.size());

		mBlurShader.setDirection(Renderer::ShaderBlur::Direction::Vertical);

		RENDERER->setRenderTarget(*mBlurTarget);
		RENDERER->setTexture(*mBlurTarget2);
		RENDERER->clear();
		RENDERER->drawIndexed(Indices.size());
	}

	GRAPHICS->pop();

	{
		auto model = glm::scale(glm::mat4(1.0f), { PLATFORM->getLogicalWidth(), PLATFORM->getLogicalHeight(), 1.0f });

		GRAPHICS->push(Renderer::Sampler::Linear);
		GRAPHICS->push(Renderer::BlendStates::AlphaBlend);

		if (mBloom)
			GRAPHICS->draw(mSourceTarget, model);

		GRAPHICS->draw(mBlurTarget, model);
		
		for (int i = 0; i < mGlowSamples; i++)
			GRAPHICS->draw(mBlurTarget, model);

		GRAPHICS->pop(2);
	}

	GRAPHICS->setBatching(prev_batching);

	Node::endRender();
}