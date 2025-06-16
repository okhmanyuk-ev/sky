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
}