#include "scene_helpers.h"
#include <Scene/actionable.h>
#include <Shared/common_actions.h>
#include <imgui.h>
#include <Scene/clickable.h>
#include <Scene/rectangle.h>

using namespace Shared;

std::shared_ptr<Scene::Label> SceneHelpers::MakeFastPopupLabel(std::shared_ptr<Scene::Node> holder,
	std::shared_ptr<Scene::Node> target, const std::string& text, float text_size, float move_duration)
{
	auto label = std::make_shared<Scene::Actionable<Scene::Label>>();
	label->setFont(GET_CACHED_FONT("default"));
	label->setFontSize(text_size);
	label->setText(text);
	label->setPosition(holder->unproject(target->project({ target->getSize() / 2.0f })));
	label->setPivot({ 0.5f, 0.5f });
	label->setAlpha(0.0f);
	label->runAction(CommonActions::MakeSequence(
		CommonActions::Show(label, 0.5f),
		CommonActions::ChangePositionByDirection(label, { 0.0f, -1.0f }, 64.0f, move_duration),
		CommonActions::Hide(label, 0.5f),
		CommonActions::Kill(label)
	));
	holder->attach(label);
	return label;
}

std::shared_ptr<Scene::Node> SceneHelpers::MakeFastButton(const std::string& title, float title_size, std::function<void(std::shared_ptr<Scene::Node>)> callback)
{
	auto node = std::make_shared<Scene::Clickable<Scene::Rectangle>>();
	node->setAlpha(0.33f);
	node->setClickCallback([node, callback] { callback(node); });
	node->setChooseCallback([node] { node->setAlpha(0.66f); });
	node->setCancelChooseCallback([node] { node->setAlpha(0.33f); });

	auto label = std::make_shared<Scene::Label>();
	label->setFont(GET_CACHED_FONT("default"));
	label->setFontSize(title_size);
	label->setText(title);
	label->setAnchor({ 0.5f, 0.5f });
	label->setPivot({ 0.5f, 0.5f });
	node->attach(label);

	return node;
}

std::shared_ptr<Scene::Node> SceneHelpers::MakeFastButton(const std::string& title, float title_size, std::function<void()> callback)
{
	return MakeFastButton(title, title_size, [callback](auto node) { callback(); });
}

std::shared_ptr<Scene::Node> SceneHelpers::MakeHorizontalGrid(float height, 
	const std::vector<std::pair<float/*width*/, std::shared_ptr<Scene::Node>>> items)
{
	auto holder = std::make_shared<Scene::Node>();
	holder->setHeight(height);

	float x = 0.0f;

	for (auto [width, node] : items)
	{
		auto cell = std::make_shared<Scene::Node>();
		cell->setHeight(height);
		cell->setWidth(width);
		cell->setX(x);
		holder->attach(cell);
		
		cell->attach(node);

		x += width;
	}

	holder->setWidth(x);

	return holder;
}

std::shared_ptr<Scene::Node> SceneHelpers::MakeHorizontalGrid(const glm::vec2& cell_size, 
	const std::vector<std::shared_ptr<Scene::Node>> items)
{
	auto items_width = std::vector<std::pair<float, std::shared_ptr<Scene::Node>>>();

	for (auto item : items)
	{
		items_width.push_back({ cell_size.x, item });
	}

	return MakeHorizontalGrid(cell_size.y, items_width);
}

std::shared_ptr<Scene::Node> SceneHelpers::MakeVerticalGrid(float width,
	const std::vector<std::pair<float/*height*/, std::shared_ptr<Scene::Node>>> items)
{
	auto holder = std::make_shared<Scene::Node>();
	holder->setWidth(width);

	float y = 0.0f;

	for (auto [height, node] : items)
	{
		auto cell = std::make_shared<Scene::Node>();
		cell->setHeight(height);
		cell->setWidth(width);
		cell->setY(y);
		holder->attach(cell);

		cell->attach(node);

		y += height;
	}

	holder->setHeight(y);

	return holder;
}

std::shared_ptr<Scene::Node> SceneHelpers::MakeVerticalGrid(const glm::vec2& cell_size,
	const std::vector<std::shared_ptr<Scene::Node>> items)
{
	auto items_height = std::vector<std::pair<float, std::shared_ptr<Scene::Node>>>();

	for (auto item : items)
	{
		items_height.push_back({ cell_size.y, item });
	}

	return MakeVerticalGrid(cell_size.x, items_height);
}

void SceneHelpers::ShowGraphEditor(Scene::Node& root)
{
	ImGui::Begin("Scene");

	std::function<void(Scene::Node&)> show = [&show](Scene::Node& node) {
		int flags = ImGuiTreeNodeFlags_DefaultOpen;

		if (node.getNodes().size() == 0)
			flags |= ImGuiTreeNodeFlags_Leaf;

		std::string name = typeid(node).name();

		bool opened = ImGui::TreeNodeEx(&node, flags, name.c_str());

		auto colored = dynamic_cast<Scene::Color*>(&node);

		if (ImGui::BeginPopupContextItem())
		{
			auto enabled = node.isEnabled();

			ImGui::Checkbox("Enabled", &enabled);

			node.setEnabled(enabled);

			ImGui::Separator();

			if (colored)
			{
				auto color = colored->getColor();
				ImGui::ColorEdit4("Color", (float*)& color);
				colored->setColor(color);
				ImGui::Separator();
			}

			auto position = node.getPosition();
			auto size = node.getSize();
			auto stretch = node.getStretch();
			auto origin = node.getOrigin();
			auto margin = node.getMargin();
			auto anchor = node.getAnchor();
			auto pivot = node.getPivot();
			auto scale = node.getScale();
			auto rotation = node.getRotation();

			ImGui::DragFloat2("Position", (float*)&position);
			ImGui::DragFloat2("Size", (float*)&size);
			ImGui::DragFloat2("Stretch", (float*)&stretch, 0.01f, -1.0f, 1.0f);
			ImGui::DragFloat2("Origin", (float*)&origin);
			ImGui::DragFloat2("Margin", (float*)&margin);
			ImGui::DragFloat2("Anchor", (float*)&anchor, 0.01f, 0.0f, 1.0f);
			ImGui::DragFloat2("Pivot", (float*)&pivot, 0.01f, 0.0f, 1.0f);
			ImGui::DragFloat2("Scale", (float*)&scale);
			ImGui::DragFloat("Pivot", &rotation);

			node.setPosition(position);
			node.setSize(size);
			node.setStretch(stretch);
			node.setOrigin(origin);
			node.setMargin(margin);
			node.setAnchor(anchor);
			node.setPivot(pivot);
			node.setScale(scale);
			node.setRotation(rotation);

			ImGui::EndPopup();
		}
		/*else if (ImGui::IsItemHovered())
		{
			ImGui::BeginTooltip();
			ImGui::Text("Position: [%f, %f]", node.getX(), node.getY());
			ImGui::Text("Size: [%f, %f]", node.getWidth(), node.getHeight());
			ImGui::Text("Stretch: [%f, %f]", node.getHorizontalStretch(), node.getVerticalStretch());
			ImGui::Text("Origin: [%f, %f]", node.getHorizontalOrigin(), node.getVerticalOrigin());
			ImGui::Text("Margin: [%f, %f]", node.getHorizontalMargin(), node.getVerticalMargin());
			ImGui::Text("Anchor: [%f, %f]", node.getHorizontalAnchor(), node.getVerticalAnchor());
			ImGui::Text("Pivot: [%f, %f]", node.getHorizontalPivot(), node.getVerticalPivot());
			ImGui::Text("Scale: [%f, %f]", node.getHorizontalScale(), node.getVerticalScale());
			ImGui::Text("Rotation: [%f]", node.getRotation());
			ImGui::EndTooltip();
		}*/

		if (opened)
		{
			for (auto& n : node.getNodes())
				show(*n);

			ImGui::TreePop();
		}
	};

	for (auto& node : root.getNodes())
		show(*node);

	ImGui::End();
}