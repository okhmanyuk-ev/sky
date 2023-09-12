#include "sliced_sprite.h"

using namespace Scene;

void SlicedSprite::draw()
{
	Node::draw();

	assert(mTexture != nullptr);
	
	if (getAlpha() <= 0.0f)
		return;

	auto model = glm::scale(getTransform(), { getAbsoluteSize(), 1.0f });
	
	GRAPHICS->pushSampler(getSampler());
	GRAPHICS->pushBlendMode(getBlendMode());
	GRAPHICS->pushModelMatrix(model);
	GRAPHICS->drawSlicedSprite(nullptr, mTexture, mCenterRegion, getAbsoluteSize(), std::nullopt, getColor());
	GRAPHICS->pop(3);
}

void SlicedSprite::update(Clock::Duration dTime)
{
	Node::update(dTime);

	assert(mTexture != nullptr);

	auto width = static_cast<float>(mTexture->getWidth());
	auto height = static_cast<float>(mTexture->getHeight());

	if (getWidth() < width)
		setWidth(width);

	if (getHeight() < height)
		setHeight(height);
}
