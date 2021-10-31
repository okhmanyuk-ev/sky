#include "color.h"

using namespace Scene;

Color::Color()
{
	//
}

Color::Color(const glm::vec3& color) : Color()
{
	setColor(color);
}

Color::Color(const glm::vec4& color) : Color()
{
	setColor(color);
}
