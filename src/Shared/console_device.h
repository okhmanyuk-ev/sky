#pragma once

// TODO: change smart history
// no cycle and checking,
// just remove equal items when adding

#include <Console/device.h>
#include <Console/system.h>
#include <Common/interpolator.h>
#include <Common/event_system.h>
#include <Common/frame_system.h>
#include <Common/timer.h>
#include <Platform/system.h>
#include <Platform/keyboard.h>
#include <Platform/mouse.h>
#include <Platform/touch.h>
#include <glm/glm.hpp>
#include <glm/ext.hpp>
#include <Graphics/color.h>
#include <Shared/touch_emulator.h>

#include <deque>
#include <imgui.h>

namespace Shared
{
	class ConsoleDevice : public Console::Device, 
		public Common::FrameSystem::Frameable,
		public Common::EventSystem::Listenable<Platform::Keyboard::Event>,
		public Common::EventSystem::Listenable<TouchEmulator::Event>
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
			Standart, // enter from keyboard
			Candidates, // choosing between candidates
			History, // choosing between history
			Completion // just completed from candidates
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

		bool isOpened() const override { return mState != State::Closed && mState != State::Closing; }
		void setEnabled(bool value) override { mEnabled = value; }

	private:
		void frame() override;
		void showCandidates(float height, float top);
		void showFastLogs();
		void drawText(const Text& text, glm::vec4 colorMultiplier = { 1.0f, 1.0f, 1.0f, 1.0f });

		void event(const Platform::Keyboard::Event& e) override;
		void event(const TouchEmulator::Event& e) override;
		
		void handleInputCompletion(ImGuiTextEditCallbackData* data);
		void handleInputHistory(ImGuiTextEditCallbackData* data);
	
	private:
		State mState = State::Closed;
		char mInputText[128] = {};
		std::vector<std::string> mInputHistory;
		int mInputHistoryPos = 0;
		std::deque<Text> mBuffer;
		bool mScrollToBack = false;
		Common::Interpolator mInterpolator;
		
	private:
		struct Candidate
		{
			enum class Type
			{
				CVar,
				Command,
				Alias
			};

			Type type;

			std::string name;
			std::string description;
			std::string args;
			std::vector<std::string> alias_value;
		};

	private:
		std::vector<Candidate> getCandidates(const std::string& match) const;

	private:
		std::vector<Candidate> mCandidates;
		int mSelectedCandidate = 0;
		bool mNeedToComplete = false;
		bool mCheckMouseForCandidates = false;
		bool mCheckScrollForCandidates = false;
		InputState mInputState = InputState::None;
		bool mEnabled = true;
		Common::Timer mButtonTimer;
		int mButtonAttempts = 0;
		const int MaxButtonAttempts = 5;
	};
}
