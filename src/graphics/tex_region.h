#pragma  once

#include <glm/glm.hpp>

namespace Graphics
{
	struct TexRegion
	{
		TexRegion(glm::vec2 pos, glm::vec2 size);

		glm::vec2 pos;
		glm::vec2 size;
	};

	// normalized texture coordinates of rectangle corners, unlike TexRegion can describe a rotated area
	struct TexCoords
	{
		glm::vec2 top_left;
		glm::vec2 top_right;
		glm::vec2 bottom_left;
		glm::vec2 bottom_right;
	};
}