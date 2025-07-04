#include "sprite.h"

using namespace Scene;

void Sprite::draw()
{
	Node::draw();

	if (mTexture == nullptr)
		mTexture = DefaultTexture;

	if (getAlpha() <= 0.0f)
		return;

	auto model = glm::scale(getTransform(), { getAbsoluteSize(), 1.0f });
	auto color = getColor();

	GRAPHICS->pushSampler(getSampler());
	GRAPHICS->pushBlendMode(getBlendMode());
	GRAPHICS->pushTextureAddress(mTextureAddress);
	GRAPHICS->pushModelMatrix(model);
	GRAPHICS->drawTexturedRectangle(mEffect, mTexture, mTexRegion, color, color, color, color);
	GRAPHICS->pop(4);
}

void Sprite::update(sky::Duration dTime)
{
	Node::update(dTime);

	if (mTexture == nullptr)
		return;

	if (getAbsoluteWidth() <= 0.0f && getHorizontalStretch() <= 0.0f)
		applyTextureWidth();

	if (getAbsoluteHeight() <= 0.0f && getVerticalStretch() <= 0.0f)
		applyTextureHeight();
}

void Sprite::applyTextureWidth()
{
	setWidth(mTexRegion.has_value() ? mTexRegion->size.x : static_cast<float>(mTexture->getWidth()));
}

void Sprite::applyTextureHeight()
{
	setHeight(mTexRegion.has_value() ? mTexRegion->size.y : static_cast<float>(mTexture->getHeight()));
}

void Sprite::applyTextureSize()
{
	applyTextureWidth();
	applyTextureHeight();
}

void Sprite::setTexture(const Graphics::TexturePart& value)
{
	setTexture(value.getTexture());
	setTexRegion(value.getRegion());
}
