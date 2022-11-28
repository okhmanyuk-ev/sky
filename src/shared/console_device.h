#pragma once

// TODO: change smart history
// no cycle and checking,
// just remove equal items when adding

#include <console/device.h>
#include <console/system.h>
#include <common/interpolator.h>
#include <common/event_system.h>
#include <common/frame_system.h>
#include <common/timer.h>
#include <platform/system.h>
#include <platform/input.h>
#include <glm/glm.hpp>
#include <glm/ext.hpp>
#include <graphics/color.h>
#include <shared/touch_emulator.h>

#include <deque>
#include <imgui.h>

namespace Shared
{
	class ConsoleDevice : public Console::Device, 
		public Common::FrameSystem::Frameable,
		public Common::Event::Listenable<Platform::Input::Keyboard::Event>,
		public Common::Event::Listenable<TouchEmulator::Event>,
		public Common::Event::Listenable<Platform::System::VirtualKeyboardTextChanged>,
		public Common::Event::Listenable<Platform::System::VirtualKeyboardEnterPressed>
	{
	public:
		enum class State
		{
			Opened,
			Closed,
			Opening,
			Closing
		};

		enum class InputState
		{
			None,
			Text, // enter from keyboard
			Hints, // choosing between hints
			History, // choosing between history
			Completion // just completed from hints
		};

	private:
		struct Text
		{
			Console::Color color;
			std::string text;
			bool linebreak;
			Clock::TimePoint time;
		};

	public:
		ConsoleDevice();

		void write(const std::string& s, Console::Color color = Console::Color::Default) override;
		void writeLine(const std::string& s, Console::Color color = Console::Color::Default) override;
		void clear() override;

	public:
		auto getState() const { return mState; }

		void toggle();
		void close();
		void open();

		bool isOpened() const override { return mState != State::Closed; }
		
		bool isEnabled() const override { return mEnabled; }
		void setEnabled(bool value) override { mEnabled = value; }

		auto isHiddenButtonEnabled() const { return mHiddenButtonEnabled; }
		void setHiddenButtonEnabled(bool value) { mHiddenButtonEnabled = value; }

	private:
		void onFrame() override;
		void showHints(float height, float top);
		void showFastLogs();
		void showCloseButton(float pos_y);
		void drawText(const Text& text, glm::vec4 colorMultiplier = { 1.0f, 1.0f, 1.0f, 1.0f });
		void enterInput();

		void onEvent(const Platform::Input::Keyboard::Event& e) override;
		void onEvent(const TouchEmulator::Event& e) override;
		void onEvent(const Platform::System::VirtualKeyboardTextChanged& e) override;
		void onEvent(const Platform::System::VirtualKeyboardEnterPressed& e) override;
		
		void handleInputCompletion(ImGuiTextEditCallbackData* data);
		void handleInputHistory(ImGuiTextEditCallbackData* data);
	
	private:
		State mState = State::Closed;
		static const int inline InputTextSize = 128;
		char mInputText[InputTextSize] = {};
		std::vector<std::string> mInputHistory;
		int mInputHistoryPos = 0;
		std::deque<Text> mBuffer;
		bool mScrollToBack = false;
		bool mScrollToBackForce = false;
		Common::Interpolator mInterpolator;
		bool mHiddenButtonEnabled = true;
		
	private:
		struct Hint
		{
			enum class Type
			{
				CVar,
				Command,
				Alias
			};

			Type type;

			std::string name;
			std::optional<std::string> description;
			std::string args;
			std::vector<std::string> alias_value;
		};

	private:
		std::vector<Hint> getHints(const std::string& match) const;

	private:
		std::vector<Hint> mHints;
		int mSelectedHint = 0;
		bool mNeedToComplete = false;
		bool mCheckMouseForHints = false;
		bool mCheckMouseForClose = false;
		bool mCheckScrollForHints = false;
		InputState mInputState = InputState::None;
		bool mEnabled = true;
		Common::Timer mButtonTimer;
		int mButtonAttempts = 0;
		const int MaxButtonAttempts = 5;
	};
}
