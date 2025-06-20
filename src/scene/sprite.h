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
		inline static std::shared_ptr<skygfx::Texture> DefaultTexture = nullptr;
		inline static skygfx::TextureAddress DefaultTextureAddress = skygfx::TextureAddress::Clamp;

	protected:
		void update(sky::Duration dTime) override;
		void draw() override;

	private:
		void applyTextureWidth();
		void applyTextureHeight();

	public:
		void applyTextureSize();

	public:
		auto getTexture() const { return mTexture; }
		void setTexture(std::shared_ptr<skygfx::Texture> value) { mTexture = value; }
		void setTexture(const Graphics::TexturePart& value);

		const auto& getTexRegion() const { return mTexRegion; }
		void setTexRegion(const std::optional<Graphics::TexRegion>& value) { mTexRegion = value; }

		auto getTextureAddress() const { return mTextureAddress; }
		void setTextureAddress(skygfx::TextureAddress value) { mTextureAddress = value; }

		auto getEffect() const { return mEffect; }
		void setEffect(sky::effects::IEffect* value) { mEffect = value; }

	private:
		std::shared_ptr<skygfx::Texture> mTexture;
		std::optional<Graphics::TexRegion> mTexRegion;
		skygfx::TextureAddress mTextureAddress = DefaultTextureAddress;
		sky::effects::IEffect* mEffect = nullptr;
	};
}
