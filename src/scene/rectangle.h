#pragma once

#include <scene/node.h>
#include <scene/color.h>

namespace Scene
{
	class RectangleComponent : public ComponentNode::Component, public Color
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

	public:
		RectangleComponent();

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

	class Rectangle : public ComponentNode
	{
	public:
		using Corner = RectangleComponent::Corner;
		using Edge = RectangleComponent::Edge;

		Rectangle();

		glm::vec4 getColor() const;
		void setColor(const glm::vec4& value);
		void setColor(const glm::vec3& value);

		float getAlpha() const;
		void setAlpha(float value);

		std::shared_ptr<Color> getCornerColor(Corner corner) const;
		std::shared_ptr<Color> getEdgeColor(Edge edge) const;

		float getRounding() const;
		void setRounding(float value);

		bool isAbsoluteRounding() const;
		void setAbsoluteRounding(bool value);

		bool isSlicedSpriteOptimizationEnabled() const;
		void setSlicedSpriteOptimizationEnabled(bool value);

		std::shared_ptr<RectangleComponent> getRectangleComponent() const;

	private:
		std::shared_ptr<RectangleComponent> mRectangleComponent;
	};
}