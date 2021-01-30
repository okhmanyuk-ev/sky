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

void SceneEditor::onEvent(const Platform::Input::Mouse::Event& e)
{
	mMousePos = { e.x, e.y };

	if (mNodeSelectingMode)
	{
		if (e.type == Platform::Input::Mouse::Event::Type::Wheel)
		{
			if (e.wheelY < 0.0f)
				mSelectedNode += 1;
			else
				mSelectedNode -= 1;
		}
		if (e.type == Platform::Input::Mouse::Event::Type::ButtonUp && e.button == Platform::Input::Mouse::Button::Right)
		{
			mWantOpenNodeEditor = true;
		}
	}
}

void SceneEditor::frame()
{
	if (!mEnabled)
		return;

	mNodeSelectingMode = PLATFORM->isKeyPressed(Platform::Input::Keyboard::Key::Ctrl) && !ImGui::IsPopupOpen("NodeEditor");

	showNodeTreeWindow();

	if (mBatchGroupsEnabled)
	{
		showBatchGroupsMenu();
	}
	else
	{
		if (mNodeSelectingMode)
		{
			highlightNodeUnderCursor();
		}
		else
		{
			highlightHoveredNode();
		}
	}
}

void SceneEditor::showNodeTreeWindow()
{
	ImGui::Begin("Scene", &mEnabled);
	if (ImGui::Button("Batch Groups"))
	{
		mBatchGroupsEnabled = true;
	}
	ImGui::Separator();
	showRecursiveNodeTree(mScene.getRoot());
	if (mWantOpenNodeEditor)
	{
		ImGui::OpenPopup("NodeEditor");
	}
	mWantOpenNodeEditor = false;
	if (ImGui::BeginPopup("NodeEditor"))
	{
		showNodeEditor(mHoveredNode);
		ImGui::EndPopup();
	}
	ImGui::End();
}

void SceneEditor::showRecursiveNodeTree(std::shared_ptr<Scene::Node> node)
{
	int flags = ImGuiTreeNodeFlags_DefaultOpen;

	if (node->getNodes().size() == 0)
		flags |= ImGuiTreeNodeFlags_Leaf;

	if (node == mHoveredNode)
		flags |= ImGuiTreeNodeFlags_Selected;

	auto& n = *node.get();
	auto name = typeid(n).name();
	bool opened = ImGui::TreeNodeEx((void*)&*node, flags, "%s", name);

	if (ImGui::IsItemClicked(ImGuiMouseButton_Right))
	{
		mWantOpenNodeEditor = true;
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

	auto batch_group = node->getBatchGroup();
	ImGui::InputTextWithHint("Batch Group", "No batch group", batch_group.data(), batch_group.size(), ImGuiInputTextFlags_ReadOnly);

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

		auto text = label->getText();
		ImGui::InputTextWithHint("Text", "No text", text.data(), text.size(), ImGuiInputTextFlags_ReadOnly);
		
		ImGui::Separator();

		if (auto texture = label->getFont()->getTexture(); texture != nullptr)
		{
			ImGui::Text("%dx%d", texture->getWidth(), texture->getHeight());
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
			glm::i32vec2 texture_size = { texture->getWidth(), texture->getHeight() };
			ImGui::InputInt2("Resolution", (int*)&texture_size, ImGuiInputTextFlags_ReadOnly);

			auto tex_region = sprite->getTexRegion();
			ImGui::InputFloat4("Region", (float*)&tex_region, "%.0f", ImGuiInputTextFlags_ReadOnly);

			mEditorSpriteTexture = texture;
			drawImage(mEditorSpriteTexture, sprite->getTexRegion());
			ImGui::Separator();
		}
	}

	if (auto rectangle = std::dynamic_pointer_cast<Scene::Rectangle>(node); rectangle != nullptr)
	{
		auto absolute_rounding = rectangle->isAbsoluteRounding();
		auto rounding = rectangle->getRounding();

		ImGui::Checkbox("Absolute Rounding", &absolute_rounding);
		ImGui::SliderFloat("Rounding", &rounding, 0.0f, absolute_rounding ? 128.0f : 1.0f);
		ImGui::Separator();
		
		rectangle->setAbsoluteRounding(absolute_rounding);
		rectangle->setRounding(rounding);		
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

	auto absolute_size = node->getAbsoluteSize();
	ImGui::InputFloat2("Absolute Size", (float*)&absolute_size, "%.3f", ImGuiInputTextFlags_ReadOnly);
	ImGui::Separator();

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

		ImGui::BeginTooltip();
		drawImage(mSpriteTexture, sprite->getTexRegion());
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
	if (ImGui::IsWindowHovered(ImGuiHoveredFlags_AnyWindow | ImGuiHoveredFlags_AllowWhenBlockedByPopup) || ImGui::IsAnyItemActive())
		return;

	auto nodes = mScene.getNodes(mMousePos);

	if (nodes.empty())
		return;

	mSelectedNode = glm::clamp(mSelectedNode, 0, (int)nodes.size() - 1);

	ImGui::BeginTooltip();
	for (int i = 0; i < nodes.size(); i++)
	{
		auto node = *std::next(nodes.begin(), i);
		auto name = typeid(node).name();
		auto selected = i == mSelectedNode;
		ImGui::Selectable(name, selected);
		if (selected)
		{
			highlightNode(node, Graphics::Color::Yellow, true);
			mHoveredNode = node;
		}
		else
		{
			highlightNode(node, Graphics::Color::White, false);
		}
	}

	ImGui::EndTooltip();
}

void SceneEditor::highlightHoveredNode()
{
	if (mHoveredNode == nullptr)
		return;

	highlightNode(mHoveredNode);
}

void SceneEditor::highlightNode(std::shared_ptr<Scene::Node> node, const glm::vec3& color, bool filled)
{
	if (node == nullptr)
		return;

	if (!node->hasScene())
		return;

	if (!node->isTransformReady())
		return;

	auto [pos, size] = node->getGlobalBounds();

	pos /= PLATFORM->getScale();
	size /= PLATFORM->getScale();

	auto model = glm::mat4(1.0f);
	model = glm::translate(model, { pos, 0.0f });
	model = glm::scale(model, { size, 1.0f });

	GRAPHICS->begin();
	GRAPHICS->pushOrthoMatrix();
	if (filled)
	{
		GRAPHICS->drawRectangle(model, { color, 0.25f });
	}
	GRAPHICS->drawLineRectangle(model, { color, 1.0f });
	GRAPHICS->pop();
	GRAPHICS->end();
}

void SceneEditor::drawImage(const std::shared_ptr<Renderer::Texture>& texture, const Graphics::TexRegion& region)
{
	glm::vec2 size = { (float)texture->getWidth(), (float)texture->getHeight() };

	const float MaxSize = 256.0f;

	auto max = glm::max(size.x, size.y);
	float scale = 1.0f;

	if (max > MaxSize)
		scale = MaxSize / max;
		
	size *= scale;

	auto pos = ImGui::GetCursorScreenPos();

	auto prev_cursor_pos = ImGui::GetCursorPos();

	ImGui::Image((ImTextureID)&texture, ImVec2(size.x, size.y));
	
	if (region.size.x > 0.0f && region.size.y > 0.0f)
	{
		auto new_cursor_pos = ImGui::GetCursorPos();

		ImGui::SetCursorPos(prev_cursor_pos);

		auto drawList = ImGui::GetWindowDrawList();

		auto region_pos = region.pos * scale;
		auto region_size = region.size * scale;

		auto p = ImGui::GetCursorScreenPos();
		auto p1 = ImVec2(p.x + region_pos.x, p.y + region_pos.y);
		auto p2 = ImVec2(p1.x + region_size.x, p1.y + region_size.y);
		auto color = ImGui::GetColorU32(ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
		drawList->AddRect(p1, p2, color);

		ImGui::SetCursorPos(new_cursor_pos);
	}

	if (ImGui::IsItemHovered())
	{
		const auto& io = ImGui::GetIO();

		float region_sz = 64.0f;

		region_sz = glm::min(region_sz, glm::min(size.x, size.y));

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

void SceneEditor::showBatchGroupsMenu()
{
	mBatchGroups.clear();
	Scene::Scene::MakeBatchLists(mBatchGroups, mScene.getRoot());

	ImGui::Begin("BatchGroups", &mBatchGroupsEnabled);
	
	bool enabled = mScene.isBatchGroupsEnabled();

	ImGui::Checkbox("Enabled", &enabled);
	ImGui::Separator();

	mScene.setBatchGroupsEnabled(enabled);

	for (const auto& [name, nodes] : mBatchGroups)
	{
		ImGui::Selectable(name.c_str());

		if (ImGui::IsItemHovered())
		{
			for (auto node : nodes)
			{
				if (node.expired())
					continue;

				highlightNode(node.lock());
			}
		}

		if (ImGui::BeginPopupContextItem())
		{
			for (auto _node : nodes)
			{
				if (_node.expired())
					continue;

				auto node = _node.lock();

				auto _name = typeid(node).name();
				ImGui::Selectable(_name);
				if (ImGui::IsItemHovered())
				{
					highlightNode(node, Graphics::Color::Yellow);
					showTooltip(node);
				}
			}
			ImGui::EndPopup();
		}
	}
	ImGui::End();
}
