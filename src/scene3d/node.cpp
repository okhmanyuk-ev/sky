#include "node.h"
#include <glm/glm.hpp>
#include <glm/ext.hpp>

using namespace Scene3D;

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
	mNodes.push_back(node);
	node->mParent = this;
}

void Node::detach(std::shared_ptr<Node> node)
{
	assert(node->mParent == this);
	mNodes.remove(node);
	node->mParent = nullptr;
}

void Node::update(Clock::Duration dTime)
{
	//
}

void Node::enterDraw(Driver& driver)
{
	//
}

void Node::draw(Driver& driver)
{
	//
}

void Node::leaveDraw(Driver& driver)
{
	//
}

void Node::updateTransform()
{
	//auto parent_size = hasParent() ? getParent()->getAbsoluteSize() : (getScene()->getViewport().size / PLATFORM->getScale());

	mTransform = hasParent() ? getParent()->getTransform() : glm::mat4(1.0f);
	//mTransform = glm::translate(mTransform, { getAnchor() * parent_size, 0.0f });
	//mTransform = glm::translate(mTransform, { getPosition(), 0.0f });
	mTransform = glm::rotate(mTransform, getRotation().x, { 1.0f, 0.0f, 0.0f });
	mTransform = glm::rotate(mTransform, getRotation().y, { 0.0f, 1.0f, 0.0f });
	mTransform = glm::rotate(mTransform, getRotation().z, { 0.0f, 0.0f, 1.0f });
	//mTransform = glm::rotate(mTransform, getRadialAnchor() * glm::pi<float>() * 2.0f, { 0.0f, 0.0f, 1.0f });
	//mTransform = glm::rotate(mTransform, -getRadialPivot() * glm::pi<float>() * 2.0f, { 0.0f, 0.0f, 1.0f });
	mTransform = glm::scale(mTransform, getScale());
	//mTransform = glm::translate(mTransform, { -getPivot() * getAbsoluteSize(), 0.0f });
	//mTransform = glm::translate(mTransform, { -getOrigin(), 0.0f });

	mTransformReady = true;
}
