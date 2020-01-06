#pragma once

#include <Core/engine.h>
#include <Platform/system.h>
#include <Graphics/system.h>
#include <Common/event_system.h>
#include <Scene/node.h>

#include <Platform/mouse.h>
#include <Platform/touch.h>

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
		auto& getRoot() { return mRoot; }

	private:
		std::shared_ptr<Node> mRoot = std::make_shared<Node>();
		std::list<std::weak_ptr<Node>> mTouchedNodes;
	};
}