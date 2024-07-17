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
		void update(Clock::Duration dTime) override;
		void draw() override;

	private:
		void applyTextureWidth();
		void applyTextureHeight();

	public:
		void applyTextureSize();

	public:
		auto getTexture() const { return mTexture; }
		void setTexture(std::shared_ptr<skygfx::Texture> value) { mTexture = value; }
		void setTexture(const Graphics::TexCell& value);

		auto getTexRegion() const { return mTexRegion; }
		void setTexRegion(const Graphics::TexRegion& value) { mTexRegion = value; }

		auto getTextureAddress() const { return mTextureAddress; }
		void setTextureAddress(skygfx::TextureAddress value) { mTextureAddress = value; }

		auto getEffect() const { return mEffect; }
		void setEffect(sky::effects::IEffect* value) { mEffect = value; }

	private:
		std::shared_ptr<skygfx::Texture> mTexture;
		Graphics::TexRegion mTexRegion = {};
		skygfx::TextureAddress mTextureAddress = DefaultTextureAddress;
		sky::effects::IEffect* mEffect = nullptr;

	public:
		struct DirectTexCoords
		{
			glm::vec2 top_left_uv = { 0.0f, 0.0f };
			glm::vec2 top_right_uv = { 0.0f, 0.0f };
			glm::vec2 bottom_left_uv = { 0.0f, 0.0f };
			glm::vec2 bottom_right_uv = { 0.0f, 0.0f };
		};

	public:
		auto getDirectTexCoords() const { return mDirectTexCoords; }
		void setDirectTexCoords(std::optional<DirectTexCoords> value) { mDirectTexCoords = value; }

	private:
		std::optional<DirectTexCoords> mDirectTexCoords;
	};
}
