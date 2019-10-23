#include "circle.h"

using namespace Scene;

void Circle::draw()
{
	auto model = glm::scale(getTransform(), { getSize(), 1.0f });

	if (mFilled)
		GRAPHICS->drawCircle(model, mSegments, getColor());
	else
		GRAPHICS->drawLineCircle(model, mSegments, getColor());

	Node::draw();
}