#pragma once

#include <Scene/node.h>
#include <Scene/color.h>

namespace Scene
{
	class Rectangle : public Node, public Color
	{
	protected:
		void draw() override;

	public:
		bool isFilled() const { return mFilled; }
		void setFilled(bool value) { mFilled = value; }

	private:
		bool mFilled = true;
	};
}