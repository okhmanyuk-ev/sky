#pragma once

#include <scene/node.h>
#include <scene/color.h>

namespace Scene
{
	class Rectangle : public Node, public Color
	{
	public:
		enum class Corner
		{
			TopLeft,
			TopRight,
			BottomLeft,
			BottomRight
		};

	protected:
		void draw() override;

	public:
		void setHorizontalGradient(const glm::vec4& left, const glm::vec4& right);
		void setHorizontalGradient(const glm::vec3& left, const glm::vec3& right);

		void setVerticalGradient(const glm::vec4& top, const glm::vec4& bottom);
		void setVerticalGradient(const glm::vec3& top, const glm::vec3& bottom);

	public:
		auto getCornerColor(Corner corner) const { return mCornerColors.at(corner); }
		void setCornerColor(Corner corner, const glm::vec4& value) { mCornerColors[corner] = value; }
		void setCornerColor(Corner corner, const glm::vec3& value) { setCornerColor(corner, { value, 1.0f }); }

	public:
		std::map<Corner, glm::vec4> mCornerColors = {
			{ Corner::TopLeft, { Graphics::Color::White, 1.0f } },
			{ Corner::TopRight, { Graphics::Color::White, 1.0f } },
			{ Corner::BottomLeft, { Graphics::Color::White, 1.0f } },
			{ Corner::BottomRight, { Graphics::Color::White, 1.0f } },
		};
	};
}