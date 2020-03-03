#pragma once

#include <core/engine.h>
#include <common/frame_system.h>
#include <common/event_system.h>
#include <platform/system.h>
#include <renderer/system.h>
#include <renderer/texture.h>

#include <platform/keyboard.h>
#include <platform/mouse.h>
#include <platform/touch.h>

#include <imgui.h>

#include <glm/glm.hpp>
#include <glm/ext.hpp>

#include <renderer/shaders/default.h>

#include <cstddef>

namespace Shared
{
	class ImguiSystem :
		public Common::EventSystem::Listenable<Platform::Keyboard::Event>,
		public Common::EventSystem::Listenable<Platform::Mouse::Event>,
		public Common::EventSystem::Listenable<Platform::Touch::Event>
	{
	public:
		ImguiSystem();
		~ImguiSystem();

	private:
		void begin();
		void end();
		
	public:
		void present();

	private:
		const Renderer::Vertex::Layout ImguiLayout = { sizeof(ImDrawVert), {
			{ Renderer::Vertex::Attribute::Type::Position, Renderer::Vertex::Attribute::Format::R32G32F, offsetof(ImDrawVert, pos) },
			{ Renderer::Vertex::Attribute::Type::Color, Renderer::Vertex::Attribute::Format::R8G8B8A8UN, offsetof(ImDrawVert, col) },
			{ Renderer::Vertex::Attribute::Type::TexCoord, Renderer::Vertex::Attribute::Format::R32G32F, offsetof(ImDrawVert, uv) } } };

		std::shared_ptr<Renderer::Shaders::Default> mShader = std::make_shared<Renderer::Shaders::Default>(ImguiLayout);
		
	private:	
		void event(const Platform::Keyboard::Event& e) override;
		void event(const Platform::Mouse::Event& e) override;
		void event(const Platform::Touch::Event& e) override;

	private:
		std::shared_ptr<Renderer::Texture> mTexture;
		std::set<int> mReleasedKeyboardKeys;
		std::set<int> mReleasedMouseButtons;
		glm::vec2 mMousePos;
		glm::vec2 mMouseWheel;
	};
}