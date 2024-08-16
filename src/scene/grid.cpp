#include "grid.h"

using namespace Scene;

Grid::Grid()
{
}

void Grid::update(Clock::Duration dTime)
{
	Node::update(dTime);

	glm::vec2 pos = { 0.0f, 0.0f };

	if (mOrientation == Orientation::Vertical)
	{
		float max_height_in_row = 0.0f;

		for (auto node : getNodes())
		{
			if (pos.x + node->getWidth() > getAbsoluteWidth())
			{
				pos.x = 0.0f;
				pos.y += max_height_in_row;
				max_height_in_row = 0.0f;
			}
			node->setPosition(pos);
			pos.x += node->getWidth();
			max_height_in_row = glm::max(max_height_in_row, node->getHeight());
		}
	}
	else
	{
		float max_width_in_column = 0.0f;

		for (auto node : getNodes())
		{
			if (pos.y + node->getHeight() > getAbsoluteHeight())
			{
				pos.y = 0.0f;
				pos.x += max_width_in_column;
				max_width_in_column = 0.0f;
			}
			node->setPosition(pos);
			pos.y += node->getHeight();
			max_width_in_column = glm::max(max_width_in_column, node->getWidth());
		}
	}
}