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

	GRAPHICS->pushSampler(getSampler());
	GRAPHICS->pushBlendMode(getBlendMode());
	GRAPHICS->pushTextureAddress(mTextureAddress);
	GRAPHICS->pushModelMatrix(model);

	auto color = getColor();

	if (mDirectTexCoords.has_value())
	{
		auto coords = mDirectTexCoords.value();

		GRAPHICS->drawTexturedRectangle(mEffect, mTexture, coords.top_left_uv, coords.top_right_uv,
			coords.bottom_left_uv, coords.bottom_right_uv, color, color, color, color);
	}
	else
	{
		GRAPHICS->drawTexturedRectangle(mEffect, mTexture, mTexRegion, color, color, color, color);
	}
	GRAPHICS->pop(4);
}

void Sprite::update(Clock::Duration dTime)
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
	if (mTexRegion.size.x > 0.0f)
		setWidth(mTexRegion.size.x);
	else
		setWidth(static_cast<float>(mTexture->getWidth()));
}

void Sprite::applyTextureHeight()
{
	if (mTexRegion.size.y > 0.0f)
		setHeight(mTexRegion.size.y);
	else
		setHeight(static_cast<float>(mTexture->getHeight()));
}

void Sprite::applyTextureSize()
{
	applyTextureWidth();
	applyTextureHeight();
}

void Sprite::setTexture(const Graphics::TexCell& value)
{
	setTexture(value.getTexture());
	setTexRegion(value.getRegion());
}