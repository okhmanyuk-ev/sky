#pragma once

#include <Scene/render_layer.h>
#include <Renderer/render_target.h>
#include <Common/event_system.h>
#include <stack>
#include <Renderer/shader_blur.h>

namespace Scene
{
	class BloomLayer : public RenderLayer<Node>
	{
	protected:
		void event(const Platform::System::ResizeEvent& e) override;
		void postprocess(std::shared_ptr<Renderer::RenderTarget> render_texture) override;

	public:
		auto getBlurPasses() const { return mBlurPasses; }
		void setBlurPasses(int value) { mBlurPasses = value; }

		auto getGlowPasses() const { return mGlowPasses; }
		void setGlowPasses(int value) { mGlowPasses = value; }

		auto getDownscaleFactor() const { return mDownscaleFactor; }
		void setDownscaleFactor(float value) { mDownscaleFactor = value; mTargetsDirty = true; }

	private:
		std::shared_ptr<Renderer::RenderTarget> mBlurTarget1;
		std::shared_ptr<Renderer::RenderTarget> mBlurTarget2;
		bool mTargetsDirty = true;
		Renderer::ShaderBlur mBlurShader = Renderer::ShaderBlur(Renderer::Vertex::PositionTexture::Layout);
		float mTargetWidth = 0.0f;
		float mTargetHeight = 0.0f;
		int mBlurPasses = 1;
		int mGlowPasses = 1;
		float mDownscaleFactor = 4.0f;
		float mPrevScale = 0.0f;
	};
}