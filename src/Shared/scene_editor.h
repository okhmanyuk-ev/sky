#pragma once

#include <Scene/scene.h>

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
		Scene::Scene& mScene;
		glm::vec2 mMousePos = { 0.0f, 0.0f };
	};
}