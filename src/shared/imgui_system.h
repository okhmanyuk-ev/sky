#pragma once

#include <core/engine.h>
#include <common/frame_system.h>
#include <common/event_system.h>
#include <renderer/all.h>

#include <platform/system.h>
#include <platform/input.h>

#include <imgui.h>
#include <imgui_stdlib.h>

#include <glm/glm.hpp>
#include <glm/ext.hpp>

#include <cstddef>

#define IMGUI_SYSTEM ENGINE->getSystem<Shared::ImguiSystem>()

namespace Shared
{
	class ImguiSystem :
		public Common::Event::Listenable<Platform::Input::Keyboard::Event>,
		public Common::Event::Listenable<Platform::Input::Keyboard::CharEvent>,
		public Common::Event::Listenable<Platform::Input::Mouse::ButtonEvent>,
		public Common::Event::Listenable<Platform::Input::Mouse::MoveEvent>,
		public Common::Event::Listenable<Platform::Input::Mouse::ScrollEvent>,
		public Common::Event::Listenable<Platform::Input::Touch::Event>
	{
	public:
		ImguiSystem();
		~ImguiSystem();

	public:
		void begin();
		void end();
		void ensureFont();

	public:
		auto isSamplerNearest() const { return mSamplerNearest; }
		void setSamplerNearest(bool value) { mSamplerNearest = value; }

		auto isScaleIndependence() const { return mScaleIndependence; }
		void setScaleIndependence(bool value) { mScaleIndependence = value; }

		auto getLogicalSize() const { return mLogicalSize; }
		auto getLogicalWidth() const { return mLogicalSize.x; }
		auto getLogicalHeight() const { return mLogicalSize.y; }

		float getScale() const;

	private:
		bool mSamplerNearest = true;
		bool mScaleIndependence = false;
		glm::vec2 mLogicalSize = { 0.0f, 0.0f };

	private:
		void onEvent(const Platform::Input::Keyboard::Event& e) override;
		void onEvent(const Platform::Input::Keyboard::CharEvent& e) override;
		void onEvent(const Platform::Input::Mouse::ButtonEvent& e) override;
		void onEvent(const Platform::Input::Mouse::MoveEvent& e) override;
		void onEvent(const Platform::Input::Mouse::ScrollEvent& e) override;
		void onEvent(const Platform::Input::Touch::Event& e) override;

	private:
		std::set<int> mReleasedKeyboardKeys;
		std::set<int> mReleasedMouseButtons;
		glm::vec2 mMousePos;
		glm::vec2 mMouseWheel;
	};
}
