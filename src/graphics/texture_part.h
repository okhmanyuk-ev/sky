#pragma  once

#include <graphics/tex_region.h>
#include <memory>
#include <skygfx/skygfx.h>

namespace Graphics
{
	class TexturePart
	{
	public:
		TexturePart() = default;
		TexturePart(std::shared_ptr<skygfx::Texture> texture, const TexRegion& region);

	public:
		operator std::shared_ptr<skygfx::Texture>() const { return mTexture; }

	public:
		auto getTexture() const { return mTexture; }
		const auto& getRegion() const { return mRegion; }

		float getWidth() const;
		float getHeight() const;

	private:
		std::shared_ptr<skygfx::Texture> mTexture = nullptr;
		TexRegion mRegion = {};
	};
}