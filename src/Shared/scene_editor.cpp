#include "scene_editor.h"
#include <imgui.h>
#include <Scene/color.h>

using namespace Shared;

SceneEditor::SceneEditor(Scene::Scene& scene) : mScene(scene)
{
	//
}

void SceneEditor::event(const Platform::Mouse::Event& e)
{
	mMousePos = { e.x, e.y };
}

void SceneEditor::show()
{
	ImGui::Begin("Scene");

	std::function<void(Scene::Node&)> show = [&show](Scene::Node& node) {
		int flags = ImGuiTreeNodeFlags_DefaultOpen;

		if (node.getNodes().size() == 0)
			flags |= ImGuiTreeNodeFlags_Leaf;

		auto name = typeid(node).name();
		bool opened = ImGui::TreeNodeEx(&node, flags, name);

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
			ImGui::DragFloat2("Scale", (float*)&scale, 0.01f, 0.0f, 2.0f);
			ImGui::SliderAngle("Rotation", &rotation);

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

	for (auto& node : mScene.getRoot()->getNodes())
		show(*node);

	ImGui::End();


	auto nodes = mScene.getNodes(mMousePos);
	nodes.clear(); // TODO: !

	if (!nodes.empty()) 
	{
		ImGui::BeginTooltip();
		for (auto node : nodes)
		{
			auto name = typeid(node).name();
			ImGui::Text(name);
		}
		ImGui::EndTooltip();
	}
}