#pragma once

#include <glm/glm.hpp>

namespace Scene
{
	class Transform
	{
	public:
		auto getSize() const { return mSize; }
		void setSize(const glm::vec2& value) { mSize = value; }

		auto getStretch() const { return mStretch; }
		void setStretch(const glm::vec2& value) { mStretch = value; }

		auto getPosition() const { return mPosition; }
		void setPosition(const glm::vec2& value) { mPosition = value; }

		auto getOrigin() const { return mOrigin; }
		void setOrigin(const glm::vec2& value) { mOrigin = value; }

		auto getMargin() const { return mMargin; }
		void setMargin(const glm::vec2& value) { mMargin = value; }

		auto getAnchor() const { return mAnchor; }
		void setAnchor(const glm::vec2& value) { mAnchor = value; }

		auto getPivot() const { return mPivot; }
		void setPivot(const glm::vec2& value) { mPivot = value; }

		auto getScale() const { return mScale; }
		void setScale(const glm::vec2& value) { mScale = value; }

		auto getRotation() const { return mRotation; }
		void setRotation(float value) { mRotation = value; }

	private:
		glm::vec2 mSize = { 0.0f, 0.0f };
		glm::vec2 mStretch = { -1.0f, -1.0f };
		glm::vec2 mPosition = { 0.0f, 0.0f };
		glm::vec2 mOrigin = { 0.0f, 0.0f };
		glm::vec2 mMargin = { 0.0f, 0.0f };
		glm::vec2 mAnchor = { 0.0f, 0.0f };
		glm::vec2 mPivot = { 0.0f, 0.0f };
		glm::vec2 mScale = { 1.0f, 1.0f };
		float mRotation = 0.0f; // radians

	public:
		auto getWidth() const { return mSize.x; }
		void setWidth(float value) { mSize.x = value; }

		auto getHeight() const { return mSize.y; }
		void setHeight(float value) { mSize.y = value; }

		auto getHorizontalSize() const { return mSize.x; }
		void setHorizontalSize(float value) { mSize.x = value; }

		auto getVerticalSize() const { return mSize.y; }
		void setVerticalSize(float value) { mSize.y = value; }

		void setSize(float value) { mSize = { value, value }; }

		auto getHorizontalStretch() const { return mStretch.x; }
		void setHorizontalStretch(float value) { mStretch.x = value; }

		auto getVerticalStretch() const { return mStretch.y; }
		void setVerticalStretch(float value) { mStretch.y = value; }

		void setStretch(float value) { mStretch = { value, value }; }

		auto getX() const { return mPosition.x; }
		void setX(float value) { mPosition.x = value; }
		
		auto getY() const { return mPosition.y; }
		void setY(float value) { mPosition.y = value; }

		auto getHorizontalPosition() const { return mPosition.x; }
		void setHorizontalPosition(float value) { mPosition.x = value; }

		auto getVerticalPosition() const { return mPosition.y; }
		void setVerticalPosition(float value) { mPosition.y = value; }

		auto getHorizontalOrigin() const { return mOrigin.x; }
		void setHorizontalOrigin(float value) { mOrigin.x = value; }

		auto getVerticalOrigin() const { return mOrigin.y; }
		void setVerticalOrigin(float value) { mOrigin.y = value; }

		auto getHorizontalMargin() const { return mMargin.x; }
		void setHorizontalMargin(float value) { mMargin.x = value; }

		auto getVerticalMargin() const { return mMargin.y; }
		void setVerticalMargin(float value) { mMargin.y = value; }

		auto getHorizontalAnchor() const { return mAnchor.x; }
		void setHorizontalAnchor(float value) { mAnchor.x = value; }

		auto getVerticalAnchor() const { return mAnchor.y; }
		void setVerticalAnchor(float value) { mAnchor.y = value; }

		auto getHorizontalPivot() const { return mPivot.x; }
		void setHorizontalPivot(float value) { mPivot.x = value; }
	
		auto getVerticalPivot() const { return mPivot.y; }
		void setVerticalPivot(float value) { mPivot.y = value; }
	
		auto getHorizontalScale() const { return mScale.x; }
		void setHorizontalScale(float value) { mScale.x = value; }

		auto getVerticalScale() const { return mScale.y; }
		void setVerticalScale(float value) { mScale.y = value; }

		void setScale(float value) { mScale = { value, value }; }
	};
}