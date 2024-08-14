#include "imgui_system.h"
#include <platform/defines.h>
#include <common/event_system.h>
#include <platform/system.h>
#include <graphics/color.h>
#include <shared/imgui_user.h>
#include <graphics/all.h>

using namespace Shared;

ImguiSystem::ImguiSystem()
{
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();

	auto& io = ImGui::GetIO();

	io.IniFilename = NULL;

	using Platform::Input::Keyboard::Key;
	io.KeyMap[ImGuiKey_Tab] = static_cast<int>(Key::Tab);
	io.KeyMap[ImGuiKey_LeftArrow] = static_cast<int>(Key::Left);
	io.KeyMap[ImGuiKey_RightArrow] = static_cast<int>(Key::Right);
	io.KeyMap[ImGuiKey_UpArrow] = static_cast<int>(Key::Up);
	io.KeyMap[ImGuiKey_DownArrow] = static_cast<int>(Key::Down);
	io.KeyMap[ImGuiKey_PageUp] = static_cast<int>(Key::PageUp);
	io.KeyMap[ImGuiKey_PageDown] = static_cast<int>(Key::PageDown);
	io.KeyMap[ImGuiKey_Home] = static_cast<int>(Key::Home);
	io.KeyMap[ImGuiKey_End] = static_cast<int>(Key::End);
	io.KeyMap[ImGuiKey_Insert] = static_cast<int>(Key::Insert);
	io.KeyMap[ImGuiKey_Delete] = static_cast<int>(Key::Delete);
	io.KeyMap[ImGuiKey_Backspace] = static_cast<int>(Key::Backspace);
	io.KeyMap[ImGuiKey_Space] = static_cast<int>(Key::Space);
	io.KeyMap[ImGuiKey_Enter] = static_cast<int>(Key::Enter);
	io.KeyMap[ImGuiKey_Escape] = static_cast<int>(Key::Escape);
	io.KeyMap[ImGuiKey_A] = static_cast<int>(Key::A);
	io.KeyMap[ImGuiKey_C] = static_cast<int>(Key::C);
	io.KeyMap[ImGuiKey_V] = static_cast<int>(Key::V);
	io.KeyMap[ImGuiKey_X] = static_cast<int>(Key::X);
	io.KeyMap[ImGuiKey_Y] = static_cast<int>(Key::Y);
	io.KeyMap[ImGuiKey_Z] = static_cast<int>(Key::Z);
}

ImguiSystem::~ImguiSystem()
{
	ImGui::DestroyContext();
}

void ImguiSystem::begin()
{
	mLogicalSize = { PLATFORM->getLogicalWidth(), PLATFORM->getLogicalHeight() };
	mLogicalSize /= getScale();

	ensureFont();

	ImGuiIO& io = ImGui::GetIO();

	io.DeltaTime = Clock::ToSeconds(FRAME->getTimeDelta());

	io.DisplaySize.x = getLogicalWidth();
	io.DisplaySize.y = getLogicalHeight();

	io.MousePos.x = mMousePos.x / PLATFORM->getScale() / getScale();
	io.MousePos.y = mMousePos.y / PLATFORM->getScale() / getScale();

	io.MouseWheel += mMouseWheel.y;

	io.KeyCtrl = PLATFORM->isKeyPressed(Platform::Input::Keyboard::Key::LeftCtrl) ||
		PLATFORM->isKeyPressed(Platform::Input::Keyboard::Key::RightCtrl);

	io.KeyShift = PLATFORM->isKeyPressed(Platform::Input::Keyboard::Key::LeftShift) ||
		PLATFORM->isKeyPressed(Platform::Input::Keyboard::Key::RightShift);

	io.KeyAlt = PLATFORM->isKeyPressed(Platform::Input::Keyboard::Key::LeftAlt) ||
		PLATFORM->isKeyPressed(Platform::Input::Keyboard::Key::RightAlt);

	mMouseWheel = { 0.0f, 0.0f };

	ImGui::NewFrame();

	for (auto key : mReleasedKeyboardKeys)
		io.KeysDown[key] = false;

	mReleasedKeyboardKeys.clear();

	for (auto button : mReleasedMouseButtons)
		io.MouseDown[button] = false;

	mReleasedMouseButtons.clear();
}

void ImguiSystem::end()
{
	ImGui::Render();

	GRAPHICS->begin();
	GRAPHICS->pushSampler(mSampler);
	GRAPHICS->pushOrthoMatrix(getLogicalWidth(), getLogicalHeight());

	auto drawData = ImGui::GetDrawData();
	drawData->ScaleClipRects({ PLATFORM->getScale() * getScale(), PLATFORM->getScale() * getScale() });

	for (int i = 0; i < drawData->CmdListsCount; i++)
	{
		const auto cmds = drawData->CmdLists[i];

		uint32_t index_offset = 0;

		for (auto& cmd : cmds->CmdBuffer)
		{
			if (cmd.UserCallback)
			{
				cmd.UserCallback(cmds, &cmd);
			}
			else
			{
				auto texture = *(std::shared_ptr<skygfx::Texture>*)cmd.TextureId;
				GRAPHICS->pushScissor(skygfx::Scissor{ {cmd.ClipRect.x, cmd.ClipRect.y }, { cmd.ClipRect.z - cmd.ClipRect.x, cmd.ClipRect.w - cmd.ClipRect.y } });
				GRAPHICS->draw(nullptr, texture, [&](skygfx::utils::MeshBuilder& mesh_builder) {
					mesh_builder.begin(skygfx::utils::MeshBuilder::Mode::Triangles);

					for (uint32_t i = index_offset; i < index_offset + cmd.ElemCount; i++)
					{
						const auto& v = cmds->VtxBuffer[cmds->IdxBuffer[i]];
						mesh_builder.vertex({
							.pos = { v.pos.x, v.pos.y, 0.0f },
							.color = glm::unpackUnorm4x8(v.col),
							.texcoord = { v.uv.x, v.uv.y }
						});
					}

					mesh_builder.end();
				});
				GRAPHICS->pop();
			}
			index_offset += cmd.ElemCount;
		}
	}

	GRAPHICS->pop(2);
	GRAPHICS->end();
}

void ImguiSystem::ensureFont()
{
	auto& io = ImGui::GetIO();

	if (io.Fonts->IsBuilt())
		return;

	uint8_t* data;
	int32_t width;
	int32_t height;

	io.Fonts->GetTexDataAsRGBA32(&data, &width, &height);
	io.Fonts->TexID = ImGui::User::GetImTextureID(std::make_shared<skygfx::Texture>(width, height, skygfx::Format::Byte4, data));
}

void ImguiSystem::onEvent(const Platform::Input::Touch::Event& e)
{
	mMousePos = e.pos;

	auto& io = ImGui::GetIO();

	if (e.type == Platform::Input::Touch::Event::Type::Begin)
		io.MouseDown[0] = true;
	else if (e.type == Platform::Input::Touch::Event::Type::End)
		mReleasedMouseButtons.insert(0);
}

void ImguiSystem::onEvent(const Platform::Input::Keyboard::Event& e)
{
	auto& io = ImGui::GetIO();

	int key = static_cast<int>(e.key);

	if (e.type == Platform::Input::Keyboard::Event::Type::Pressed)
		io.KeysDown[key] = true;
	else
		mReleasedKeyboardKeys.insert(key);
}

void ImguiSystem::onEvent(const Platform::Input::Keyboard::CharEvent& e)
{
	auto& io = ImGui::GetIO();
	io.AddInputCharacter(e.codepoint);
}

void ImguiSystem::onEvent(const Platform::Input::Mouse::ButtonEvent& e)
{
	auto& io = ImGui::GetIO();

	const static std::unordered_map<Platform::Input::Mouse::Button, int> ButtonIndexMap = {
		{ Platform::Input::Mouse::Button::Left, 0 },
		{ Platform::Input::Mouse::Button::Middle, 1 },
		{ Platform::Input::Mouse::Button::Right, 2 }
	};

	if (e.type == Platform::Input::Mouse::ButtonEvent::Type::Pressed)
	{
		io.MouseDown[ButtonIndexMap.at(e.button)] = true;
	}
	else if (e.type == Platform::Input::Mouse::ButtonEvent::Type::Released)
	{
		mReleasedMouseButtons.insert(ButtonIndexMap.at(e.button));
	}
}

void ImguiSystem::onEvent(const Platform::Input::Mouse::MoveEvent& e)
{
	mMousePos = e.pos;
}

void ImguiSystem::onEvent(const Platform::Input::Mouse::ScrollEvent& e)
{
	mMouseWheel += e.scroll;
}

float ImguiSystem::getScale() const
{
	float result = 1.0f;

	if (mScaleIndependence)
		result /= PLATFORM->getScale();

	return result;
}
