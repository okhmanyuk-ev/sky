#include "sprite.h"

using namespace Scene;

void Sprite::draw()
{
	Node::draw();

	assert(mTexture); // you should setup Sprite::DefaultTexture
	
	if (mTexture == nullptr)
		return;
	
	if (getAlpha() <= 0.0f)
		return;
	
	auto model = glm::scale(getTransform(), { getAbsoluteSize(), 1.0f });
	
	GRAPHICS->pushSampler(getSampler());
	GRAPHICS->pushBlendMode(getBlendMode());
	GRAPHICS->pushTextureAddress(mTextureAddress);
	GRAPHICS->pushMipmapBias(mMipmapBias);
	GRAPHICS->pushModelMatrix(model);
	GRAPHICS->drawSprite(mTexture, mTexRegion, getColor(), mShader);
	GRAPHICS->pop(5);
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