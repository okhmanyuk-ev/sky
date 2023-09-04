#pragma once

#include <string>
#include <glm/glm.hpp>

namespace sky::effects
{
	struct alignas(16) Sdf
	{
		glm::vec4 color = { 1.0f, 1.0f, 1.0f, 1.0f };
		float min_value = 0.0f;
		float max_value = 0.0f;
		float smooth_factor = 0.0f;

		static const std::string Shader;
	};
}