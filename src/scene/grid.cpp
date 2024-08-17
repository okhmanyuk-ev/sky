#include "grid.h"

using namespace Scene;

void Grid::update(Clock::Duration dTime)
{
	Node::update(dTime);

	glm::vec2 pos = { 0.0f, 0.0f };

	if (mDirection == Direction::RightDown)
	{
		float max_row_height = 0.0f;
		size_t items_in_row = 0;

		for (auto node : getNodes())
		{
			auto need_break = [&] {
				if (mMaxItemsInRow.has_value())
					return items_in_row >= mMaxItemsInRow.value();

				return pos.x + node->getWidth() > getAbsoluteWidth();
			}();

			if (need_break)
			{
				pos.x = 0.0f;
				pos.y += max_row_height;
				max_row_height = 0.0f;
				items_in_row = 0;
			}
			node->setPosition(pos);
			pos.x += node->getWidth();
			max_row_height = glm::max(max_row_height, node->getHeight());
			items_in_row++;
		}
	}
	else if (mDirection == Direction::DownRight)
	{
		float max_column_width = 0.0f;
		size_t items_in_column = 0;

		for (auto node : getNodes())
		{
			auto need_break = [&] {
				if (mMaxItemsInRow.has_value())
					return items_in_column >= mMaxItemsInRow.value();

				return pos.y + node->getHeight() > getAbsoluteHeight();
			}();

			if (need_break)
			{
				pos.x += max_column_width;
				pos.y = 0.0f;
				max_column_width = 0.0f;
				items_in_column = 0;
			}
			node->setPosition(pos);
			pos.y += node->getHeight();
			max_column_width = glm::max(max_column_width, node->getWidth());
			items_in_column++;
		}
	}
}

void Column::update(Clock::Duration dTime)
{
	Node::update(dTime);

	float y = 0.0f;

	for (auto node : getNodes())
	{
		node->setY(y);
		y += node->getHeight();
	}
}