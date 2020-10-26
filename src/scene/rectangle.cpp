#include "rectangle.h"
#include <renderer/shaders/rounded.h>

using namespace Scene;

void Rectangle::draw()
{
	Node::draw();

	if (getAlpha() <= 0.0f)
		return;

	auto model = glm::scale(getTransform(), { getSize(), 1.0f });
	auto color = getColor();

	auto top_left_color = mCornerColors.at(Corner::TopLeft) * color;
	auto top_right_color = mCornerColors.at(Corner::TopRight) * color;
	auto bottom_left_color = mCornerColors.at(Corner::BottomLeft) * color;
	auto bottom_right_color = mCornerColors.at(Corner::BottomRight) * color;
	
	if (mRounding > 0.0f)
	{
		GRAPHICS->drawRoundedRectangle(model, top_left_color, top_right_color, bottom_left_color, 
			bottom_right_color, getSize(), mRounding, mAbsoluteRounding);
	}
	else
	{
		GRAPHICS->drawRectangle(model, top_left_color, top_right_color, bottom_left_color, bottom_right_color);
	}
}

void Rectangle::setHorizontalGradient(const glm::vec4& left, const glm::vec4& right)
{
	setCornerColor(Corner::TopLeft, left);
	setCornerColor(Corner::BottomLeft, left);
	setCornerColor(Corner::TopRight, right);
	setCornerColor(Corner::BottomRight, right);
}

void Rectangle::setHorizontalGradient(const glm::vec3& left, const glm::vec3& right)
{
	setHorizontalGradient({ left, 1.0f }, { right, 1.0f });
}

void Rectangle::setVerticalGradient(const glm::vec4& top, const glm::vec4& bottom)
{
	setCornerColor(Corner::TopLeft, top);
	setCornerColor(Corner::TopRight, top);
	setCornerColor(Corner::BottomLeft, bottom);
	setCornerColor(Corner::BottomRight, bottom);
}

void Rectangle::setVerticalGradient(const glm::vec3& top, const glm::vec3& bottom)
{
	setVerticalGradient({ top, 1.0f }, { bottom, 1.0f });
}
