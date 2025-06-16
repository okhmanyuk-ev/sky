#include "texture_part.h"

using namespace Graphics;

TexturePart::TexturePart(std::shared_ptr<skygfx::Texture> texture, std::optional<TexRegion> region) :
	mTexture(texture), mRegion(region)
{
}

float TexturePart::getWidth() const
{
	if (mRegion.has_value())
		return mRegion.value().size.x;

	return (float)mTexture->getWidth();
}

float TexturePart::getHeight() const
{
	if (mRegion.has_value())
		return mRegion.value().size.y;

	return (float)mTexture->getHeight();
}
