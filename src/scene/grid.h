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

	protected:
		void update(sky::Duration dTime) override;

	public:
		auto getDirection() const { return mDirection; }
		void setDirection(Direction value) { mDirection = value; }

		auto getMaxItemsInRow() const { return mMaxItemsInRow; }
		void setMaxItemsInRow(std::optional<size_t> value) { mMaxItemsInRow = value; }

		auto getAlign() const { return mAlign; }
		void setAlign(float value) { mAlign = value; }

		bool isBreakToFitSize() const { return mBreakToFitSize; }
		void setBreakToFitSize(bool value) { mBreakToFitSize = value; }

		bool isUseAbsoluteSizeOfItems() const { return mUseAbsoluteSizeOfItems; }
		void setUseAbsoluteSizeOfItems(bool value) { mUseAbsoluteSizeOfItems = value; }

	private:
		Direction mDirection = Direction::RightDown;
		std::optional<size_t> mMaxItemsInRow;
		bool mBreakToFitSize = true;
		bool mUseAbsoluteSizeOfItems = true;
		float mAlign = 0.0f;
	};

	class Row : public Grid
	{
	public:
		Row();
	};

	class Column : public Grid
	{
	public:
		Column();
	};
}
