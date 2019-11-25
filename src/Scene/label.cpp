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
	if (mFont == nullptr || mFontSize <= 0.0f /*|| (mMultiline && getWidth() <= 0.0f)*/)
	{
		Node::draw();
		return;
	}

	if (mMultiline)
	{
		auto scale = mFont->getScaleFactorForSize(mFontSize);
		auto fontSize = mFontSize * glm::max(getHorizontalScale(), getVerticalScale());
		auto base_transform = getTransform();

		auto draw = [this, base_transform, scale, fontSize](const utf8_string& s, float y) {
			auto model = glm::translate(base_transform, { 0.0f, y, 0.0f });
			model = glm::scale(model, { scale, scale, 1.0f });
			GRAPHICS->draw(*mFont, s, model, fontSize, getColor(), mOutlineThickness, mOutlineColor);
		};

		utf8_string s = "";
		float y = 0.0f;
		for (int i = 0; i < mText.length(); i++)
		{
			auto width = mFont->getStringWidth(s + mText.at(i), mFontSize);

			if (width >= getWidth())
			{
				draw(s, y);				
				y += mFont->getStringHeight(s, mFontSize);
				s.clear();
			}

			s += mText.at(i);
		}
		draw(s, y);

		setHeight(y + mFont->getStringHeight(s));
	}
	else
	{
		auto scale = mFont->getScaleFactorForSize(mFontSize);

		auto model = glm::scale(getTransform(), { scale, scale, 1.0f });
		auto fontSize = mFontSize * glm::max(getHorizontalScale(), getVerticalScale());

		GRAPHICS->draw(*mFont, mText, model, fontSize, getColor(), mOutlineThickness, mOutlineColor);
	}

	Node::draw();
}