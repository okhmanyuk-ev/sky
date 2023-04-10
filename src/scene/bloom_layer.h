#pragma once

#include <scene/render_layer.h>
#include <skygfx/utils.h>

namespace Scene
{
	class BloomLayer : public RenderLayer<Node>
	{
	protected:
		std::shared_ptr<skygfx::RenderTarget> postprocess(std::shared_ptr<skygfx::RenderTarget> render_texture) override;

	public:
		auto getIntensity() const { return mBloom.getIntensity(); }
		void setIntensity(float value) { mBloom.setIntensity(value); }

		auto getBrightThreshold() const { return mBloom.getBrightThreshold(); }
		void setBrightThreshold(float value) { mBloom.setBrightThreshold(value); }

	private:
		skygfx::utils::passes::Bloom mBloom;
	};
}