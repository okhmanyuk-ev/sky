#pragma once

#include <scene/render_layer.h>
#include <renderer/render_target.h>
#include <common/event_system.h>
#include <stack>
#include <renderer/shaders/blur.h>
#include <renderer/shaders/bright_filter.h>

namespace Scene
{
	class BloomLayer : public RenderLayer<Node>
	{
	protected:
		void onEvent(const Platform::System::ResizeEvent& e) override;
		void postprocess(std::shared_ptr<Renderer::RenderTarget> render_texture) override;

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
		std::shared_ptr<Renderer::RenderTarget> mBlurTarget1;
		std::shared_ptr<Renderer::RenderTarget> mBlurTarget2;
		bool mTargetsDirty = true;
		std::shared_ptr<Renderer::Shaders::Blur> mBlurShader = std::make_shared<Renderer::Shaders::Blur>(Renderer::Vertex::PositionColorTexture::Layout);
		std::shared_ptr<Renderer::Shaders::BrightFilter> mBrightFilterShader = std::make_shared<Renderer::Shaders::BrightFilter>(Renderer::Vertex::PositionColorTexture::Layout);
		std::shared_ptr<Renderer::Shaders::Default> mDefaultShader = std::make_shared<Renderer::Shaders::Default>(Renderer::Vertex::PositionColorTexture::Layout);
		float mTargetWidth = 0.0f;
		float mTargetHeight = 0.0f;
		int mBlurPasses = 1;
		float mGlowIntensity = 2.0f;
		float mDownscaleFactor = 4.0f;
		float mPrevScale = 0.0f;
		float mPrevDownscaleFactor = mDownscaleFactor;
	};
}