#include "imscene.h"

void ImScene::ImScene::begin()
{
	assert(!mWorking);
	mWorking = true;
}

glm::mat4 ImScene::ImScene::push(const std::string& name, const Transform& transform)
{
	assert(mWorking);

	glm::vec2 parent_size;

	if (mNodeStack.empty())
	{
		parent_size = glm::vec2(PLATFORM->getLogicalWidth(), PLATFORM->getLogicalHeight());
	}
	else
	{
		parent_size = mNodeStack.top().size;
	}

	auto node = Node();
	
	auto stretch = transform.getStretch();
	auto scale = transform.getScale();
	auto size = transform.getSize();

	auto verticalStretch = stretch.y / scale.y;
	auto horizontalStretch = stretch.x / scale.x;
	//auto verticalMargin = getVerticalMargin();
	//auto horizontalMargin = getHorizontalMargin();

	if (horizontalStretch >= 0.0f)
		size.x = (parent_size.x * horizontalStretch) /*- horizontalMargin*/;

	if (verticalStretch >= 0.0f)
		size.y = (parent_size.y * verticalStretch)/* - verticalMargin*/;
	
	node.size = size;

	if (mNodeStack.empty())
	{
		node.matrix = glm::mat4(1.0f);
	}
	else
	{
		node.matrix = mNodeStack.top().matrix;
	}

	node.matrix = glm::translate(node.matrix, { transform.getAnchor() * parent_size, 0.0f });
	node.matrix = glm::translate(node.matrix, { transform.getPosition(), 0.0f });
	//mTransform = glm::rotate(mTransform, getRotation(), { 0.0f, 0.0f, 1.0f });
	node.matrix = glm::scale(node.matrix, { transform.getScale(), 1.0f });
	node.matrix = glm::translate(node.matrix, { -transform.getPivot() * node.size, 0.0f });
	//mTransform = glm::translate(mTransform, { -getOrigin(), 0.0f });

	mNodeStack.push(node);

	return glm::scale(node.matrix, { node.size, 1.0f });
}

void ImScene::ImScene::pop(int count)
{
	assert(mWorking);

	for (int i = 0; i < count; i++)
	{
		mNodeStack.pop();
	}
}

void ImScene::ImScene::end()
{
	assert(mWorking);
	assert(mNodeStack.empty());

	mWorking = false;
}
