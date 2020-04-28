#include "sliced_sprite.h"

using namespace Scene;

void SlicedSprite::draw()
{
	if (mTexture == nullptr)
	{
		Node::draw();
		return;
	}

	auto model = glm::scale(getTransform(), { getSize(), 1.0f });
	
	GRAPHICS->pushSampler(getSampler());
	GRAPHICS->pushBlendMode(getBlendMode());
	GRAPHICS->drawSlicedSprite(mTexture, model, mCenterRegion, getSize(), getColor());
	GRAPHICS->pop(2);

	Node::draw();
}

void SlicedSprite::update()
{
	if (mTexture == nullptr)
	{
		Node::update();
		return;
	}
	
	auto width = static_cast<float>(mTexture->getWidth());
	auto height = static_cast<float>(mTexture->getHeight());

	if (getWidth() < width)
		setWidth(width);

	if (getHeight() < height)
		setHeight(height);

	Node::update();
}
