#include <scene/circle.h>

using namespace Scene;

void Circle::draw()
{
	auto model = glm::scale(getTransform(), { getSize(), 1.0f });
	auto color = getColor();
	auto inner_color = mInnerColor * color;
	auto outer_color = mOuterColor * color;
	GRAPHICS->pushBlendMode(getBlendMode());
	if (mSegments > 0)
	{
		GRAPHICS->drawSegmentedCircle(model, mSegments, inner_color, outer_color, mFill, mBegin, mEnd);
	}
	else
	{
		GRAPHICS->drawCircle(model, inner_color, outer_color, mFill, mBegin, mEnd);
	}
	GRAPHICS->pop();
	Node::draw();
}