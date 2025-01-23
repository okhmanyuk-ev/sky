#pragma once

#include <common/scheduler.h>
#include <sky/dispatcher.h>
#include <skygfx/skygfx.h>
#include <sky/console.h>
#include <platform/system.h>
#include <platform/input.h>
#include <imgui.h>
#include <imgui_stdlib.h>
#include <glm/glm.hpp>
#include <glm/ext.hpp>
#include <cstddef>
#include <set>

#define IMGUI_SYSTEM sky::Locator<Shared::ImguiSystem>::GetService()

namespace Shared
{
	class ImguiSystem :
		public sky::Listenable<Platform::Input::Keyboard::Event>,
		public sky::Listenable<Platform::Input::Keyboard::CharEvent>,
		public sky::Listenable<Platform::Input::Mouse::ButtonEvent>,
		public sky::Listenable<Platform::Input::Mouse::MoveEvent>,
		public sky::Listenable<Platform::Input::Mouse::ScrollEvent>,
		public sky::Listenable<Platform::Input::Touch::Event>
	{
	public:
		ImguiSystem();
		~ImguiSystem();

	public:
		void begin();
		void end();
		void ensureFont();

	public:
		bool isSamplerNearest() const { return mSamplerNearest; }
		void setSamplerNearest(bool value) { mSamplerNearest = value; }

		bool isScaleIndependence() const { return mScaleIndependence; }
		void setScaleIndependence(bool value) { mScaleIndependence = value; }

		auto getLogicalSize() const { return mLogicalSize; }
		auto getLogicalWidth() const { return mLogicalSize.x; }
		auto getLogicalHeight() const { return mLogicalSize.y; }

		float getScale() const;

	private:
		sky::CVar<bool> mSamplerNearest = sky::CVar<bool>("imgui_sampler_nearest", false);
		sky::CVar<bool> mScaleIndependence = sky::CVar<bool>("imgui_scale_independence", false);
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
