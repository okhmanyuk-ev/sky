#pragma once

#include <glm/glm.hpp>
#include <graphics/color.h>

namespace Scene
{
	class Color
	{
	public:
		auto getColor() const { return mColor; }
		void setColor(const glm::vec4& value) { mColor = value; }
		void setColor(const glm::vec3& value) { mColor = { value, mColor.a }; }

		float getAlpha() const { return mColor.a; }
		void setAlpha(float value) { mColor.a = value; }

	private:
		glm::vec4 mColor = { Graphics::Color::White, 1.0f };
	};
}