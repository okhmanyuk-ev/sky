#include "grid.h"

using namespace Scene;

void Grid::update(Clock::Duration dTime)
{
	Node::update(dTime);

	if (mDirection == Direction::RightDown)
	{
		glm::vec2 row_size = { 0.0f, 0.0f };
		std::vector<Node*> row;
		float row_y = 0.0f;

		auto setup_row = [this](std::vector<Node*> items, glm::vec2 row_size, float row_y) {
			glm::vec2 pos = { getAbsoluteWidth() >= row_size.x ? (getAbsoluteWidth() - row_size.x) * mAlign : 0.0f, row_y };
			for (auto item : items)
			{
				item->setPosition(pos);
				pos.x += item->getAbsoluteWidth();
			}
		};

		for (auto node : getNodes())
		{
			auto item_size = node->getAbsoluteSize();

			bool row_completed = mMaxItemsInRow.has_value() ? row.size() >= mMaxItemsInRow.value() :
				row_size.x + item_size.x > getAbsoluteWidth();

			if (row_completed)
			{
				setup_row(row, row_size, row_y);
				row_y += row_size.y;
				row_size = { 0.0f, 0.0f };
				row.clear();
			}

			row.push_back(node.get());
			row_size.x += item_size.x;
			row_size.y = glm::max(row_size.y, item_size.y);
		}
		setup_row(row, row_size, row_y);
	}
	else if (mDirection == Direction::DownRight)
	{
		glm::vec2 column_size = { 0.0f, 0.0f };
		std::vector<Node*> column;
		float column_x = 0.0f;

		auto setup_column = [this](std::vector<Node*> items, glm::vec2 column_size, float column_x) {
			glm::vec2 pos = { column_x, getAbsoluteHeight() >= column_size.y ? (getAbsoluteHeight() - column_size.y) * mAlign : 0.0f };
			for (auto item : items)
			{
				item->setPosition(pos);
				pos.y += item->getAbsoluteHeight();
			}
		};

		for (auto node : getNodes())
		{
			auto item_size = node->getAbsoluteSize();

			bool column_completed = mMaxItemsInRow.has_value() ? column.size() >= mMaxItemsInRow.value() :
				column_size.y + item_size.y > getAbsoluteHeight();

			if (column_completed)
			{
				setup_column(column, column_size, column_x);
				column_x += column_size.x;
				column_size = { 0.0f, 0.0f };
				column.clear();
			}

			column.push_back(node.get());
			column_size.y += item_size.y;
			column_size.x = glm::max(column_size.x, item_size.x);
		}
		setup_column(column, column_size, column_x);
	}
}

void Row::update(Clock::Duration dTime)
{
	Node::update(dTime);

	float x = 0.0f;

	for (auto node : getNodes())
	{
		node->setX(x);
		x += mRowReadAbsoluteSize ? node->getAbsoluteWidth() : node->getWidth();
	}
}

void Column::update(Clock::Duration dTime)
{
	Node::update(dTime);

	float y = 0.0f;

	for (auto node : getNodes())
	{
		node->setY(y);
		y += mColumnReadAbsoluteSize ? node->getAbsoluteHeight() : node->getHeight();
	}
}
