#pragma once

#include <scene/all.h>

namespace Shared
{
	class SceneEditor : public Common::FrameSystem::Frameable,
		public Common::Event::Listenable<Platform::Input::Mouse::Event>
	{
	public:
		SceneEditor(Scene::Scene& scene);
		~SceneEditor();

	private:
		void onEvent(const Platform::Input::Mouse::Event& e) override;

	private:
		void frame() override;

	private:
		void showRecursiveNodeTree(std::shared_ptr<Scene::Node> node);
		void showNodeEditor(std::shared_ptr<Scene::Node> node);
		void showTooltip(std::shared_ptr<Scene::Node> node);
		void highlightNodeUnderCursor();
		void highlightHoveredNode();
		void highlightNode(std::shared_ptr<Scene::Node> node, const glm::vec3& color = Graphics::Color::White);
		void drawImage(const std::shared_ptr<Renderer::Texture>& texture);
		void showBatchGroupsMenu();
		
	public:
		bool isEnabled() const { return mEnabled; }
		void setEnabled(bool value) { mEnabled = value; }

	private:
		bool mEnabled = false;
		bool mBatchGroupsEnabled = false;
		Scene::Scene& mScene;
		Scene::Scene::BatchGroups mBatchGroups;
		glm::vec2 mMousePos = { 0.0f, 0.0f };
		std::shared_ptr<Renderer::Texture> mSpriteTexture = nullptr;
		std::shared_ptr<Renderer::Texture> mEditorSpriteTexture = nullptr;
		std::shared_ptr<Renderer::Texture> mEditorFontTexture = nullptr;
		std::shared_ptr<Scene::Node> mHoveredNode = nullptr;
	};
}