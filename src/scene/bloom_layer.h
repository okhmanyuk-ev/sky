#pragma once

#include <scene/render_layer.h>
#include <skygfx/utils.h>

namespace Scene
{
	class BloomLayer : public RenderLayer<Node>
	{
	public:
		BloomLayer();

	protected:
		std::shared_ptr<skygfx::RenderTarget> postprocess(std::shared_ptr<skygfx::RenderTarget> render_texture) override;

	public:
		auto getIntensity() const { return mIntensity; }
		void setIntensity(float value) { mIntensity = value; }

		auto getBrightThreshold() const { return mBrightThreshold; }
		void setBrightThreshold(float value) { mBrightThreshold = value; }

	private:
		float mBrightThreshold = 1.0f;
		float mIntensity = 2.0f;
	};
}