#pragma once

#include <core/engine.h>
#include <common/frame_system.h>
#include <common/event_system.h>
#include <renderer/all.h>

#include <platform/system.h>
#include <platform/input.h>

#include <imgui.h>

#include <glm/glm.hpp>
#include <glm/ext.hpp>

#include <cstddef>

#define IMGUI_SYSTEM ENGINE->getSystem<Shared::ImguiSystem>()

namespace Shared
{
	class ImguiSystem :
		public Common::Event::Listenable<Platform::Input::Keyboard::Event>,
		public Common::Event::Listenable<Platform::Input::Mouse::Event>,
		public Common::Event::Listenable<Platform::Input::Touch::Event>
	{
	public:
		ImguiSystem();
		~ImguiSystem();

	public:
		void begin();
		void end();	
		void ensureFont();

	private:
		const skygfx::Vertex::Layout ImguiLayout = { sizeof(ImDrawVert), {
			{ skygfx::Vertex::Attribute::Type::Position, skygfx::Vertex::Attribute::Format::R32G32F, offsetof(ImDrawVert, pos) },
			{ skygfx::Vertex::Attribute::Type::Color, skygfx::Vertex::Attribute::Format::R8G8B8A8UN, offsetof(ImDrawVert, col) },
			{ skygfx::Vertex::Attribute::Type::TexCoord, skygfx::Vertex::Attribute::Format::R32G32F, offsetof(ImDrawVert, uv) } }
		};

		std::shared_ptr<Renderer::Shaders::Generic> mShader = std::make_shared<Renderer::Shaders::Generic>(ImguiLayout);
		
	public:
		auto getSampler() const { return mSampler; }
		void setSampler(skygfx::Sampler value) { mSampler = value; }

		auto isScaleIndependence() const { return mScaleIndependence; }
		void setScaleIndependence(bool value) { mScaleIndependence = value; }

		auto getLogicalSize() const { return mLogicalSize; }
		auto getLogicalWidth() const { return mLogicalSize.x; }
		auto getLogicalHeight() const { return mLogicalSize.y; }

		float getScale() const;

	private:
		skygfx::Sampler mSampler = skygfx::Sampler::Nearest;
		bool mScaleIndependence = false;
		glm::vec2 mLogicalSize = { 0.0f, 0.0f };

	private:	
		void onEvent(const Platform::Input::Keyboard::Event& e) override;
		void onEvent(const Platform::Input::Mouse::Event& e) override;
		void onEvent(const Platform::Input::Touch::Event& e) override;

	private:
		std::set<int> mReleasedKeyboardKeys;
		std::set<int> mReleasedMouseButtons;
		glm::vec2 mMousePos;
		glm::vec2 mMouseWheel;
	};
}