#include "rectangle.h"
#include <renderer/shaders/rounded.h>

using namespace Scene;

void Rectangle::draw()
{
	Node::draw();

	if (getAlpha() <= 0.0f)
		return;

	auto model = glm::scale(getTransform(), { getAbsoluteSize(), 1.0f });

	auto color = getColor();

	auto top_left_color = color;
	auto top_right_color = color;
	auto bottom_left_color = color;
	auto bottom_right_color = color;

	top_left_color *= getCornerColor(Corner::TopLeft)->getColor();
	top_right_color *= getCornerColor(Corner::TopRight)->getColor();
	bottom_left_color *= getCornerColor(Corner::BottomLeft)->getColor();
	bottom_right_color *= getCornerColor(Corner::BottomRight)->getColor();

	auto edge_top_color = getEdgeColor(Edge::Top)->getColor();
	auto edge_bottom_color = getEdgeColor(Edge::Bottom)->getColor();
	auto edge_left_color = getEdgeColor(Edge::Left)->getColor();
	auto edge_right_color = getEdgeColor(Edge::Right)->getColor();

	top_left_color *= edge_top_color;
	top_left_color *= edge_left_color;

	top_right_color *= edge_top_color;
	top_right_color *= edge_right_color;

	bottom_left_color *= edge_bottom_color;
	bottom_left_color *= edge_left_color;

	bottom_right_color *= edge_bottom_color;
	bottom_right_color *= edge_right_color;

	GRAPHICS->pushModelMatrix(model);

	if (mRounding > 0.0f)
	{
		static auto colors = std::vector<glm::vec4>(4);

		colors[0] = top_left_color;
		colors[1] = top_right_color;
		colors[2] = bottom_left_color;
		colors[3] = bottom_right_color;

		bool one_color = std::all_of(colors.begin() + 1, colors.end(), 
			std::bind(std::equal_to<glm::vec4>(), std::placeholders::_1, colors.front()));

		if (one_color && mSlicedSpriteOptimizationEnabled)
		{
			GRAPHICS->drawRoundedSlicedRectangle(top_left_color, getAbsoluteSize(), mRounding, mAbsoluteRounding);
		}
		else
		{
			GRAPHICS->drawRoundedRectangle(top_left_color, top_right_color, bottom_left_color,
				bottom_right_color, getAbsoluteSize(), mRounding, mAbsoluteRounding);
		}
	}
	else
	{
		GRAPHICS->drawRectangle(top_left_color, top_right_color, bottom_left_color, bottom_right_color);
	}

	GRAPHICS->pop();
}