#pragma once

#include <Scene/label.h>
#include <Shared/cache_system.h>
#include <tinyutf8.hpp>

namespace Shared::SceneHelpers
{
	std::shared_ptr<Scene::Label> MakeFastPopupLabel(std::shared_ptr<Scene::Node> holder, 
		std::shared_ptr<Scene::Node> target, const utf8_string& text, float text_size, float move_duration = 0.5f);

	std::shared_ptr<Scene::Node> MakeFastButton(const utf8_string& title, float title_size,
		std::function<void(std::shared_ptr<Scene::Node>)> callback = nullptr);
	
	std::shared_ptr<Scene::Node> MakeFastButton(const utf8_string& title, float title_size,
		std::function<void()> callback = nullptr);

	std::tuple<std::shared_ptr<Scene::Node>, std::function<void(bool)>> MakeFastCheckbox(
		const utf8_string& title, float title_size, bool checked, std::function<void(bool)> changeCallback = nullptr);

	std::vector<std::shared_ptr<Scene::Node>> MakeFastRadioButtons(std::vector<utf8_string> titles,
		float title_size, int choosed, std::function<void(int)> changeCallback = nullptr);

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