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

		bool isFilled() const { return mFilled; }
		void setFilled(bool value) { mFilled = value; }

	private:
		int mSegments = 32;
		bool mFilled = true;
	};
}