#pragma  once

#include <renderer/texture.h>
#include <graphics/tex_region.h>
#include <memory>

namespace Graphics
{
	class TexCell
	{
	public:
		TexCell() { }
		TexCell(std::shared_ptr<Renderer::Texture> texture, const TexRegion& region) :
			mTexture(texture), mRegion(region)
		{ }

	public:
		operator std::shared_ptr<Renderer::Texture>() const { return mTexture; }

	public:
		auto getTexture() const { return mTexture; }
		const auto& getRegion() const { return mRegion; }

		float getWidth() const 
		{ 
			if (mRegion.size.x > 0.0f)
				return mRegion.size.x;

			return (float)mTexture->getWidth();
		}

		float getHeight() const
		{
			if (mRegion.size.y > 0.0f)
				return mRegion.size.y;

			return (float)mTexture->getHeight();
		}

	private:
		std::shared_ptr<Renderer::Texture> mTexture = nullptr;
		TexRegion mRegion = {};
	};
}