#pragma once

#include <list>
#include <graphics/system.h>
#include <scene/transform.h>
#include <glm/glm.hpp>
#include <glm/ext.hpp>
#include <common/actions.h>

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

		struct Bounds
		{
			glm::vec2 pos = { 0.0f, 0.0f };
			glm::vec2 size = { 0.0f, 0.0f };

			bool isIntersect(const Bounds& other) const;
		};

	public:
		Node();
		~Node();

	public:
		virtual void attach(std::shared_ptr<Node> node, AttachDirection attach_direction = AttachDirection::Back);
		virtual void detach(std::shared_ptr<Node> node);
		void clear();

		using SortPredicate = std::function<bool(const std::shared_ptr<Node>&, const std::shared_ptr<Node>&)>;
		void sort(SortPredicate predicate);

		glm::vec2 project(const glm::vec2& value) const;
		glm::vec2 unproject(const glm::vec2& value) const;

		Bounds getGlobalBounds() const;

		virtual Scene* getScene() const;
		virtual bool hitTest(const glm::vec2& value) const;
		virtual bool interactTest(const glm::vec2& value) const;

		virtual void updateTransform();
		virtual void updateAbsoluteSize();
		virtual void updateAbsoluteScale();

	protected:
		virtual void enterUpdate();
		virtual void update(sky::Duration dTime);
		virtual void leaveUpdate();
		virtual void enterDraw();
		virtual void draw();
		virtual void leaveDraw();
		virtual void touch(Touch type, const glm::vec2& pos);
		virtual void scroll(float x, float y);

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

		bool isTouchTransparent() const { return mTouchTransparent; }
		void setTouchTransparent(bool value) { mTouchTransparent = value; }

		auto isTouching() const { return mTouching; }

		auto isTransformReady() const { return mTransformReady; }

		const auto& getBatchGroup() const { return mBatchGroup; }
		void setBatchGroup(std::optional<std::string> value) { mBatchGroup = value; }

		auto getAbsoluteSize() const { return mAbsoluteSize; }
		auto getAbsoluteWidth() const { return mAbsoluteSize.x; }
		auto getAbsoluteHeight() const { return mAbsoluteSize.y; }

		auto getAbsoluteScale() const { return mAbsoluteScale; }

	protected:
		void setAbsoluteSize(const glm::vec2& value) { mAbsoluteSize = value; }
		void setAbsoluteScale(const glm::vec2& value) { mAbsoluteScale = value; }

	private:
		Node* mParent = nullptr;
		std::list<std::shared_ptr<Node>> mNodes;
		glm::mat4 mTransform = glm::mat4(1.0f);
		bool mEnabled = true;
		bool mVisible = true;
		bool mInteractions = true;
		bool mTouchable = false;
		bool mTouchTransparent = false;
		bool mTouching = false;
		bool mTransformReady = false;
		std::optional<std::string> mBatchGroup;
		glm::vec2 mAbsoluteSize = { 0.0f, 0.0f };
		glm::vec2 mAbsoluteScale = { 1.0f, 1.0f };

	public:
		void runAction(std::unique_ptr<Actions::Action> action) { mActions.add(std::move(action)); }
		void clearActions() { mActions.clear(); }
		bool hasActions() const { return mActions.hasActions(); }

	private:
		Actions::GenericActionsPlayer<Actions::Parallel> mActions;
	};
}