#include "sprite.h"

using namespace Scene;

void Sprite::draw()
{
	if (mTexture == nullptr)
	{
		Node::draw();
		return;
	}

	auto model = glm::scale(getTransform(), { getSize(), 1.0f });
	
	GRAPHICS->push(getSampler());
	GRAPHICS->push(getBlendMode());
	GRAPHICS->pushTextureAddress(mTextureAddress);
	GRAPHICS->draw(mTexture, model, mTexRegion, getColor());
	GRAPHICS->pop(3);

	Node::draw();
}

void Sprite::update()
{
	if (mTexture == nullptr)
	{
		Node::update();
		return;
	}

	if (getWidth() <= 0.0f)
	{
		if (mTexRegion.size.x > 0.0f)
			setWidth(mTexRegion.size.x);
		else
			setWidth(static_cast<float>(mTexture->getWidth()));
	}

	if (getHeight() <= 0.0f)
	{
		if (mTexRegion.size.y > 0.0f)
			setHeight(mTexRegion.size.y);
		else
			setHeight(static_cast<float>(mTexture->getHeight()));
	}

	Node::update();
}
