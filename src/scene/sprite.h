#pragma once

#include <scene/node.h>
#include <scene/color.h>
#include <scene/blend.h>
#include <scene/sampler.h>
#include <graphics/all.h>

namespace Scene
{
	class Sprite : public Node, public Color, public Blend, public Sampler
	{
	public:
		inline static Renderer::TextureAddress DefaultTextureAddress = Renderer::TextureAddress::Clamp;

	protected:
		void update() override;
		void draw() override;

	private:
		void applyTextureWidth();
		void applyTextureHeight();

	public:
		void applyTextureSize();

	public:
		auto getTexture() const { return mTexture; }
		void setTexture(std::shared_ptr<Renderer::Texture> value) { mTexture = value; }
		void setTexture(const Graphics::TexCell& value);

		auto getTexRegion() const { return mTexRegion; }
		void setTexRegion(const Graphics::TexRegion& value) { mTexRegion = value; }

		auto getTextureAddress() const { return mTextureAddress; }
		void setTextureAddress(Renderer::TextureAddress value) { mTextureAddress = value; }

		auto getShader() const { return mShader; }
		void setShader(std::shared_ptr<Renderer::ShaderMatrices> value) { mShader = value; }

	private:
		std::shared_ptr<Renderer::Texture> mTexture = nullptr;
		Graphics::TexRegion mTexRegion = {};
		Renderer::TextureAddress mTextureAddress = DefaultTextureAddress;
		std::shared_ptr<Renderer::ShaderMatrices> mShader = nullptr;
	};
}
