#pragma once

#include <Scene/node.h>
#include <Scene/color.h>
#include <Graphics/tex_region.h>

namespace Scene
{
	class Sprite : public Node, public Color
	{
	protected:
		void update() override;
		void draw() override;

	public:
		auto getTexture() const { return mTexture; }
		void setTexture(const std::shared_ptr<Renderer::Texture>& value) { mTexture = value; }

		auto getTexRegion() const { return mTexRegion; }
		void setTexRegion(const Graphics::TexRegion& value) { mTexRegion = value; }
		
		auto getSampler() const { return mSampler; }
		void setSampler(Renderer::Sampler value) { mSampler = value; }

	private:
		std::shared_ptr<Renderer::Texture> mTexture;
		Graphics::TexRegion mTexRegion = {};
		Renderer::Sampler mSampler = Renderer::Sampler::Nearest;
	};
}
