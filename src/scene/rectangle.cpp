#include "rectangle.h"

using namespace Scene;

RectangleComponent::RectangleComponent()
{
	setDrawCallback([this](Node& self) {
		if (getAlpha() <= 0.0f)
			return;

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

		auto absolute_size = self.getAbsoluteSize();
		auto model = glm::scale(self.getTransform(), { absolute_size, 1.0f });

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
				GRAPHICS->drawRoundedSlicedRectangle(top_left_color, absolute_size, mRounding, mAbsoluteRounding);
			}
			else
			{
				GRAPHICS->drawRoundedRectangle(top_left_color, top_right_color, bottom_left_color,
					bottom_right_color, absolute_size, mRounding, mAbsoluteRounding);
			}
		}
		else
		{
			GRAPHICS->drawRectangle(nullptr, top_left_color, top_right_color, bottom_left_color, bottom_right_color);
		}

		GRAPHICS->pop();
	});
}

Rectangle::Rectangle()
{
	mRectangleComponent = std::make_shared<RectangleComponent>();
	addComponent(mRectangleComponent);
}

glm::vec4 Rectangle::getColor() const
{
	return mRectangleComponent->getColor();
}

void Rectangle::setColor(const glm::vec4& value)
{
	mRectangleComponent->setColor(value);
}

void Rectangle::setColor(const glm::vec3& value)
{
	mRectangleComponent->setColor(value);
}

float Rectangle::getAlpha() const
{
	return mRectangleComponent->getAlpha();
}

void Rectangle::setAlpha(float value)
{
	mRectangleComponent->setAlpha(value);
}

std::shared_ptr<Color> Rectangle::getCornerColor(Corner corner) const
{
	return mRectangleComponent->getCornerColor(corner);
}

std::shared_ptr<Color> Rectangle::getEdgeColor(Edge edge) const
{
	return mRectangleComponent->getEdgeColor(edge);
}

float Rectangle::getRounding() const
{
	return mRectangleComponent->getRounding();
}

void Rectangle::setRounding(float value)
{
	mRectangleComponent->setRounding(value);
}

bool Rectangle::isAbsoluteRounding() const
{
	return mRectangleComponent->isAbsoluteRounding();
}

void Rectangle::setAbsoluteRounding(bool value)
{
	mRectangleComponent->setAbsoluteRounding(value);
}

bool Rectangle::isSlicedSpriteOptimizationEnabled() const
{
	return mRectangleComponent->isSlicedSpriteOptimizationEnabled();
}

void Rectangle::setSlicedSpriteOptimizationEnabled(bool value)
{
	mRectangleComponent->setSlicedSpriteOptimizationEnabled(value);
}

std::shared_ptr<RectangleComponent> Rectangle::getRectangleComponent() const
{
	return mRectangleComponent;
}
