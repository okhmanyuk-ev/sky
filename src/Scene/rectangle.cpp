#include "rectangle.h"

using namespace Scene;

void Rectangle::draw()
{
	auto model = glm::scale(getTransform(), { getSize(), 1.0f });

	if (mFilled)
		GRAPHICS->drawRectangle(model, getColor());
	else
		GRAPHICS->drawLineRectangle(model, getColor());
	
	Node::draw();
}