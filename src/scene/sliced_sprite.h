#pragma once

#include <scene/node.h>
#include <scene/color.h>
#include <scene/blend.h>
#include <scene/sampler.h>
#include <graphics/tex_region.h>

namespace Scene
{
	class SlicedSprite : public Node, public Color, public Blend, public Sampler
	{
	protected:
		void update() override;
		void draw() override;

	public:
		auto getTexture() const { return mTexture; }
		void setTexture(std::shared_ptr<Renderer::Texture> value) { mTexture = value; }

		auto getCenterRegion() const { return mCenterRegion; }
		void setCenterRegion(const Graphics::TexRegion& value) { mCenterRegion = value; }

	private:
		std::shared_ptr<Renderer::Texture> mTexture = nullptr;
		Graphics::TexRegion mCenterRegion = {};
	};
}
