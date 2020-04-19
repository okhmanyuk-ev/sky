#pragma once

#include <scene/scene.h>

namespace Shared
{
	class SceneEditor : public Common::EventSystem::Listenable<Platform::Mouse::Event>
	{
	public:
		SceneEditor(Scene::Scene& scene);

	private:
		void event(const Platform::Mouse::Event& e) override;

	public:
		void show();

	private:
		void showRecursiveNodeTree(std::shared_ptr<Scene::Node> node);
		void showTooltip(std::shared_ptr<Scene::Node> node);
		void highlightNodeUnderCursor();
		void highlightHoveredNode();
		void highlightNode(std::shared_ptr<Scene::Node> node);

	private:
		Scene::Scene& mScene;
		glm::vec2 mMousePos = { 0.0f, 0.0f };
		std::shared_ptr<Renderer::Texture> mSpriteTexture = nullptr;
		std::shared_ptr<Scene::Node> mHoveredNode = nullptr;
	};
}