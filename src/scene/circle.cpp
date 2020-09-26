#include <scene/circle.h>

using namespace Scene;

void Circle::draw()
{
	Node::draw();

	if (getAlpha() <= 0.0f)
		return;

	auto model = glm::scale(getTransform(), { getSize(), 1.0f });
	auto color = getColor();
	auto inner_color = mInnerColor * color;
	auto outer_color = mOuterColor * color;
	GRAPHICS->pushBlendMode(getBlendMode());
	GRAPHICS->drawCircle(model, inner_color, outer_color, mFill, mPie);
	GRAPHICS->pop();
}

void SegmentedCircle::draw()
{
	Node::draw();

	if (getAlpha() <= 0.0f)
		return;

	auto model = glm::scale(getTransform(), { getSize(), 1.0f });
	auto color = getColor();
	auto inner_color = mInnerColor * color;
	auto outer_color = mOuterColor * color;
	GRAPHICS->pushBlendMode(getBlendMode());
	GRAPHICS->drawSegmentedCircle(model, mSegments, inner_color, outer_color, mFill);
	GRAPHICS->pop();
}