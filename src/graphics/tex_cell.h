#pragma  once

#include <renderer/texture.h>
#include <graphics/tex_region.h>
#include <memory>

namespace Graphics
{
	class TexCell
	{
	public:
		TexCell(std::shared_ptr<Renderer::Texture> texture, const TexRegion& region) :
			mTexture(texture), mRegion(region)
		{ }

	public:
		auto getTexture() const { return mTexture; }
		const auto& getRegion() const { return mRegion; }

	private:
		std::shared_ptr<Renderer::Texture> mTexture;
		TexRegion mRegion;
	};
}