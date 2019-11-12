#pragma once

#include <Scene/label.h>
#include <Shared/cache_system.h>

namespace Shared::SceneHelpers
{
	std::shared_ptr<Scene::Label> MakeFastPopupLabel(std::shared_ptr<Scene::Node> holder, 
		std::shared_ptr<Scene::Node> target, const std::wstring& text, float text_size, float move_duration = 0.5f);

	std::shared_ptr<Scene::Node> MakeFastButton(const std::wstring& title, float title_size, 
		std::function<void(std::shared_ptr<Scene::Node>)> callback = nullptr);
	
	std::shared_ptr<Scene::Node> MakeFastButton(const std::wstring& title, float title_size,
		std::function<void()> callback = nullptr);

	std::shared_ptr<Scene::Node> MakeHorizontalGrid(float height, const std::vector<std::pair<float/*width*/, 
		std::shared_ptr<Scene::Node>>> items);
	
	std::shared_ptr<Scene::Node> MakeHorizontalGrid(const glm::vec2& cell_size, 
		const std::vector<std::shared_ptr<Scene::Node>> items); // TODO: std::span<..>

	std::shared_ptr<Scene::Node> MakeVerticalGrid(float width, const std::vector<std::pair<float/*height*/,
		std::shared_ptr<Scene::Node>>> items);

	std::shared_ptr<Scene::Node> MakeVerticalGrid(const glm::vec2& cell_size,
		const std::vector<std::shared_ptr<Scene::Node>> items); // TODO: std::span<..>

	void ShowGraphEditor(Scene::Node& root);
}