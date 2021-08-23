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

		enum class Edge
		{
			Top,
			Bottom,
			Left,
			Right
		};

	protected:
		void draw() override;

	public:
		auto getCornerColor(Corner corner) const { return mCornerColors.at(corner); }
		auto getEdgeColor(Edge edge) const { return mEdgeColors.at(edge); }

	private:
		std::map<Edge, std::shared_ptr<Color>> mEdgeColors = {
			{ Edge::Top, std::make_shared<Color>() },
			{ Edge::Bottom, std::make_shared<Color>() },
			{ Edge::Left, std::make_shared<Color>() },
			{ Edge::Right, std::make_shared<Color>() },
		};

		std::map<Corner, std::shared_ptr<Color>> mCornerColors = {
			{ Corner::TopLeft, std::make_shared<Color>() },
			{ Corner::TopRight, std::make_shared<Color>() },
			{ Corner::BottomLeft, std::make_shared<Color>() },
			{ Corner::BottomRight, std::make_shared<Color>() },
		};

	public:
		auto getRounding() const { return mRounding; }
		void setRounding(float value) { mRounding = value; }

		auto isAbsoluteRounding() const { return mAbsoluteRounding; }
		void setAbsoluteRounding(bool value) { mAbsoluteRounding = value; }

		auto isSlicedSpriteOptimizationEnabled() const { return mSlicedSpriteOptimizationEnabled; }
		void setSlicedSpriteOptimizationEnabled(bool value) { mSlicedSpriteOptimizationEnabled = value; }

	private:
		float mRounding = 0.0f;
		bool mAbsoluteRounding = false;
		bool mSlicedSpriteOptimizationEnabled = true;
	};
}