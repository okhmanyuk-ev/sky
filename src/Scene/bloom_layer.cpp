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

	if (mTargetsDirty)
	{
		float factor = mDownscaleFactor * scale;

		mTargetWidth = render_texture->getWidth() / factor;
		mTargetHeight = render_texture->getHeight() / factor;

		mBlurTarget1 = std::make_shared<Renderer::RenderTarget>(mTargetWidth, mTargetHeight);
		mBlurTarget2 = std::make_shared<Renderer::RenderTarget>(mTargetWidth, mTargetHeight);

		mTargetsDirty = false;
	}

	auto was_batching = GRAPHICS->isBatching();
	GRAPHICS->setBatching(false);

	{
		auto state = GRAPHICS->getCurrentState();
		state.renderTarget = mBlurTarget1;
		state.blendMode = Renderer::BlendStates::AlphaBlend;
		state.viewport = Renderer::Viewport::FullRenderTarget(*mBlurTarget1);
		state.projectionMatrix = glm::orthoLH(0.0f, mTargetWidth, mTargetHeight, 0.0f, -1.0f, 1.0f);
		state.sampler = Renderer::Sampler::Linear;

		auto model = glm::scale(glm::mat4(1.0f), { mTargetWidth, mTargetHeight, 1.0f });

		GRAPHICS->push(state);
		GRAPHICS->clear();
		GRAPHICS->draw(render_texture, model);
		GRAPHICS->pop();
	}
	{
		GRAPHICS->push(Graphics::System::State());

		mBlurShader.setResolution({ mTargetWidth, mTargetHeight });

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
		RENDERER->setViewport(Renderer::Viewport::FullRenderTarget(*mBlurTarget1));
		RENDERER->setSampler(Renderer::Sampler::Linear);
		RENDERER->setScissor(nullptr);

		for (int i = 0; i < mBlurPasses; i++)
		{
			mBlurShader.setDirection(Renderer::ShaderBlur::Direction::Horizontal);

			RENDERER->setRenderTarget(*mBlurTarget2);
			RENDERER->setTexture(*mBlurTarget1);
			RENDERER->clear();
			RENDERER->drawIndexed(Indices.size());

			mBlurShader.setDirection(Renderer::ShaderBlur::Direction::Vertical);

			RENDERER->setRenderTarget(*mBlurTarget1);
			RENDERER->setTexture(*mBlurTarget2);
			RENDERER->clear();
			RENDERER->drawIndexed(Indices.size());
		}

		GRAPHICS->pop();
	}
	{
		auto state = GRAPHICS->getCurrentState();
		state.renderTarget = render_texture;
		state.blendMode = Renderer::BlendStates::AlphaBlend;
		state.viewport = Renderer::Viewport::FullRenderTarget(*render_texture);
		state.projectionMatrix = glm::orthoLH(0.0f, (float)render_texture->getWidth(), (float)render_texture->getHeight(), 0.0f, -1.0f, 1.0f);
		state.sampler = Renderer::Sampler::Linear;

		auto model = glm::scale(glm::mat4(1.0f), { render_texture->getWidth(), render_texture->getHeight(), 1.0f });

		GRAPHICS->push(state);
		//GRAPHICS->clear(); // uncomment to get only blur effect

		for (int i = 0; i < mGlowPasses; i++)
			GRAPHICS->draw(mBlurTarget1, model);

		GRAPHICS->pop();
	}

	GRAPHICS->setBatching(was_batching);
}