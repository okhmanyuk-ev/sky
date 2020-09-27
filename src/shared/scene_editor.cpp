#include "scene_editor.h"
#include <imgui.h>
#include <console/system.h>
#include <common/console_commands.h>

using namespace Shared;

SceneEditor::SceneEditor(Scene::Scene& scene) : mScene(scene)
{
	CONSOLE->registerCVar("g_editor", { "bool" }, CVAR_GETTER_BOOL_FUNC(isEnabled), CVAR_SETTER_BOOL_FUNC(setEnabled));
}

SceneEditor::~SceneEditor()
{
	CONSOLE->removeCVar("g_editor");
}

void SceneEditor::event(const Platform::Input::Mouse::Event& e)
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
		ImGui::SliderFloat("Font Size", &fontSize, 0.0f, 96.0f);
		label->setFontSize(fontSize);
		ImGui::Separator();

		auto texture = label->getFont()->getTexture();

		if (texture != nullptr)
		{
			mEditorFontTexture = texture;
			drawImage(mEditorFontTexture);
			ImGui::Separator();
		}
	}

	if (auto sprite = std::dynamic_pointer_cast<Scene::Sprite>(node); sprite != nullptr)
	{
		auto texture = sprite->getTexture();

		if (texture != nullptr)
		{
			mEditorSpriteTexture = texture;
			drawImage(mEditorSpriteTexture);
			ImGui::Separator();
		}
	}

	if (auto rectangle = std::dynamic_pointer_cast<Scene::Rectangle>(node); rectangle != nullptr)
	{
		auto rounding = rectangle->getRounding();
		ImGui::SliderFloat("Rounding", &rounding, 0.0f, 1.0f);
		rectangle->setRounding(rounding);
		ImGui::Separator();
	}

	if (auto circle = std::dynamic_pointer_cast<Scene::Circle>(node); circle != nullptr)
	{
		auto radius = circle->getRadius();
		auto thickness = circle->getThickness();
		auto fill = circle->getFill();
		auto pie = circle->getPie();
		auto pie_pivot = circle->getPiePivot();
		auto inner_color = circle->getInnerColor();
		auto outer_color = circle->getOuterColor();

		ImGui::DragFloat("Radius", &radius);
		ImGui::DragFloat("Thickness", &thickness);
		ImGui::SliderFloat("Fill", &fill, 0.0f, 1.0f);
		ImGui::SliderFloat("Pie", &pie, 0.0f, 1.0f);
		ImGui::SliderFloat("Pie Pivot", &pie_pivot, -1.0f, 1.0f);
		ImGui::ColorEdit4("Inner Color", (float*)&inner_color);
		ImGui::ColorEdit4("Outer Color", (float*)&outer_color);

		circle->setRadius(radius);
		circle->setThickness(thickness);
		circle->setFill(fill);
		circle->setPie(pie);
		circle->setPiePivot(pie_pivot);
		circle->setInnerColor(inner_color);
		circle->setOuterColor(outer_color);
	
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
	auto radial_anchor = node->getRadialAnchor();
	auto radial_pivot = node->getRadialPivot();

	ImGui::DragFloat2("Position", (float*)&position);
	ImGui::DragFloat2("Size", (float*)&size);
	ImGui::DragFloat2("Stretch", (float*)&stretch, 0.01f, -1.0f, 1.0f);
	ImGui::DragFloat2("Origin", (float*)&origin);
	ImGui::DragFloat2("Margin", (float*)&margin);
	ImGui::DragFloat2("Anchor", (float*)&anchor, 0.01f, 0.0f, 1.0f);
	ImGui::DragFloat2("Pivot", (float*)&pivot, 0.01f, 0.0f, 1.0f);
	ImGui::DragFloat2("Scale", (float*)&scale, 0.01f, 0.0f, 2.0f);
	ImGui::SliderAngle("Rotation", &rotation);
	ImGui::SliderFloat("Radial Anchor", &radial_anchor, 0.0f, 1.0f);
	ImGui::SliderFloat("Radial Pivot", &radial_pivot, 0.0f, 1.0f);

	node->setPosition(position);
	node->setSize(size);
	node->setStretch(stretch);
	node->setOrigin(origin);
	node->setMargin(margin);
	node->setAnchor(anchor);
	node->setPivot(pivot);
	node->setScale(scale);
	node->setRotation(rotation);
	node->setRadialAnchor(radial_anchor);
	node->setRadialPivot(radial_pivot);
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

void SceneEditor::drawImage(const std::shared_ptr<Renderer::Texture>& texture)
{
	glm::vec2 size = { (float)texture->getWidth(), (float)texture->getHeight() };

	const float MaxSize = 256.0f;

	auto max = glm::max(size.x, size.y);

	if (max > MaxSize)
		size *= (MaxSize / max);

	auto pos = ImGui::GetCursorScreenPos();
	
	ImGui::Text("%dx%d", texture->getWidth(), texture->getHeight());
	ImGui::Image((ImTextureID)&texture, ImVec2(size.x, size.y));

	if (ImGui::IsItemHovered())
	{
		const auto& io = ImGui::GetIO();

		float region_sz = 96.0f;

		region_sz = glm::min(region_sz, size.x, size.y);

		float region_x = io.MousePos.x - pos.x - region_sz * 0.5f;
		float region_y = io.MousePos.y - pos.y - region_sz * 0.5f;

		region_x = glm::clamp(region_x, 0.0f, size.x - region_sz);
		region_y = glm::clamp(region_y, 0.0f, size.y - region_sz);

		float zoom = 4.0f;
		auto uv0 = ImVec2(region_x / size.x, region_y / size.y);
		auto uv1 = ImVec2((region_x + region_sz) / size.x, (region_y + region_sz) / size.y);

		ImGui::BeginTooltip();
		ImGui::Image((ImTextureID)&texture, ImVec2(region_sz * zoom, region_sz * zoom), uv0, uv1);
		ImGui::EndTooltip();
	}
}