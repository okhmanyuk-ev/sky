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

	struct alignas(16) Circle
	{
		glm::vec4 color = { 1.0f, 1.0f, 1.0f, 1.0f };
		glm::vec4 inner_color = { 1.0f, 1.0f, 1.0f, 1.0f };
		glm::vec4 outer_color = { 1.0f, 1.0f, 1.0f, 1.0f };
		float fill = 1.0f;
		float pie = 1.0f;

		static const std::string Shader;
	};

	struct alignas(16) Rounded
	{
		glm::vec4 color = { 1.0f, 1.0f, 1.0f, 1.0f };
		glm::vec2 size;
		float radius;

		static const std::string Shader;
	};
}