#include "imgui_system.h"
#include <platform/defines.h>
#include <common/event_system.h>
#include <platform/system.h>
#include <graphics/color.h>
#include <renderer/texture.h>
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

	io.KeyCtrl = PLATFORM->isKeyPressed(Platform::Input::Keyboard::Key::Ctrl);
	io.KeyShift = PLATFORM->isKeyPressed(Platform::Input::Keyboard::Key::Shift);
	io.KeyAlt = PLATFORM->isKeyPressed(Platform::Input::Keyboard::Key::Alt);

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

		auto vertex_buffer = Renderer::Buffer{ cmds->VtxBuffer.Data, static_cast<size_t>(cmds->VtxBuffer.size()) };
		auto index_buffer = Renderer::Buffer{ cmds->IdxBuffer.Data, static_cast<size_t>(cmds->IdxBuffer.size()) };

		int indexOffset = 0;

		for (auto& cmd : cmds->CmdBuffer)
		{
			if (cmd.UserCallback)
			{
				cmd.UserCallback(cmds, &cmd);
			}
			else
			{
				auto texture = *(std::shared_ptr<Renderer::Texture>*)cmd.TextureId;
				GRAPHICS->pushScissor(skygfx::Scissor{ {cmd.ClipRect.x, cmd.ClipRect.y }, { cmd.ClipRect.z - cmd.ClipRect.x, cmd.ClipRect.w - cmd.ClipRect.y } });
				GRAPHICS->drawGeneric(skygfx::Topology::TriangleList, vertex_buffer, index_buffer, mShader, texture, cmd.ElemCount, indexOffset);
				GRAPHICS->pop();
			}
			indexOffset += cmd.ElemCount;
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
	io.Fonts->TexID = ImGui::User::GetImTextureID(std::make_shared<Renderer::Texture>(width, height, 4, data));
}

void ImguiSystem::onEvent(const Platform::Input::Touch::Event& e)
{
	mMousePos = { static_cast<float>(e.x), static_cast<float>(e.y) };

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

	if (e.asciiChar != 0 && e.type == Platform::Input::Keyboard::Event::Type::Pressed)
		io.AddInputCharacter(e.asciiChar);
}

void ImguiSystem::onEvent(const Platform::Input::Mouse::Event& e)
{
	auto& io = ImGui::GetIO();

	if (e.type == Platform::Input::Mouse::Event::Type::Move)
	{
		mMousePos = { static_cast<float>(e.x), static_cast<float>(e.y) };
	}
	else if (e.type == Platform::Input::Mouse::Event::Type::Wheel)
	{
		mMouseWheel.x += e.wheelX;
		mMouseWheel.y += e.wheelY;
	}
	else if (e.type == Platform::Input::Mouse::Event::Type::ButtonDown)
	{
		if (e.button == Platform::Input::Mouse::Button::Left)
			io.MouseDown[0] = true;
		else if (e.button == Platform::Input::Mouse::Button::Right)
			io.MouseDown[1] = true;
		else if (e.button == Platform::Input::Mouse::Button::Middle)
			io.MouseDown[2] = true;
	}
	else if (e.type == Platform::Input::Mouse::Event::Type::ButtonUp)
	{
		if (e.button == Platform::Input::Mouse::Button::Left)
			mReleasedMouseButtons.insert(0);
		else if (e.button == Platform::Input::Mouse::Button::Right)
			mReleasedMouseButtons.insert(1);
		else if (e.button == Platform::Input::Mouse::Button::Middle)
			mReleasedMouseButtons.insert(2);
	}
}

float ImguiSystem::getScale() const
{
	float result = 1.0f;

	if (mScaleIndependence)
		result /= PLATFORM->getScale();

	return result;
}