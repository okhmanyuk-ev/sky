#pragma  once

#include <glm/glm.hpp>

namespace Graphics
{
	struct TexRegion
	{
		glm::vec2 pos = { 0.0f, 0.0f };
		glm::vec2 size = { 0.0f, 0.0f };
	};
}