#pragma once

#include <Scene/node.h>
#include <Scene/color.h>

namespace Scene
{
	class Circle : public Node, public Color
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

	private:
		int mSegments = 32;
		float mFill = 1.0f;
		float mBegin = 0.0f;
		float mEnd = 1.0f;
	};
}