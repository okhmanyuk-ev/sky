#include "scene_editor.h"
#include <imgui.h>

using namespace Shared;

SceneEditor::SceneEditor(Scene::Scene& scene) : mScene(scene)
{
	//
}

void SceneEditor::event(const Platform::Mouse::Event& e)
{
	mMousePos = { e.x, e.y };
}

void SceneEditor::frame()
{
	if (!mEnabled)
		return;

	ImGui::Begin("Scene", &mEnabled);
	showRecursiveNodeTree(mScene.getRoot());
	ImGui::End();

	highlightNodeUnderCursor();
	highlightHoveredNode();
}

void SceneEditor::showRecursiveNodeTree(std::shared_ptr<Scene::Node> node)
{
	int flags = ImGuiTreeNodeFlags_DefaultOpen;

	if (node->getNodes().size() == 0)
		flags |= ImGuiTreeNodeFlags_Leaf;

	auto name = typeid(*node).name();
	bool opened = ImGui::TreeNodeEx((void*)&*node, flags, name);

	if (ImGui::BeginPopupContextItem())
	{
		showNodeEditor(node);
		ImGui::EndPopup();
	}
	else if (ImGui::IsItemHovered())
	{
		mHoveredNode = node;
		showTooltip(node);
	}

	if (opened)
	{
		for (auto _node : node->getNodes())
			showRecursiveNodeTree(_node);

		ImGui::TreePop();
	}
}

void SceneEditor::showNodeEditor(std::shared_ptr<Scene::Node> node)
{
	auto enabled = node->isEnabled();
	ImGui::Checkbox("Enabled", &enabled);
	node->setEnabled(enabled);

	ImGui::Separator();

	if (auto colored = std::dynamic_pointer_cast<Scene::Color>(node); colored != nullptr)
	{
		auto color = colored->getColor();
		ImGui::ColorEdit4("Color", (float*)&color);
		colored->setColor(color);
		ImGui::Separator();
	}

	if (auto label = std::dynamic_pointer_cast<Scene::Label>(node); label != nullptr)
	{
		auto fontSize = label->getFontSize();
		ImGui::DragFloat("Font Size", &fontSize, 1.0f, 0.0f, 96.0f);
		label->setFontSize(fontSize);
		ImGui::Separator();

		auto texture = label->getFont()->getTexture();

		if (texture != nullptr)
		{
			mEditorFontTexture = texture;

			glm::vec2 size = { (float)texture->getWidth(), (float)texture->getHeight() };

			const float MaxSize = 128.0f;

			auto max = glm::max(size.x, size.y);

			if (max > MaxSize)
				size *= (MaxSize / max);

			ImGui::Image((ImTextureID)&mEditorFontTexture, ImVec2(size.x, size.y));
			ImGui::Separator();
		}
	}

	if (auto sprite = std::dynamic_pointer_cast<Scene::Sprite>(node); sprite != nullptr)
	{
		auto texture = sprite->getTexture();

		if (texture != nullptr)
		{
			mEditorSpriteTexture = texture;

			glm::vec2 size = { (float)texture->getWidth(), (float)texture->getHeight() };

			const float MaxSize = 128.0f;

			auto max = glm::max(size.x, size.y);

			if (max > MaxSize)
				size *= (MaxSize / max);

			ImGui::Image((ImTextureID)&mEditorSpriteTexture, ImVec2(size.x, size.y));
			ImGui::Separator();
		}
	}

	if (auto rectangle = std::dynamic_pointer_cast<Scene::Rectangle>(node); rectangle != nullptr)
	{
		auto rounding = rectangle->getRounding();
		ImGui::DragFloat("Rounding", (float*)&rounding, 0.01f, 0.0f, 1.0f);
		rectangle->setRounding(rounding);
		ImGui::Separator();
	}

	auto position = node->getPosition();
	auto size = node->getSize();
	auto stretch = node->getStretch();
	auto origin = node->getOrigin();
	auto margin = node->getMargin();
	auto anchor = node->getAnchor();
	auto pivot = node->getPivot();
	auto scale = node->getScale();
	auto rotation = node->getRotation();

	ImGui::DragFloat2("Position", (float*)&position);
	ImGui::DragFloat2("Size", (float*)&size);
	ImGui::DragFloat2("Stretch", (float*)&stretch, 0.01f, -1.0f, 1.0f);
	ImGui::DragFloat2("Origin", (float*)&origin);
	ImGui::DragFloat2("Margin", (float*)&margin);
	ImGui::DragFloat2("Anchor", (float*)&anchor, 0.01f, 0.0f, 1.0f);
	ImGui::DragFloat2("Pivot", (float*)&pivot, 0.01f, 0.0f, 1.0f);
	ImGui::DragFloat2("Scale", (float*)&scale, 0.01f, 0.0f, 2.0f);
	ImGui::SliderAngle("Rotation", &rotation);

	node->setPosition(position);
	node->setSize(size);
	node->setStretch(stretch);
	node->setOrigin(origin);
	node->setMargin(margin);
	node->setAnchor(anchor);
	node->setPivot(pivot);
	node->setScale(scale);
	node->setRotation(rotation);
}

void SceneEditor::showTooltip(std::shared_ptr<Scene::Node> node)
{
	if (auto sprite = std::dynamic_pointer_cast<Scene::Sprite>(node); sprite != nullptr)
	{
		auto texture = sprite->getTexture();

		if (texture == nullptr)
			return;

		mSpriteTexture = texture;

		glm::vec2 size = { (float)texture->getWidth(), (float)texture->getHeight() };

		const float MaxSize = 256.0f;

		auto max = glm::max(size.x, size.y);

		if (max > MaxSize)
			size *= (MaxSize / max);

		ImGui::BeginTooltip();
		ImGui::Image((ImTextureID)&mSpriteTexture, ImVec2(size.x, size.y));
		ImGui::EndTooltip();
	}
	else if (auto label = std::dynamic_pointer_cast<Scene::Label>(node); label != nullptr)
	{
		auto str = label->getText().cpp_str();
		ImGui::BeginTooltip();
		ImGui::Text("Label: %s", str.c_str());
		ImGui::EndTooltip();
	}
}

void SceneEditor::highlightNodeUnderCursor()
{
	/*auto nodes = mScene.getNodes(mMousePos);

	if (nodes.empty())
		return;

	ImGui::BeginTooltip();
	for (auto node : nodes)
	{
		auto name = typeid(node).name();
		ImGui::Text(name);
	}
	ImGui::EndTooltip();*/
}

void SceneEditor::highlightHoveredNode()
{
	if (mHoveredNode == nullptr)
		return;

	highlightNode(mHoveredNode);
}

void SceneEditor::highlightNode(std::shared_ptr<Scene::Node> node)
{
	if (node == nullptr)
		return;

	if (!node->hasScene())
		return;

	if (!node->isTransformReady())
		return;

	auto bounds = node->getGlobalBounds();

	bounds /= PLATFORM->getScale();

	auto top_y = bounds.y;
	auto bottom_y = bounds.w;
	auto left_x = bounds.x;
	auto right_x = bounds.z;

	auto model = glm::mat4(1.0f);
	model = glm::translate(model, { left_x, top_y, 0.0f });
	model = glm::scale(model, { right_x - left_x, bottom_y - top_y, 1.0f });

	GRAPHICS->begin();
	GRAPHICS->pushOrthoMatrix();
	GRAPHICS->drawRectangle(model, { Graphics::Color::White, 0.25f });
	GRAPHICS->drawLineRectangle(model, { Graphics::Color::White, 1.0f });
	GRAPHICS->pop();
	GRAPHICS->end();
}