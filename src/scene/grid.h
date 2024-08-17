#pragma once

#include <scene/node.h>

namespace Scene
{
	class Grid : public Node
	{
	public:
		enum class Direction
		{
			RightDown,
			DownRight
		};

	public:
		Grid();

	protected:
		void update(Clock::Duration dTime) override;

	public:
		auto getDirection() const { return mDirection; }
		void setDirection(Direction value) { mDirection = value; }

		auto getMaxItemsInRow() const { return mMaxItemsInRow; }
		void setMaxItemsInRow(std::optional<size_t> value) { mMaxItemsInRow = value; }

	private:
		Direction mDirection = Direction::RightDown;
		std::optional<size_t> mMaxItemsInRow;
	};
}
