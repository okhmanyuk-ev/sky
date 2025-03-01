#pragma once

#include <sky/console.h>
#include <common/interpolator.h>
#include <sky/dispatcher.h>
#include <sky/scheduler.h>
#include <sky/timer.h>
#include <platform/system.h>
#include <platform/input.h>
#include <glm/glm.hpp>
#include <glm/ext.hpp>
#include <shared/touch_emulator.h>

#include <deque>
#include <imgui.h>

namespace sky
{
	class ImguiConsole : public sky::Console,
		public sky::Scheduler::Frameable,
		public sky::Listenable<Platform::Input::Keyboard::Event>,
		public sky::Listenable<Shared::TouchEmulator::Event>,
		public sky::Listenable<Platform::System::VirtualKeyboardTextChanged>,
		public sky::Listenable<Platform::System::VirtualKeyboardEnterPressed>
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
			sky::TimePoint time;
		};

	public:
		ImguiConsole();

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

		void scrollToBottom();

	private:
		void onFrame() override;
		void showHints(float height, float top);
		void showFastLogs();
		void showCloseButton(float pos_y);
		void drawText(const Text& text, glm::vec4 colorMultiplier = { 1.0f, 1.0f, 1.0f, 1.0f });
		void enterInput();

		void onEvent(const Platform::Input::Keyboard::Event& e) override;
		void onEvent(const Shared::TouchEmulator::Event& e) override;
		void onEvent(const Platform::System::VirtualKeyboardTextChanged& e) override;
		void onEvent(const Platform::System::VirtualKeyboardEnterPressed& e) override;

		void handleInputCompletion(ImGuiInputTextCallbackData* data);
		void handleInputHistory(ImGuiInputTextCallbackData* data);

	private:
		State mState = State::Closed;
		std::string mInputText;
		std::vector<std::string> mInputHistory;
		int mInputHistoryPos = 0;
		std::deque<Text> mBuffer;
		bool mNeedScrollToBottom = false;
		Common::Interpolator mInterpolator;
		bool mHiddenButtonEnabled = true;
		bool mAtBottom = false;

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
		sky::Timer mButtonTimer;
		int mButtonAttempts = 0;
		const int MaxButtonAttempts = 5;
	};
}
