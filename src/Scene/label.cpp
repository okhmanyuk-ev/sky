#include "label.h"

using namespace Scene;

void Label::update()
{
	if (mFont == nullptr || mFontSize <= 0.0f)
	{
		Node::update();
		return;
	}
	
	setWidth(mFont->getStringWidth(mText, mFontSize));
	setHeight(mFont->getStringHeight(mText, mFontSize));

	Node::update();
}

void Label::draw()
{
	if (mFont == nullptr || mFontSize <= 0.0f)
	{
		Node::draw();
		return;
	}

	auto scale = mFont->getScaleFactorForSize(mFontSize);

	auto model = glm::scale(getTransform(), { scale, scale, 1.0f });
	auto fontSize = mFontSize * std::fmax(getHorizontalScale(), getVerticalScale());

	GRAPHICS->draw(*mFont, mText, model, fontSize, getColor(), mOutlineThickness, mOutlineColor);
	
	Node::draw();
}