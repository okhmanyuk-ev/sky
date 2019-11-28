#include "scene_helpers.h"
#include <Scene/actionable.h>
#include <Shared/action_helpers.h>
#include <imgui.h>

using namespace Shared;

std::shared_ptr<Scene::Label> SceneHelpers::MakeFastPopupLabel(std::shared_ptr<Scene::Node> holder,
	std::shared_ptr<Scene::Node> target, const utf8_string& text, float text_size, float move_duration)
{
	auto label = std::make_shared<Scene::Actionable<Scene::Label>>();
	label->setFont(GET_CACHED_FONT("default"));
	label->setFontSize(text_size);
	label->setText(text);
	label->setPosition(holder->unproject(target->project({ target->getSize() / 2.0f })));
	label->setPivot({ 0.5f, 0.5f });
	label->setAlpha(0.0f);
	label->runAction(ActionHelpers::MakeSequence(
		ActionHelpers::Show(label, 0.5f),
		ActionHelpers::ChangePositionByDirection(label, { 0.0f, -1.0f }, 64.0f, move_duration),
		ActionHelpers::Hide(label, 0.5f),
		ActionHelpers::Kill(label)
	));
	holder->attach(label);
	return label;
}

std::tuple<std::shared_ptr<Scene::Node>, std::function<void(bool)>> SceneHelpers::MakeFastCheckbox(
	const utf8_string& title, float title_size, bool checked, std::function<void(bool)> changeCallback)
{
	auto holder = std::make_shared<Scene::Clickable<Scene::Node>>();

	auto outer_rect = std::make_shared<Scene::Actionable<Scene::Rectangle>>();
	outer_rect->setAlpha(0.33f);
	outer_rect->setVerticalStretch(1.0f);
	outer_rect->setMargin({ 8.0f, 8.0f });
	outer_rect->setAnchor({ 0.0f, 0.5f });
	outer_rect->setPivot({ 0.0f, 0.5f });
	holder->attach(outer_rect);

	outer_rect->runAction(ActionHelpers::ExecuteInfinite([outer_rect] {
		outer_rect->setWidth(outer_rect->getHeight());
	}));

	auto inner_rect = std::make_shared<Scene::Rectangle>();
	inner_rect->setAnchor({ 0.5f, 0.5f });
	inner_rect->setPivot({ 0.5f, 0.5f });
	inner_rect->setStretch({ 0.66f, 0.66f });
	inner_rect->setAlpha(0.66);
	inner_rect->setEnabled(checked);
	outer_rect->attach(inner_rect);

	auto label = std::make_shared<Scene::Label>();
	label->setFont(GET_CACHED_FONT("default"));
	label->setFontSize(title_size);
	label->setText(title);
	label->setAnchor({ 1.0f, 0.5f });
	label->setPivot({ 0.0f, 0.5f });
	label->setX(8.0f);
	outer_rect->attach(label);

	holder->setClickCallback([inner_rect, changeCallback] { 
		inner_rect->setEnabled(!inner_rect->isEnabled());
		changeCallback(inner_rect->isEnabled());
	});
	holder->setChooseCallback([outer_rect] { outer_rect->setAlpha(0.66f); });
	holder->setCancelChooseCallback([outer_rect] { outer_rect->setAlpha(0.33f); });

	auto setter = [inner_rect](bool value) {
		inner_rect->setEnabled(value);
	};

	return { holder, setter };
}

std::vector<std::shared_ptr<Scene::Node>> SceneHelpers::MakeFastRadioButtons(std::vector<utf8_string> titles,
	float title_size, int choosed, std::function<void(int)> changeCallback)
{
	std::vector<std::shared_ptr<Scene::Node>> result;
	auto setters = std::make_shared<std::vector<std::function<void(bool)>>>();

	int index = 0;

	for (const auto& title : titles)
	{
		auto [checkbox, setter] = MakeFastCheckbox(title, title_size, choosed == index, [index, setters, changeCallback](bool value) {
			for (auto setter : *setters)
				setter(false);
			
			setters->at(index)(true);
			changeCallback(index);
		});

		setters->push_back(setter);
		result.push_back(checkbox);

		index += 1;
	}

	return result;
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

SceneHelpers::FastButton::FastButton(const utf8_string& title, float fontSize)
{
	setChooseCallback([this] { setAlpha(0.66f); });
	setCancelChooseCallback([this] { setAlpha(0.33f); });

	mLabel = std::make_shared<Scene::Label>();
	mLabel->setFont(GET_CACHED_FONT("default"));
	mLabel->setFontSize(fontSize);
	mLabel->setText(title);
	mLabel->setAnchor({ 0.5f, 0.5f });
	mLabel->setPivot({ 0.5f, 0.5f });
	attach(mLabel);

	setButtonActive(true);
}

void SceneHelpers::FastButton::setButtonActive(bool value)
{
	setAlpha(value ? 0.33f : 0.125f);
	setClickEnabled(value);
	mButtonActive = value;
}

SceneHelpers::Progressbar::Progressbar()
{
	setAlpha(0.33f);
	
	mProgress = std::make_shared<Scene::Rectangle>();
	mProgress->setAnchor({ 0.0f, 0.5f });
	mProgress->setPivot({ 0.0f, 0.5f });
	mProgress->setVerticalStretch(1.0f);
	mProgress->setAlpha(0.66f);
	attach(mProgress);
}

void SceneHelpers::Progressbar::setProgress(float value)
{
	mProgress->setHorizontalStretch(value);
}

float SceneHelpers::Progressbar::getProgress() const
{
	return mProgress->getHorizontalStretch();
}

void SceneHelpers::Progressbar::addProgressWithIndicator(float value)
{
	auto indicator = std::make_shared<Scene::Rectangle>();
	indicator->setAnchor({ getProgress(), 0.5f });
	indicator->setPivot({ 0.0f, 0.5f });
	indicator->setStretch({ 0.0f, 1.0f });
	indicator->setAlpha(mProgress->getAlpha());
	indicator->setColor(Graphics::Color::Yellow);
	attach(indicator);

	runAction(Shared::ActionHelpers::MakeSequence(
		Shared::ActionHelpers::ChangeHorizontalStretch(indicator, value, 0.5f),
		Shared::ActionHelpers::ChangeColor(indicator, Graphics::Color::White, 0.5f),
		Shared::ActionHelpers::Execute([this, value, indicator] {
			setProgress(getProgress() + value);
			indicator->setEnabled(false);
		}),
		Shared::ActionHelpers::Kill(indicator)
	));
}