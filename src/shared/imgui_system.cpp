#include "imgui_system.h"
#include <Platform/defines.h>
#include <Common/event_system.h>
#include <Platform/system.h>
#include <Graphics/color.h>
#include <Renderer/texture.h>

using namespace Shared;

ImguiSystem::ImguiSystem()
{
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	
	auto& io = ImGui::GetIO();

	io.IniFilename = NULL;
	io.FontDefault = io.Fonts->AddFontDefault();
	
	{
		using Platform::Keyboard::Key;
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
	
	uint8_t* data;
	int32_t width;
	int32_t height;

	io.Fonts->GetTexDataAsRGBA32(&data, &width, &height);

	mTexture = new Renderer::Texture(width, height, 4, data);
	
	io.Fonts->TexID = mTexture;

	begin();
}

ImguiSystem::~ImguiSystem()
{
	ImGui::DestroyContext();
	delete mTexture;
}

void ImguiSystem::begin()
{
	ImGuiIO& io = ImGui::GetIO();

	io.DeltaTime = Clock::ToSeconds(FRAME->getTimeDelta());
	
	io.DisplaySize.x = PLATFORM->getLogicalWidth();
	io.DisplaySize.y = PLATFORM->getLogicalHeight();

	io.MousePos.x = mMousePos.x / PLATFORM->getScale();
	io.MousePos.y = mMousePos.y / PLATFORM->getScale();

	io.MouseWheel += mMouseWheel.y;

	io.KeyCtrl = PLATFORM->isKeyPressed(Platform::Keyboard::Key::Ctrl);
	io.KeyShift = PLATFORM->isKeyPressed(Platform::Keyboard::Key::Shift);
	io.KeyAlt = PLATFORM->isKeyPressed(Platform::Keyboard::Key::Alt);

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

	auto drawData = ImGui::GetDrawData();
	drawData->ScaleClipRects({ PLATFORM->getScale(), PLATFORM->getScale() });

	auto view = glm::lookAtLH(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(0.0f, 1.0f, 0.0f));
	auto projection = glm::orthoLH(0.0f, PLATFORM->getLogicalWidth(), PLATFORM->getLogicalHeight(), 0.0f, -1.0f, 1.0f);
	auto model = glm::mat4(1.0f);
	
	mShader->setProjectionMatrix(projection);
	mShader->setViewMatrix(view);
	mShader->setModelMatrix(model);

	RENDERER->setRenderTarget(nullptr);
	RENDERER->setViewport(Renderer::Viewport::FullScreen());
	RENDERER->setTopology(Renderer::Topology::TriangleList);
	RENDERER->setSampler(Renderer::Sampler::Nearest);
	RENDERER->setDepthMode(Renderer::DepthMode::None);
	RENDERER->setCullMode(Renderer::CullMode::None);
	RENDERER->setBlendMode(Renderer::BlendStates::NonPremultiplied);
	RENDERER->setShader(mShader);

	for (int i = 0; i < drawData->CmdListsCount; i++)
	{
		const auto cmds = drawData->CmdLists[i];
		
		RENDERER->setVertexBuffer({ cmds->VtxBuffer.Data, static_cast<size_t>(cmds->VtxBuffer.size()) });
		RENDERER->setIndexBuffer({ cmds->IdxBuffer.Data, static_cast<size_t>(cmds->IdxBuffer.size()) });

		int indexOffset = 0;

		for (auto& cmd : cmds->CmdBuffer)
		{
			if (cmd.UserCallback)
			{
				cmd.UserCallback(cmds, &cmd);
			}
			else
			{
				RENDERER->setTexture(*static_cast<Renderer::Texture*>(cmd.TextureId));
				RENDERER->setScissor({ {cmd.ClipRect.x, cmd.ClipRect.y }, { cmd.ClipRect.z - cmd.ClipRect.x, cmd.ClipRect.w - cmd.ClipRect.y } });
				RENDERER->drawIndexed(cmd.ElemCount, indexOffset);
			}
			indexOffset += cmd.ElemCount;
		}
	}
}

void ImguiSystem::present()
{
	end();
	begin();
}

void ImguiSystem::event(const Platform::Touch::Event& e)
{
	mMousePos = { static_cast<float>(e.x), static_cast<float>(e.y) };

	auto& io = ImGui::GetIO();

	if (e.type == Platform::Touch::Event::Type::Begin)
		io.MouseDown[0] = true;
	else if (e.type == Platform::Touch::Event::Type::End)
		mReleasedMouseButtons.insert(0);
}

void ImguiSystem::event(const Platform::Keyboard::Event& e)
{
	auto& io = ImGui::GetIO();

	int key = static_cast<int>(e.key);

	if (e.type == Platform::Keyboard::Event::Type::Pressed)
		io.KeysDown[key] = true;
	else
		mReleasedKeyboardKeys.insert(key);

	if (e.asciiChar != 0 && e.type == Platform::Keyboard::Event::Type::Pressed)
		io.AddInputCharacter(e.asciiChar);
}

void ImguiSystem::event(const Platform::Mouse::Event& e)
{
	auto& io = ImGui::GetIO();

	if (e.type == Platform::Mouse::Event::Type::Move)
	{
		mMousePos = { static_cast<float>(e.x), static_cast<float>(e.y) };
	}
	else if (e.type == Platform::Mouse::Event::Type::Wheel)
	{
		mMouseWheel.x += e.wheelX;
		mMouseWheel.y += e.wheelY;
	}
	else if (e.type == Platform::Mouse::Event::Type::ButtonDown)
	{
		if (e.button == Platform::Mouse::Button::Left)
			io.MouseDown[0] = true;
		else if (e.button == Platform::Mouse::Button::Right)
			io.MouseDown[1] = true;
		else if (e.button == Platform::Mouse::Button::Middle)
			io.MouseDown[2] = true;
	}
	else if (e.type == Platform::Mouse::Event::Type::ButtonUp)
	{
		if (e.button == Platform::Mouse::Button::Left)
			mReleasedMouseButtons.insert(0);
		else if (e.button == Platform::Mouse::Button::Right)
			mReleasedMouseButtons.insert(1);
		else if (e.button == Platform::Mouse::Button::Middle)
			mReleasedMouseButtons.insert(2);
	}
}