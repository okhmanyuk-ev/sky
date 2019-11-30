#include "label.h"

using namespace Scene;

void Label::update()
{
	if (mFont == nullptr || mFontSize <= 0.0f)
	{
		Node::update();
		return;
	}

	if (!mMultiline)
	{
		setWidth(mFont->getStringWidth(mText, mFontSize));
		setHeight(mFont->getStringHeight(mText, mFontSize));
	}

	Node::update();
}

void Label::draw()
{
	if (mFont == nullptr || mFontSize <= 0.0f || (mMultiline && getWidth() <= 0.0f))
	{
		Node::draw();
		return;
	}

	auto fontSize = mFontSize * glm::max(getHorizontalScale(), getVerticalScale());
	auto scale = mFont->getScaleFactorForSize(fontSize);
	auto model = glm::scale(getTransform(), { scale, scale, 1.0f });

	GRAPHICS->push(Renderer::Sampler::Linear);

	if (mMultiline)
	{
		auto height = GRAPHICS->drawMultilineString(*mFont, mText, model, fontSize, getWidth(),
			getColor(), mOutlineThickness, mOutlineColor);
		setHeight(height);
	}
	else
	{
		GRAPHICS->drawString(*mFont, mText, model, fontSize, getColor(), mOutlineThickness, mOutlineColor);
	}

	GRAPHICS->pop();

	Node::draw();
}