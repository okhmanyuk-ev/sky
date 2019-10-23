#include "node.h"

#include <algorithm>
#include <cassert>

using namespace Scene;

Node::Node()
{
	//
}

Node::~Node()
{
	//
}

void Node::attach(std::shared_ptr<Node> node)
{
	assert(node->mParent == nullptr);
	mNodes.push_back(node);
	node->mParent = this;
}

void Node::detach(std::shared_ptr<Node> node)
{
	assert(node->mParent == this);
	mNodes.remove(node);
	node->mParent = nullptr;
}

void Node::clear()
{
	while (!mNodes.empty())
	{
		detach(mNodes.front());
	}
}

glm::vec2 Node::project(const glm::vec2& value) const
{
	auto vp = Renderer::Viewport::FullScreen();

	auto width = PLATFORM->getLogicalWidth();
	auto height = PLATFORM->getLogicalHeight();

	glm::vec3 original = { value, 0.0f };
	glm::mat4 projection = glm::orthoLH(0.0f, width, height, 0.0f, -1.0f, 1.0f);
	glm::vec4 viewport = { 0.0f, vp.size.y, vp.size.x, -vp.size.y };
	glm::vec3 result = glm::project(original, getTransform(), projection, viewport);

	return { result.x, result.y };
}

glm::vec2 Node::unproject(const glm::vec2& value) const
{
	auto vp = Renderer::Viewport::FullScreen();

	auto width = PLATFORM->getLogicalWidth();
	auto height = PLATFORM->getLogicalHeight();

	glm::vec3 original = { value, 0.0f };
	glm::mat4 projection = glm::orthoLH(0.0f, width, height, 0.0f, -1.0f, 1.0f);
	glm::vec4 viewport = { 0.0f, vp.size.y, vp.size.x, -vp.size.y };
	glm::vec3 result = glm::unProject(original, getTransform(), projection, viewport);

	return { result.x, result.y };
}

glm::vec4 Node::getGlobalBounds() const
{
	auto tl = project({ 0.0f, 0.0f });
	auto tr = project({ getWidth(), 0.0f });
	auto bl = project({ 0.0f, getHeight() });
	auto br = project({ getWidth(), getHeight() });

	return {
		glm::min(glm::min(tl, tr), glm::min(bl, br)),
		glm::max(glm::max(tl, tr), glm::max(bl, br))
	};
}

bool Node::hitTest(const glm::vec2& value) const
{
	return
		value.x >= 0.0f &&
		value.y >= 0.0f &&
		value.x <= getWidth() &&
		value.y <= getHeight();
}

bool Node::interactTest(const glm::vec2& value) const
{
	return true;
}

void Node::updateTransform()
{
	auto verticalStretch = getVerticalStretch();
	auto horizontalStretch = getHorizontalStretch();
	auto verticalMargin = getVerticalMargin();
	auto horizontalMargin = getHorizontalMargin();

	if (hasParent())
	{
		if (verticalStretch >= 0.0f)
			setHeight((getParent()->getHeight() * verticalStretch) - (verticalMargin * 2.0f));

		if (horizontalStretch >= 0.0f)
			setWidth((getParent()->getWidth() * horizontalStretch) - (horizontalMargin * 2.0f));

		mTransform = getParent()->getTransform();
		mTransform = glm::translate(mTransform, { getAnchor() * getParent()->getSize(), 0.0f });
	}
	else
	{
		if (verticalStretch >= 0.0f)
			setHeight(PLATFORM->getLogicalHeight() * verticalStretch);

		if (horizontalStretch >= 0.0f)
			setWidth(PLATFORM->getLogicalWidth() * horizontalStretch);

		mTransform = glm::mat4(1.0f);
	}

	mTransform = glm::translate(mTransform, { getPosition(), 0.0f });
	mTransform = glm::rotate(mTransform, getRotation(), { 0.0f, 0.0f, 1.0f });
	mTransform = glm::scale(mTransform, { getScale(), 1.0f });
	mTransform = glm::translate(mTransform, { -getPivot() * getSize(), 0.0f });
	mTransform = glm::translate(mTransform, { -getOrigin(), 0.0f });
}

void Node::beginRender()
{
	//
}

void Node::endRender()
{
	//
}

void Node::update()
{
	updateTransform();
}

void Node::draw()
{
	//
}

void Node::touch(Touch type, const glm::vec2& pos)
{
	if (type == Touch::Begin)
		mTouching = true;
	else if (type == Touch::End)
		mTouching = false;
}