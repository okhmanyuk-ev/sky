#pragma once

#include <Scene/node.h>
#include <Scene/color.h>
#include <Renderer/render_target.h>
#include <Common/event_system.h>
#include <stack>
#include <Renderer/shader_blur.h>

namespace Scene
{
	class BlurLayer : public Node, public Color,
		public Common::EventSystem::Listenable<Platform::System::ResizeEvent>
	{
	private:
		void event(const Platform::System::ResizeEvent& e) override;
	
	protected:
		void beginRender() override;
		void endRender() override;

	private:
		std::shared_ptr<Renderer::RenderTarget> mSourceTarget;
		std::shared_ptr<Renderer::RenderTarget> mBlurTarget;
		std::shared_ptr<Renderer::RenderTarget> mBlurTarget2;
		Renderer::ShaderBlur mBlurShader = Renderer::ShaderBlur(Renderer::Vertex::PositionTexture::Layout);

	public:
		auto isPosteffectEnabled() const { return mPosteffectEnabled; }
		void setPosteffectEnabled(bool value) { mPosteffectEnabled = value; }

		auto getGlowSamples() const { return mGlowSamples; }
		void setGlowSamples(int value) { mGlowSamples = value; }

		auto getBlurPasses() const { return mBlurPasses; }
		void setBlurPasses(int value) { mBlurPasses = value; }

		auto getDownscaleFactor() const { return mDownscaleFactor; }
		void setDownscaleFactor(float value) { mDownscaleFactor = value; mTargetsDirty = true; }

		auto isBloom() const { return mBloom; }
		void setBloom(bool value) { mBloom = value; }

	private:
		bool  mPosteffectEnabled = true;
		bool mTargetsDirty = true;
		int mBlurPasses = 1;
		int mGlowSamples = 0;
		float mDownscaleFactor = 1.0f;
		float mPrevScale = PLATFORM->getScale();
		bool mBloom = false;
	};
}