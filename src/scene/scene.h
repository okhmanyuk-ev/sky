#pragma once

#include <core/engine.h>
#include <platform/system.h>
#include <graphics/system.h>
#include <common/event_system.h>
#include <scene/node.h>

#include <platform/input.h>

namespace Scene
{
	class Scene : 
		public Common::Event::Listenable<Platform::Input::Mouse::Event>,
		public Common::Event::Listenable<Platform::Input::Touch::Event>
	{
	private:
		class RootNode;

	public:
		using InteractTestCallback = std::function<bool(const glm::vec2&)>;

	public:
		Scene();
		~Scene();

	public:
		void frame();
		
		size_t getNodesCount(std::shared_ptr<Node> node = nullptr) const;

	private:
		void onEvent(const Platform::Input::Mouse::Event& e) override;
		void onEvent(const Platform::Input::Touch::Event& e) override;

	private:
		void recursiveNodeUpdateTransform(std::shared_ptr<Node> node);
		void recursiveNodeUpdate(std::shared_ptr<Node> node);
		void recursiveNodeDraw(std::shared_ptr<Node> node);
		bool interactTest(const glm::vec2& pos);

	private:
		std::list<std::shared_ptr<Node>> getTouchableNodes(std::shared_ptr<Node> node, const glm::vec2& pos);
		std::list<std::shared_ptr<Node>> getTouchableNodes(const glm::vec2& pos);

	public:
		std::list<std::shared_ptr<Node>> getNodes(std::shared_ptr<Node> node, const glm::vec2& pos);
		std::list<std::shared_ptr<Node>> getNodes(const glm::vec2& pos);

	public:
		auto getRoot() const { return mRoot; }

		auto getRenderTarget() const { return mRenderTarget; }
		void setRenderTarget(std::shared_ptr<Renderer::RenderTarget> value) { mRenderTarget = value; }

		auto getViewport() const { return mViewport; }

		void setInteractTestCallback(InteractTestCallback value) { mInteractTestCallback = value; }

	private:
		std::shared_ptr<RootNode> mRoot = std::make_shared<RootNode>();
		std::list<std::weak_ptr<Node>> mTouchedNodes;
		std::shared_ptr<Renderer::RenderTarget> mRenderTarget = nullptr;
		Renderer::Viewport mViewport;
		InteractTestCallback mInteractTestCallback = nullptr;
	};

	class Scene::RootNode : public Node
	{
	public:
		void setScene(Scene* value) { mScene = value; }
		Scene* getScene() const override { return mScene; }

	private:
		Scene* mScene = nullptr;
	};
}