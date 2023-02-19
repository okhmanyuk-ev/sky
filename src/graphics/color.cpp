#include "color.h"

namespace Graphics::Color
{
	glm::vec4 ToNormalized(uint8_t r, uint8_t g, uint8_t b, uint8_t a)
	{
		return { r / 255.0f, g / 255.0f, b / 255.0f, a / 255.0f };
	}

	glm::vec4 ToNormalized(uint32_t value)
	{
		auto r = ((uint8_t*)&value)[0];
		auto g = ((uint8_t*)&value)[1];
		auto b = ((uint8_t*)&value)[2];
		auto a = ((uint8_t*)&value)[3];
		return ToNormalized(r, g, b, a);
	}

	uint32_t ToUInt32(const glm::vec4& color)
	{
		uint32_t result;
		auto* bytes = (uint8_t*)&result;
		bytes[0] = static_cast<uint8_t>(color.r * 255.0f);
		bytes[1] = static_cast<uint8_t>(color.g * 255.0f);
		bytes[2] = static_cast<uint8_t>(color.b * 255.0f);
		bytes[3] = static_cast<uint8_t>(color.a * 255.0f);
		return result;
	}

	uint32_t ToUInt32(const glm::vec3& color)
	{
		return ToUInt32(glm::vec4{ color, 1.0f });
	}
}
