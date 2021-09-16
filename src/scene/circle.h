#pragma once

#include <scene/node.h>
#include <scene/color.h>
#include <scene/blend.h>

namespace Scene
{
	class Circle : public Node, public Color, public Blend
	{
	protected:
		void update(Clock::Duration dTime) override;
		void draw() override;

	public:
		auto getRadius() const { return mRadius; }
		void setRadius(float value) { mRadius = value; }

		auto getThickness() const { return mThickness; }
		void setThickness(float value) { mThickness = value; }

		auto getFill() const { return mFill; }
		void setFill(float value) { mFill = value; }

		auto getPie() const { return mPie; }
		void setPie(float value) { mPie = value; }

		auto getPiePivot() const { return mPiePivot; }
		void setPiePivot(float value) { mPiePivot = value; }

		auto getInnerColor() const { return mInnerColor; }
		void setInnerColor(const glm::vec4& value) { mInnerColor = value; }
		void setInnerColor(const glm::vec3& value) { mInnerColor = { value, mInnerColor.a }; }

		auto getOuterColor() const { return mOuterColor; }
		void setOuterColor(const glm::vec4& value) { mOuterColor = value; }
		void setOuterColor(const glm::vec3& value) { mOuterColor = { value, mOuterColor.a }; }

		void setDrawTextureWhenAvailable(bool value) { mDrawTextureWhenAvailable = value; }

	private:
		float mRadius = -1.0f;
		float mThickness = -1.0f; // working only when we have radius
		float mFill = 1.0f;
		float mPie = 1.0f;
		float mPiePivot = -1.0f;
		glm::vec4 mInnerColor = { Graphics::Color::White, 1.0f };
		glm::vec4 mOuterColor = { Graphics::Color::White, 1.0f };
		bool mDrawTextureWhenAvailable = true;
	};

	class SegmentedCircle : public Node, public Color, public Blend
	{
	protected:
		void draw() override;

	public:
		auto getSegments() const { return mSegments; }
		void setSegments(int value) { mSegments = value; }

		auto getFill() const { return mFill; }
		void setFill(float value) { mFill = value; }

		auto getInnerColor() const { return mInnerColor; }
		void setInnerColor(const glm::vec4& value) { mInnerColor = value; }
		void setInnerColor(const glm::vec3& value) { mInnerColor = { value, mInnerColor.a }; }

		auto getOuterColor() const { return mOuterColor; }
		void setOuterColor(const glm::vec4& value) { mOuterColor = value; }
		void setOuterColor(const glm::vec3& value) { mOuterColor = { value, mOuterColor.a }; }

	private:
		int mSegments = 32;
		float mFill = 1.0f;
		glm::vec4 mInnerColor = { Graphics::Color::White, 1.0f };
		glm::vec4 mOuterColor = { Graphics::Color::White, 1.0f };
	};
}