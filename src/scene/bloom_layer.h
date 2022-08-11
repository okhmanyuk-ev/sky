#pragma once

#include <scene/render_layer.h>
#include <common/event_system.h>
#include <stack>
#include <renderer/shaders/blur.h>
#include <renderer/shaders/bright_filter.h>

namespace Scene
{
	class BloomLayer : public RenderLayer<Node>
	{
	protected:
		std::shared_ptr<skygfx::RenderTarget> postprocess(std::shared_ptr<skygfx::RenderTarget> render_texture) override;

	public:
		auto getBlurPasses() const { return mBlurPasses; }
		void setBlurPasses(int value) { mBlurPasses = value; }

		auto getGlowIntensity() const { return mGlowIntensity; }
		void setGlowIntensity(float value) { mGlowIntensity = value; }

		auto getDownscaleFactor() const { return mDownscaleFactor; }
		void setDownscaleFactor(float value) { mDownscaleFactor = value; }

		auto getBrightThreshold() const { return mBrightFilterShader->getThreshold(); }
		void setBrightThreshold(float value) { mBrightFilterShader->setThreshold(value); }

	private:
		std::shared_ptr<Renderer::Shaders::Blur> mBlurShader = std::make_shared<Renderer::Shaders::Blur>(skygfx::Vertex::PositionColorTexture::Layout);
		std::shared_ptr<Renderer::Shaders::BrightFilter> mBrightFilterShader = std::make_shared<Renderer::Shaders::BrightFilter>(skygfx::Vertex::PositionColorTexture::Layout);
		int mBlurPasses = 1;
		float mGlowIntensity = 2.0f;
		float mDownscaleFactor = 4.0f;
	};
}