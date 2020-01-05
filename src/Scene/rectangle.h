#pragma once

#include <Scene/node.h>
#include <Scene/color.h>

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
		auto getCornerColor(Corner corner) const { return mCornerColors.at(corner); }
		void setCornerColor(Corner corner, const glm::vec4& value) { mCornerColors[corner] = value; }

	public:
		std::map<Corner, glm::vec4> mCornerColors = {
			{ Corner::TopLeft, { Graphics::Color::White, 1.0f } },
			{ Corner::TopRight, { Graphics::Color::White, 1.0f } },
			{ Corner::BottomLeft, { Graphics::Color::White, 1.0f } },
			{ Corner::BottomRight, { Graphics::Color::White, 1.0f } },
		};
	};
}