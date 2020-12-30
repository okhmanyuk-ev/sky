#pragma once

#include <list>
#include <graphics/system.h>
#include <scene/transform.h>
#include <glm/glm.hpp>
#include <glm/ext.hpp>

namespace Scene
{
	class Scene;

	class Node : public Transform
	{
		friend Scene;
	
	protected:
		enum class Touch
		{
			Begin,
			Continue,
			End
		};

	public:
		enum class AttachDirection
		{
			Front,
			Back
		};

	public:
		Node();
		virtual ~Node();

	public:
		void attach(std::shared_ptr<Node> node, AttachDirection attachDirection = AttachDirection::Back);
		void detach(std::shared_ptr<Node> node);
		void clear();

	public:
		glm::vec2 project(const glm::vec2& value) const;
		glm::vec2 unproject(const glm::vec2& value) const;
		
		// returning [pos, size]
		std::tuple<glm::vec2, glm::vec2> getGlobalBounds() const; 

		virtual Scene* getScene() const;
		virtual bool hitTest(const glm::vec2& value) const;
		virtual bool interactTest(const glm::vec2& value) const;

		virtual void updateTransform();

	protected:
		virtual void enterDraw();
		virtual void leaveDraw();
		virtual void update();
		virtual void draw();
		virtual void touch(Touch type, const glm::vec2& pos);

	public:
		auto getParent() const { return mParent; }
		auto hasParent() const { return mParent != nullptr; }
		auto hasScene() const { return getScene() != nullptr; }
		const auto& getNodes() const { return mNodes; }
		auto hasNodes() const { return !mNodes.empty(); }

		const auto& getTransform() const { return mTransform; }
		void setTransform(const glm::mat4& value) { mTransform = value; }

		bool isEnabled() const { return mEnabled; }
		void setEnabled(bool value) { mEnabled = value; }

		bool isVisible() const { return mVisible; }
		void setVisible(bool value) { mVisible = value; }

		bool isInteractions() const { return mInteractions; }
		void setInteractions(bool value) { mInteractions = value; }

		bool isTouchable() const { return mTouchable; }
		void setTouchable(bool value) { mTouchable = value; }

		auto getTouchMask() const { return mTouchMask; }
		void setTouchMask(int value) { mTouchMask = value; }

		auto isTouching() const { return mTouching; }

		auto isTransformReady() const { return mTransformReady; }

		bool hasBatchGroup() const { return !mBatchGroup.empty(); }

		const auto& getBatchGroup() const { return mBatchGroup; }
		void setBatchGroup(const std::string& value) { mBatchGroup = value; }

		auto getAbsoluteSize() const { return mAbsoluteSize; }
		auto getAbsoluteWidth() const { return mAbsoluteSize.x; }
		auto getAbsoluteHeight() const { return mAbsoluteSize.y; }

	private:
		Node* mParent = nullptr;
		std::list<std::shared_ptr<Node>> mNodes;
		glm::mat4 mTransform = glm::mat4(1.0f);
		bool mEnabled = true;
		bool mVisible = true;
		bool mInteractions = true;
		bool mTouchable = false;
		int mTouchMask = 1 << 0;
		bool mTouching = false;
		bool mTransformReady = false;
		std::string mBatchGroup = "";
		glm::vec2 mAbsoluteSize = { 0.0f, 0.0f };
	};
}