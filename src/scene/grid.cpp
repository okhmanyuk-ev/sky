#include "grid.h"

using namespace Scene;

void Grid::update(sky::Duration dTime)
{
	Node::update(dTime);

	auto is_need_break_group = [this](size_t group_items_count, float current_size, float max_size) {
		if (mMaxItemsInRow.has_value() && group_items_count >= mMaxItemsInRow.value())
			return true;

		if (mBreakToFitSize && current_size > max_size)
			return true;

		return false;
	};

	auto get_item_size = [this](const Node& item) {
		return mUseAbsoluteSizeOfItems ? item.getAbsoluteSize() : item.getSize();
	};

	glm::vec2 group_size = { 0.0f, 0.0f };
	std::vector<Node*> group;
	float group_pos = 0.0f;

	if (mDirection == Direction::RightDown)
	{
		auto setup_row = [this, get_item_size](const std::vector<Node*>& items, glm::vec2 row_size, float row_y) {
			glm::vec2 pos = { getAbsoluteWidth() >= row_size.x ? (getAbsoluteWidth() - row_size.x) * mAlign : 0.0f, row_y };
			for (auto item : items)
			{
				item->setPosition(pos);
				pos.x += get_item_size(*item).x;
			}
		};
		for (auto item : getNodes())
		{
			auto item_size = get_item_size(*item);
			bool break_group = is_need_break_group(group.size(), group_size.x + item_size.x, getAbsoluteWidth());
			if (break_group)
			{
				setup_row(group, group_size, group_pos);
				group_pos += group_size.y;
				group_size = { 0.0f, 0.0f };
				group.clear();
			}
			group.push_back(item.get());
			group_size.x += item_size.x;
			group_size.y = glm::max(group_size.y, item_size.y);
		}
		setup_row(group, group_size, group_pos);
	}
	else if (mDirection == Direction::DownRight)
	{
		auto setup_column = [this, get_item_size](const std::vector<Node*>& items, glm::vec2 column_size, float column_x) {
			glm::vec2 pos = { column_x, getAbsoluteHeight() >= column_size.y ? (getAbsoluteHeight() - column_size.y) * mAlign : 0.0f };
			for (auto item : items)
			{
				item->setPosition(pos);
				pos.y += get_item_size(*item).y;
			}
		};
		for (auto item : getNodes())
		{
			auto item_size = get_item_size(*item);
			bool break_group = is_need_break_group(group.size(), group_size.y + item_size.y, getAbsoluteHeight());
			if (break_group)
			{
				setup_column(group, group_size, group_pos);
				group_pos += group_size.x;
				group_size = { 0.0f, 0.0f };
				group.clear();
			}
			group.push_back(item.get());
			group_size.y += item_size.y;
			group_size.x = glm::max(group_size.x, item_size.x);
		}
		setup_column(group, group_size, group_pos);
	}
}

Row::Row()
{
	setDirection(Direction::RightDown);
	setBreakToFitSize(false);
}

Column::Column()
{
	setDirection(Direction::DownRight);
	setBreakToFitSize(false);
}
