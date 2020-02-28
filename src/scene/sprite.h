#pragma once

#include <Scene/node.h>
#include <Scene/color.h>
#include <Scene/blend.h>
#include <Scene/sampler.h>
#include <Graphics/tex_region.h>

namespace Scene
{
	class Sprite : public Node, public Color, public Blend, public Sampler
	{
	protected:
		void update() override;
		void draw() override;

	public:
		auto getTexture() const { return mTexture; }
		void setTexture(std::shared_ptr<Renderer::Texture> value) { mTexture = value; }

		auto getTexRegion() const { return mTexRegion; }
		void setTexRegion(const Graphics::TexRegion& value) { mTexRegion = value; }

		auto getTextureAddress() const { return mTextureAddress; }
		void setTextureAddress(Renderer::TextureAddress value) { mTextureAddress = value; }

	private:
		std::shared_ptr<Renderer::Texture> mTexture;
		Graphics::TexRegion mTexRegion = {};
		Renderer::TextureAddress mTextureAddress = Renderer::TextureAddress::Wrap;
	};
}
