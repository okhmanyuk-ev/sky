#include "node.h"
#include <scene/scene.h>

#include <algorithm>
#include <cassert>

using namespace Scene;

Node::Node()
{
	//
}

Node::~Node()
{
	// we should setup parent to nullptr for every child, 
	// because they can be stored locally somewhere

	for (auto node : mNodes) 
	{
		node->mParent = nullptr;
	}
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
	assert(mTransformReady);

	auto vp = getScene()->getViewport();
	auto scaled_size = vp.size / PLATFORM->getScale();

	glm::vec3 original = { value, 0.0f };
	glm::mat4 projection = glm::orthoLH(0.0f, scaled_size.x, scaled_size.y, 0.0f, -1.0f, 1.0f);
	glm::vec4 viewport = { 0.0f, vp.size.y, vp.size.x, -vp.size.y };
	glm::vec3 result = glm::project(original, getTransform(), projection, viewport);

	return { result.x, result.y };
}

glm::vec2 Node::unproject(const glm::vec2& value) const
{
	assert(mTransformReady);
	
	auto vp = getScene()->getViewport();
	auto scaled_size = vp.size / PLATFORM->getScale();

	glm::vec3 original = { value, 0.0f };
	glm::mat4 projection = glm::orthoLH(0.0f, scaled_size.x, scaled_size.y, 0.0f, -1.0f, 1.0f);
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

::Scene::Scene* Node::getScene() const
{
	if (hasParent())
		return getParent()->getScene();
	else
		return nullptr;
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

	auto parent_size = hasParent() ? getParent()->getSize() : (getScene()->getViewport().size / PLATFORM->getScale());

	if (horizontalStretch >= 0.0f)
		setWidth((parent_size.x * horizontalStretch) - horizontalMargin);

	if (verticalStretch >= 0.0f)
		setHeight((parent_size.y * verticalStretch) - verticalMargin);

	mTransform = hasParent() ? getParent()->getTransform() : glm::mat4(1.0f);
	mTransform = glm::translate(mTransform, { getAnchor() * parent_size, 0.0f });
	mTransform = glm::translate(mTransform, { getPosition(), 0.0f });
	mTransform = glm::rotate(mTransform, getRotation(), { 0.0f, 0.0f, 1.0f });
	mTransform = glm::scale(mTransform, { getScale(), 1.0f });
	mTransform = glm::translate(mTransform, { -getPivot() * getSize(), 0.0f });
	mTransform = glm::translate(mTransform, { -getOrigin(), 0.0f });

	mTransformReady = true;
}

void Node::enterDraw()
{
	//
}

void Node::leaveDraw()
{
	//
}

void Node::update()
{
	//
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