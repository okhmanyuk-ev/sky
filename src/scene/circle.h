#pragma once

#include <scene/node.h>
#include <scene/color.h>
#include <scene/blend.h>

namespace Scene
{
	class Circle : public Node, public Color, public Blend
	{
	protected:
		void draw() override;

	public:
		auto getSegments() const { return mSegments; }
		void setSegments(int value) { mSegments = value; }

		auto getFill() const { return mFill; }
		void setFill(float value) { mFill = value; }

		auto getBegin() const { return mBegin; }
		void setBegin(float value) { mBegin = value; }

		auto getEnd() const { return mEnd; }
		void setEnd(float value) { mEnd = value; }

		auto getInnerColor() const { return mInnerColor; }
		void setInnerColor(const glm::vec4& value) { mInnerColor = value; }
		void setInnerColor(const glm::vec3& value) { mInnerColor = { value, mInnerColor.a }; }

		auto getOuterColor() const { return mOuterColor; }
		void setOuterColor(const glm::vec4& value) { mOuterColor = value; }
		void setOuterColor(const glm::vec3& value) { mOuterColor = { value, mOuterColor.a }; }

	private:
		int mSegments = 0;
		float mFill = 1.0f;
		float mBegin = 0.0f;
		float mEnd = 1.0f;
		glm::vec4 mInnerColor = { Graphics::Color::White, 1.0f };
		glm::vec4 mOuterColor = { Graphics::Color::White, 1.0f };
	};
}