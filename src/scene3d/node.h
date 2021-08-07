#pragma once

#include <core/engine.h>
#include <core/clock.h>
#include <scene3d/transform.h>
#include <scene3d/driver.h>
#include <graphics/all.h>

namespace Scene3D
{
	class Node : public Transform
	{
	public:
		Node();
		virtual ~Node();

	public:
		void attach(std::shared_ptr<Node> node);
		void detach(std::shared_ptr<Node> node);

	public:
		virtual void update(Clock::Duration dTime);
		virtual void enterDraw(Driver& driver);
		virtual void draw(Driver& driver);
		virtual void leaveDraw(Driver& driver);

		virtual void updateTransform();

	public:
		auto getParent() const { return mParent; }
		auto hasParent() const { return mParent != nullptr; }
		const auto& getNodes() const { return mNodes; }

		const auto& getTransform() const { return mTransform; }
		void setTransform(const glm::mat4& value) { mTransform = value; }

		bool isEnabled() const { return mEnabled; }
		void setEnabled(bool value) { mEnabled = value; }

		bool isVisible() const { return mVisible; }
		void setVisible(bool value) { mVisible = value; }

		auto isTransformReady() const { return mTransformReady; }

	private:
		Node* mParent = nullptr;
		std::list<std::shared_ptr<Node>> mNodes;
		glm::mat4 mTransform = glm::mat4(1.0f);
		bool mEnabled = true;
		bool mVisible = true;
		bool mTransformReady = false;
	};
}