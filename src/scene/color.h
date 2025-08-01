#pragma once

#include <glm/glm.hpp>
#include <sky/color.h>

namespace Scene
{
	class Color
	{
	public:
		Color();
		Color(const glm::vec3& color);
		Color(const glm::vec4& color);
		virtual ~Color() = default;

	public:
		auto getColor() const { return mColor; }
		void setColor(const glm::vec4& value) { mColor = value; }
		void setColor(const glm::vec3& value) { mColor = { value, mColor.a }; }
		void setColor(sky::Color value) { setColor(sky::GetColor(value)); }

		auto getRGB() const { return glm::vec3{ mColor.r, mColor.g, mColor.b }; }
		void setRGB(const glm::vec3& value) { setColor(value); }

		float getAlpha() const { return mColor.a; }
		void setAlpha(float value) { mColor.a = value; }

	private:
		glm::vec4 mColor = sky::GetColor<glm::vec4>(sky::Color::White);
	};
}