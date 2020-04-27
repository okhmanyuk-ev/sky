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

	Node::update();
}
