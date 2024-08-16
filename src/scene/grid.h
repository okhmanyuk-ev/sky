#pragma once

#include <scene/node.h>

namespace Scene
{
	class Grid : public Node
	{
	public:
		enum class Orientation
		{
			Vertical,
			Horizontal
		};

	public:
		Grid();

	protected:
		void update(Clock::Duration dTime) override;

	public:
		auto getOrientation() const { return mOrientation; }
		void setOrientation(Orientation value) { mOrientation = value; }

	private:
		Orientation mOrientation = Orientation::Vertical;
	};
}
