#pragma once

#include <Core/engine.h>
#include <Common/frame_system.h>
#include <Common/event_system.h>
#include <Platform/system.h>
#include <Renderer/system.h>
#include <Renderer/texture.h>

#include <Platform/keyboard.h>
#include <Platform/mouse.h>
#include <Platform/touch.h>

#include <imgui.h>

#include <glm/glm.hpp>
#include <glm/ext.hpp>

#include <Renderer/shaders/default.h>

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

		std::shared_ptr<Renderer::ShaderDefault> mShader = std::make_shared<Renderer::ShaderDefault>(ImguiLayout);
		
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