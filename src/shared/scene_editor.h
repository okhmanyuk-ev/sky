#pragma once

#include <scene/all.h>
#include <sky/console.h>

namespace Shared
{
	class SceneEditor : public Common::FrameSystem::Frameable,
		public sky::Listenable<Platform::Input::Mouse::ButtonEvent>,
		public sky::Listenable<Platform::Input::Mouse::MoveEvent>,
		public sky::Listenable<Platform::Input::Mouse::ScrollEvent>
	{
	public:
		SceneEditor(Scene::Scene& scene);

	private:
		void onEvent(const Platform::Input::Mouse::ButtonEvent& e) override;
		void onEvent(const Platform::Input::Mouse::MoveEvent& e) override;
		void onEvent(const Platform::Input::Mouse::ScrollEvent& e) override;

	private:
		void onFrame() override;

	private:
		void showNodeTreeWindow();
		void showRecursiveNodeTree(std::shared_ptr<Scene::Node> node);
		void showNodeEditor(std::shared_ptr<Scene::Node> node);
		void showTooltip(std::shared_ptr<Scene::Node> node);
		void highlightNodeUnderCursor();
		void highlightHoveredNode();
		void highlightNode(std::shared_ptr<Scene::Node> node, const glm::vec3& color = Graphics::Color::White, bool filled = true);
		void showBatchGroupsMenu();

	public:
		static void drawImage(std::shared_ptr<skygfx::Texture> texture, std::optional<Graphics::TexRegion> region = std::nullopt,
			float max_size = 256.0f);

	private:
		sky::CVar<bool> mEnabled = sky::CVar<bool>("scene_editor", false);
		bool mBatchGroupsEnabled = false;
		Scene::Scene& mScene;
		Scene::Scene::BatchGroups mBatchGroups;
		glm::vec2 mMousePos = { 0.0f, 0.0f };
		std::shared_ptr<Scene::Node> mHoveredNode = nullptr;
		int mSelectedNode = 0;
		bool mNodeSelectingMode = false;
		bool mWantOpenNodeEditor = false;
	};
}
