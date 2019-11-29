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

	auto scale = mFont->getScaleFactorForSize(mFontSize);
	auto fontSize = mFontSize * glm::max(getHorizontalScale(), getVerticalScale());

	if (mMultiline)
	{
		auto draw = [this, scale, fontSize](utf8_string::const_iterator begin, utf8_string::const_iterator end, float y) {
			auto model = glm::translate(getTransform(), { 0.0f, y, 0.0f });
			model = glm::scale(model, { scale, scale, 1.0f });
			GRAPHICS->draw(*mFont, begin, end, model, fontSize, getColor(), mOutlineThickness, mOutlineColor);
		};

		auto drawLine = [this, fontSize, draw](utf8_string::const_iterator begin, utf8_string::const_iterator end, float y) -> utf8_string::iterator {
			for (auto it = begin; it != end; ++it)
			{
				auto s_width = mFont->getStringWidth(begin, it, fontSize);
				
				if (s_width <= getWidth())
					continue;

				auto local_end = utf8_string::const_iterator(it - 1);

				auto rit_begin = utf8_string::const_reverse_iterator(local_end);
				auto rit_end = utf8_string::const_reverse_iterator(begin);
				
				for (auto rit = rit_begin; rit != rit_end; ++rit)
				{
					if (*rit != ' ')
						continue;

					rit--;

					draw(begin, rit.base(), y);
					return rit.base();
				}

				draw(begin, local_end, y);
				return local_end;
			}
			draw(begin, end, y);
			return end;
		};

		auto it = mText.begin();
		float y = 0.0f;

		while (it != mText.end())
		{
			it = drawLine(it, mText.end(), y);
			y += mFontSize;
		}

		setHeight(y);
	}
	else
	{
		auto model = glm::scale(getTransform(), { scale, scale, 1.0f });
		GRAPHICS->draw(*mFont, mText, model, fontSize, getColor(), mOutlineThickness, mOutlineColor);
	}

	Node::draw();
}