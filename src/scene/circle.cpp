#include "circle.h"

using namespace Scene;

void Circle::draw()
{
	auto model = glm::scale(getTransform(), { getSize(), 1.0f });
	GRAPHICS->drawCircle(model, mSegments, getColor(), mFill, mBegin, mEnd);
	Node::draw();
}