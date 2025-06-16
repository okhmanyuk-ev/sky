#include "texture_part.h"

using namespace Graphics;

TexturePart::TexturePart(std::shared_ptr<skygfx::Texture> texture, const TexRegion& region) :
	mTexture(texture), mRegion(region)
{
}

float TexturePart::getWidth() const
{
	if (mRegion.size.x > 0.0f)
		return mRegion.size.x;

	return (float)mTexture->getWidth();
}

float TexturePart::getHeight() const
{
	if (mRegion.size.y > 0.0f)
		return mRegion.size.y;

	return (float)mTexture->getHeight();
}
