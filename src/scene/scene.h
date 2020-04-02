#pragma once

#include <core/engine.h>
#include <platform/system.h>
#include <graphics/system.h>
#include <common/event_system.h>
#include <scene/node.h>

#include <platform/mouse.h>
#include <platform/touch.h>

namespace Scene
{
	class Scene : 
		public Common::EventSystem::Listenable<Platform::Mouse::Event>,
		public Common::EventSystem::Listenable<Platform::Touch::Event>
	{
	public:
		Scene();
		~Scene();

	public:
		void frame();
		void updateTransformations();
		
		size_t getNodesCount(std::shared_ptr<Node> node = nullptr) const;

	private:
		void event(const Platform::Mouse::Event& e) override;
		void event(const Platform::Touch::Event& e) override;

	private:
		void recursiveNodeUpdate(const std::shared_ptr<Node>& node);
		void recursiveNodeDraw(const std::shared_ptr<Node>& node);

	private:
		std::list<std::shared_ptr<Node>> getTouchableNodes(const std::shared_ptr<Node>& node, const glm::vec2& pos);
		std::list<std::shared_ptr<Node>> getTouchableNodes(const glm::vec2& pos);

	public:
		std::list<std::shared_ptr<Node>> getNodes(const std::shared_ptr<Node>& node, const glm::vec2& pos);
		std::list<std::shared_ptr<Node>> getNodes(const glm::vec2& pos);

	public:
		auto getRoot() const { return mRoot; }

		auto getRenderTarget() const { return mRenderTarget; }
		void setRenderTarget(std::shared_ptr<Renderer::RenderTarget> value) { mRenderTarget = value; }

	private:
		std::shared_ptr<Node> mRoot = std::make_shared<Node>();
		std::list<std::weak_ptr<Node>> mTouchedNodes;
		std::shared_ptr<Renderer::RenderTarget> mRenderTarget = nullptr;
		Renderer::Viewport mViewport;
	};
}