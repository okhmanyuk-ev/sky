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
	auto tex_coords = getTexCoords();

	GRAPHICS->pushSampler(getSampler());
	GRAPHICS->pushBlendMode(getBlendMode());
	GRAPHICS->pushTextureAddress(mTextureAddress);
	GRAPHICS->pushModelMatrix(model);
	GRAPHICS->drawTexturedRectangle(mEffect, mTexture, tex_coords.top_left, tex_coords.top_right,
		tex_coords.bottom_left, tex_coords.bottom_right, color, color, color, color);
	GRAPHICS->pop(4);
}

Graphics::TexCoords Sprite::getTexCoords() const
{
	auto tl = glm::vec2{ 0.0f, 0.0f };
	auto br = glm::vec2{ 1.0f, 1.0f };

	if (mTexRegion.has_value())
	{
		auto texture_size = glm::vec2(mTexture->getWidth(), mTexture->getHeight());
		tl = mTexRegion->pos / texture_size;
		br = (mTexRegion->pos + mTexRegion->size) / texture_size;
	}

	return {
		.top_left = tl,
		.top_right = { br.x, tl.y },
		.bottom_left = { tl.x, br.y },
		.bottom_right = br
	};
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
