#include "console_device.h"

#include <common/easing.h>
#include <algorithm>
#include <cassert>
#include <shared/imgui_user.h>
#include <platform/defines.h>
#include <platform/system_android.h>
#include <shared/imgui_system.h>
#include <magic_enum.hpp>

using namespace Shared;

ConsoleDevice::ConsoleDevice()
{
	mInterpolator.setDuration(Clock::FromMilliseconds(500));
	close();
	mInterpolator.setPassed(mInterpolator.getDuration());
	mInputText[0] = 0x00;

	mButtonTimer.setCallback([this] {
		if (mButtonAttempts < 0)
			mButtonAttempts = 0;

		if (mButtonAttempts == 0)
			return;

		mButtonAttempts -= 1;
		
		LOGF("console open attempt {}/{}", mButtonAttempts, MaxButtonAttempts);
	});
	mButtonTimer.setInterval(Clock::FromSeconds(1.0f));
}

void ConsoleDevice::write(const std::string& s, Console::Color color)
{
	Text text;
	text.color = color;
	text.linebreak = false;
	text.text = s;
	text.time = Clock::Now();
	mBuffer.push_back(text);
}

void ConsoleDevice::writeLine(const std::string& s, Console::Color color)
{
	Text text;
	text.color = color;
	text.linebreak = true;
	text.text = s;
	text.time = Clock::Now();
	mBuffer.push_back(text);
	mScrollToBack = true;
}

void ConsoleDevice::clear()
{
	mBuffer.clear();
}

void ConsoleDevice::onFrame()
{
	if (!mEnabled)
		return;

	while (mBuffer.size() >= 400)
		mBuffer.pop_front();

	if (mState == State::Closed)
	{
		if (mHiddenButtonEnabled) 
		{
			ImGui::PushStyleVar(ImGuiStyleVar_Alpha, 0.0001f);
			ImGui::Begin("ConsoleButton", nullptr, ImGui::User::ImGuiWindowFlags_Overlay & ~ImGuiWindowFlags_NoInputs);
			ImGui::SetWindowPos(ImGui::User::TopRightCorner());

			if (ImGui::Button("Console"))
			{
				mButtonAttempts += 1;
				mButtonTimer.setPassed(Clock::Duration::zero());

				LOGF("console open attempt {}/{}", mButtonAttempts, MaxButtonAttempts);
		
				if (mButtonAttempts >= MaxButtonAttempts)
				{
					mButtonAttempts = 0;
					toggle();
				}
			}
		
			ImGui::End();
			ImGui::PopStyleVar(1);
		}

		showFastLogs();
		return;
	}

	ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowMinSize, ImVec2(0, 0));
	
	auto& style = ImGui::GetStyle();

	auto prevscrollsize = style.ScrollbarSize;

	style.ScrollbarSize = 2;

	ImGui::Begin("Console", nullptr, ImGuiWindowFlags_NoDecoration);

	auto bg_width = IMGUI_SYSTEM->getLogicalWidth();

	bg_width -= PLATFORM->getSafeAreaLeftMargin();
	bg_width -= PLATFORM->getSafeAreaRightMargin();

	auto bg_height = IMGUI_SYSTEM->getLogicalHeight();

#if defined(PLATFORM_MOBILE)
	bg_height *= 0.33f;
#else
	bg_height *= 0.66f;
#endif

	ImGui::SetWindowSize(ImVec2(bg_width, bg_height));
	ImGui::SetWindowPos(ImVec2(PLATFORM->getSafeAreaLeftMargin(), -ImGui::GetWindowHeight() * mInterpolator.getValue()));

	auto height = ImGui::GetWindowHeight();
	auto top = ImGui::GetWindowPos().y;

	std::vector<std::string> strings = {
		"Sky Engine",
		std::string(__DATE__) + " " + std::string(__TIME__)
	};
	
	auto backend_name = magic_enum::enum_name(RENDERER->getBackendType());
	strings.push_back(std::string(backend_name));
	
	const float margin = 17.0f;

	ImGui::SetCursorPosY(ImGui::GetCursorPos().y + ImGui::User::GetSafeAreaTop());

	auto savedCursor = ImGui::GetCursorPos();
	
	float base_x = ImGui::GetWindowWidth() - margin;

	for (const auto& str : strings)
	{
		ImGui::SetCursorPosX(base_x - ImGui::CalcTextSize(str.c_str()).x);
		ImGui::TextDisabled("%s", str.c_str());
	}
	
	ImGui::SetCursorPos(savedCursor);

	ImGui::BeginChild("ScrollingRegion", ImVec2(0, -ImGui::GetFrameHeightWithSpacing()), false);
	ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 1));

	for (auto& item : mBuffer)
	{
		drawText(item);
	}

	ImGui::PopStyleVar();

	if ((mScrollToBack && ImGui::GetScrollY() >= ImGui::GetScrollMaxY()) || mScrollToBackForce)
		ImGui::SetScrollHereY(1.0f);
	
	mScrollToBack = false;
	mScrollToBackForce = false;
	
	ImGui::EndChild();

	static auto filterLetters = [](ImGuiTextEditCallbackData* data) {
	    auto thiz = (ConsoleDevice*)data->UserData;
		if (data->EventFlag == ImGuiInputTextFlags_::ImGuiInputTextFlags_CallbackCompletion)
		{
            thiz->handleInputCompletion(data);
		}
		else if (data->EventFlag == ImGuiInputTextFlags_::ImGuiInputTextFlags_CallbackHistory)
		{
            thiz->handleInputHistory(data);
		}
		else if (data->EventFlag == ImGuiInputTextFlags_::ImGuiInputTextFlags_CallbackCharFilter)
		{
			if (data->EventChar >= 256)
				return 1;

			if (data->EventChar == '`')
				return 1;

			if (data->EventChar == '~')
				return 1;

            thiz->mInputState = InputState::Text;
		}
		else if (data->EventFlag == ImGuiInputTextFlags_::ImGuiInputTextFlags_CallbackAlways)
		{
			if (thiz->mNeedToComplete)
			{
                thiz->handleInputCompletion(data);
                thiz->mNeedToComplete = false;
			}
		}

		return 0;
	};

	ImGui::PushItemWidth(-1);

	int input_flags =
		ImGuiInputTextFlags_EnterReturnsTrue |
		ImGuiInputTextFlags_CallbackCharFilter |
		ImGuiInputTextFlags_CallbackCompletion |
		ImGuiInputTextFlags_CallbackHistory |
		ImGuiInputTextFlags_CallbackAlways;

#if defined(PLATFORM_MOBILE)
	input_flags |= ImGuiInputTextFlags_ReadOnly;
#endif

	if (ImGui::InputText("Input", mInputText, sizeof(mInputText), input_flags, filterLetters, this))
	{
		enterInput();
	}

	if (ImGui::IsItemClicked())
		PLATFORM->showVirtualKeyboard();

	ImGui::PopItemWidth();
	ImGui::SetKeyboardFocusHere();

	auto closeButtonY = ImGui::GetWindowHeight();

	ImGui::End();
	showHints(height, top);
	ImGui::PopStyleVar(2);

	showCloseButton(closeButtonY);

	style.ScrollbarSize = prevscrollsize;
}

void ConsoleDevice::showHints(float height, float top)
{
	if (mInputState != InputState::Hints)
		mSelectedHint = -1;

	if (mInputText[0] == 0x00 || mInputState == InputState::History)
	{
		mHints.clear();
		return;
	}

	mHints = getHints(mInputText);

	if (mHints.empty())
		return;

	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(4, 4));
	ImGui::SetNextWindowSizeConstraints(ImVec2(0, 0), ImVec2(-1, IMGUI_SYSTEM->getLogicalHeight() - height - top - 10));
	
	ImGui::Begin("ConsoleHints", nullptr, ImGui::User::ImGuiWindowFlags_ControlPanel & ~ImGuiWindowFlags_NoBringToFrontOnFocus);
	ImGui::SetWindowPos(ImVec2(8 + PLATFORM->getSafeAreaLeftMargin(), 4 + top + height));

	if (mSelectedHint == -1)
		ImGui::SetScrollHere();

	if (mSelectedHint > static_cast<int>(mHints.size()) - 1)
		mSelectedHint = static_cast<int>(mHints.size()) - 1;

	for (int i = 0; i < static_cast<int>(mHints.size()); i++)
	{
		bool selected = mSelectedHint == i && mInputState == InputState::Hints;

		auto& hint = mHints[i];

		ImGui::Selectable(hint.name.c_str(), selected);

		if (mCheckScrollForHints && selected)
		{
			ImGui::SetScrollHere();
			mCheckScrollForHints = false;
		}

		if (mCheckMouseForHints && ImGui::IsItemHovered(ImGuiHoveredFlags_RectOnly))
		{
		#if defined(PLATFORM_MOBILE)
			PLATFORM->setVirtualKeyboardText(hint.name);
			mInputState = InputState::Completion;
		#else
			mInputState = InputState::Hints;
			mSelectedHint = i;
			mNeedToComplete = true;
		#endif
		}

		if (auto args = hint.args; !args.empty())
		{
			ImGui::SameLine();
			ImGui::TextDisabled("%s", args.c_str());
		}

		if (hint.description.has_value())
		{
			ImGui::SameLine();
			ImGui::TextDisabled("%s", ("- " + hint.description.value()).c_str());
		}
	}

	mCheckMouseForHints = false;

	ImGui::End();
	ImGui::PopStyleVar();
}

void ConsoleDevice::showFastLogs()
{
	ImGui::Begin("Console Fast Logs", nullptr,
		ImGuiWindowFlags_NoInputs |
		ImGuiWindowFlags_NoDecoration |
		ImGuiWindowFlags_NoNav |
		ImGuiWindowFlags_NoBackground);

	ImGui::SetWindowSize(ImVec2(IMGUI_SYSTEM->getLogicalWidth(), 0));
	ImGui::SetWindowPos(ImGui::User::BottomLeftCorner(0.0f));
	
	auto now = Clock::Now();

	for (int i = 0; i < mBuffer.size(); i++)
	{
		auto item = mBuffer.at(i);

		const float MaxLifetime = 5.0f;
		const int MaxRowsOnScreen = 16;

		auto lifetime = Clock::ToSeconds(now - item.time);

		if (lifetime >= MaxLifetime)
			continue;

		if (mBuffer.size() - i > MaxRowsOnScreen)
			continue;

		auto alpha = 1.0f - glm::smoothstep(0.0f, MaxLifetime, lifetime);

		drawText(item, { 1.0f, 1.0f, 1.0f, alpha });
	}

	ImGui::End();
}

void ConsoleDevice::showCloseButton(float pos_y)
{
	if (mState != State::Opened)
		return;

	ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowMinSize, ImVec2(0, 0));
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(4, 4));

	ImGui::Begin("ConsoleCloseButton", nullptr, ImGuiWindowFlags_NoDecoration);
	ImGui::SetWindowPos(ImVec2((IMGUI_SYSTEM->getLogicalWidth()) - ImGui::GetWindowWidth() - 10.0f - PLATFORM->getSafeAreaRightMargin(), pos_y + 4.0f));

	ImGui::Button("Close");

	if (mCheckMouseForClose && ImGui::IsItemHovered(ImGuiHoveredFlags_RectOnly))
	{
		close();
	}

	mCheckMouseForClose = false;

	ImGui::End();
	
	ImGui::PopStyleVar(3);
}

void ConsoleDevice::drawText(const Text& text, glm::vec4 colorMultiplier)
{
	auto& style = ImGui::GetStyle();
	ImVec4 _col = style.Colors[ImGuiCol_Text];
	glm::vec3 color = { _col.x, _col.y, _col.z };

	switch (text.color)
	{
	case Console::Color::Black:
		color = Graphics::Color::Black;
		break;
	case Console::Color::DarkBlue:
		color = Graphics::Color::DarkBlue;
		break;
	case Console::Color::DarkGreen:
		color = Graphics::Color::DarkGreen;
		break;
	case Console::Color::DarkCyan:
		color = Graphics::Color::DarkCyan;
		break;
	case Console::Color::DarkRed:
		color = Graphics::Color::DarkRed;
		break;
	case Console::Color::DarkMagenta:
		color = Graphics::Color::DarkMagenta;
		break;
	case Console::Color::DarkYellow:
		color = Graphics::Color::DarkYellow;
		break;
	case Console::Color::Gray:
		color = Graphics::Color::Gray;
		break;
	case Console::Color::DarkGray:
		color = Graphics::Color::DarkGray;
		break;
	case Console::Color::Blue:
		color = Graphics::Color::Blue;
		break;
	case Console::Color::Green:
		color = Graphics::Color::Lime;
		break;
	case Console::Color::Cyan:
		color = Graphics::Color::Cyan;
		break;
	case Console::Color::Red:
		color = Graphics::Color::Red;
		break;
	case Console::Color::Magenta:
		color = Graphics::Color::Magenta;
		break;
	case Console::Color::Yellow:
		color = Graphics::Color::Yellow;
		break;
	case Console::Color::White:
		color = Graphics::Color::White;
		break;
	case Console::Color::Default:
		break;
	}

	auto color4 = glm::vec4(color, 1.0f);

	color4 *= colorMultiplier;

	ImGui::TextColored(ImVec4(color4.r, color4.g, color4.b, color4.a), "%s", text.text.c_str());

	if (!text.linebreak)
		ImGui::SameLine();
}

void ConsoleDevice::enterInput()
{
	if (mInputState == InputState::Hints)
	{
		mNeedToComplete = true;
	}
	else
	{
		auto line = std::string(mInputText);
		
		auto trim = [](std::string str) {
			while (str.starts_with(" "))
				str = str.substr(1);

			while (str.ends_with(" "))
				str = str.substr(0, str.size() - 1);

			return str;
		};

		line = trim(line);

		PLATFORM->setVirtualKeyboardText("");
		mInputText[0] = 0x00;
		if (!line.empty())
		{
			mInputHistory.push_back(line);
			mInputHistoryPos = static_cast<int>(mInputHistory.size());
		}
		writeLine("] " + line, Console::Color::Gray);
		mScrollToBackForce = true;
		EVENT->emit(ReadEvent({ line }));
	}
}

void ConsoleDevice::onEvent(const Platform::Input::Keyboard::Event& e)
{
	if (e.type == Platform::Input::Keyboard::Event::Type::Pressed && e.key == Platform::Input::Keyboard::Key::Tilde)
	{
		toggle();
	}
	else if (isOpened() && e.type == Platform::Input::Keyboard::Event::Type::Pressed && e.key == Platform::Input::Keyboard::Key::Escape)
	{
		close();
	}
}

void ConsoleDevice::onEvent(const TouchEmulator::Event& e)
{
	if (mState == State::Closed)
		return;

	if (e.type == TouchEmulator::Event::Type::End)
	{
		mCheckMouseForHints = true;
		mCheckMouseForClose = true;
	}
}

void ConsoleDevice::onEvent(const Platform::System::VirtualKeyboardTextChanged& e)
{
	if (!isOpened())
		return;
	
	strcpy(mInputText, e.text.c_str());
	mInputState = InputState::Text;
}

void ConsoleDevice::onEvent(const Platform::System::VirtualKeyboardEnterPressed& e)
{
	if (!isOpened())
		return;

	enterInput();
}

void ConsoleDevice::handleInputCompletion(ImGuiTextEditCallbackData* data)
{
	if (mInputState != InputState::Hints)
		return;

	auto hint_str = mHints[mSelectedHint].name;
	auto written_str = std::string(data->Buf, data->BufTextLen);

	if (written_str.length() >= hint_str.length() && written_str.substr(0, hint_str.length()) == hint_str)
	{
		mInputState = InputState::Text;
		return; // whole string is already written, do not replace it
	}
	
	strcpy(data->Buf, hint_str.c_str());
	data->CursorPos = data->SelectionStart = data->SelectionEnd = data->BufTextLen = static_cast<int>(hint_str.size());
	data->BufDirty = true;
	mInputState = InputState::Completion;
}

void ConsoleDevice::handleInputHistory(ImGuiTextEditCallbackData* data)
{
	if (mHints.size() > 0 && mInputState != InputState::History)
	{
		if (mInputState != InputState::Hints)
		{
			if (data->EventKey == ImGuiKey_::ImGuiKey_UpArrow)
				mSelectedHint = static_cast<int>(mHints.size()) - 1;
			else if (data->EventKey == ImGuiKey_::ImGuiKey_DownArrow)
				mSelectedHint = 0;
		}
		else
		{
			if (data->EventKey == ImGuiKey_::ImGuiKey_UpArrow)
				mSelectedHint--;
			else if (data->EventKey == ImGuiKey_::ImGuiKey_DownArrow)
				mSelectedHint++;

			if (mSelectedHint < 0)
				mSelectedHint = static_cast<int>(mHints.size()) - 1;
			else if (mSelectedHint > static_cast<int>(mHints.size()) - 1)
				mSelectedHint = 0;
		}

		mCheckScrollForHints = true;
		mInputState = InputState::Hints;

		return;
	}

	if (mInputHistory.size() == 0)
		return;

	mInputState = InputState::History;

	int startPos = mInputHistoryPos;

	while (true)
	{
		int lastPos = mInputHistoryPos;

		if (data->EventKey == ImGuiKey_::ImGuiKey_UpArrow)
			mInputHistoryPos--;
		else if (data->EventKey == ImGuiKey_::ImGuiKey_DownArrow)
			mInputHistoryPos++;

		if (mInputHistoryPos < 0)
			mInputHistoryPos = static_cast<int>(mInputHistory.size()) - 1;
		else if (mInputHistoryPos > static_cast<int>(mInputHistory.size()) - 1)
			mInputHistoryPos = 0;

		if (lastPos == mInputHistory.size())
			break;

		if (mInputHistory[mInputHistoryPos] != mInputHistory[lastPos])
			break;

		if (startPos == mInputHistoryPos)
			break;
	}

	auto s = mInputHistory[mInputHistoryPos];

	memcpy(data->Buf, s.data(), s.size());
	data->Buf[s.size()] = 0x00;
	data->CursorPos = data->SelectionStart = data->SelectionEnd = data->BufTextLen = static_cast<int>(s.size());
	data->BufDirty = true;
}

std::vector<ConsoleDevice::Hint> ConsoleDevice::getHints(const std::string& match) const
{
	if (match.empty())
		return {};
		
	auto lowercase_match = match;

	std::transform(lowercase_match.begin(), lowercase_match.end(), lowercase_match.begin(), tolower);

	std::vector<Hint> result = {};

	for (auto& [name, cvar] : CONSOLE->getCVars())
	{
		if (name.find(lowercase_match) == std::string::npos)
			continue;

		auto hint = Hint{
			.type = Hint::Type::CVar,
			.name = name,
			.description = cvar.getDescription(),
			.args = cvar.getArgsAsString()
		};

		result.push_back(hint);
	}

	for (auto& [name, command] : CONSOLE->getCommands())
	{
		if (name.find(lowercase_match) == std::string::npos)
			continue;

		auto hint = Hint{
			.type = Hint::Type::Command,
			.name = name,
			.description = command.getDescription(),
			.args = command.getArgsAsString()
		};
		
		result.push_back(hint);
	}

	for (auto& [name, value] : CONSOLE->getAliases())
	{
		if (name.find(lowercase_match) == std::string::npos)
			continue;

		auto hint = Hint{
			.type = Hint::Type::Alias,
			.name = name,
			.description = "alias",
			.args = Console::System::MakeStringFromTokens(value)
		};
		
		result.push_back(hint);
	}

	std::sort(result.begin(), result.end(), [](const Hint& left, const Hint& right) {
		return left.name < right.name;
	});
	
	if (result.empty())
	{
		return getHints(match.substr(0, match.length() - 1));
	}

	return result;
}

void ConsoleDevice::toggle()
{
	if (mState == State::Closed || mState == State::Closing)
		open();
	else
		close();
}

void ConsoleDevice::close()
{
	PLATFORM->hideVirtualKeyboard();

	mState = State::Closing;
	mInterpolator.setStartValue(0.0f);
	mInterpolator.setDestinationValue(1.0f);
	mInterpolator.setPassed(mInterpolator.getDuration() - mInterpolator.getPassed());
	mInterpolator.setEasingFunction(Easing::ExponentialIn);
	mInterpolator.setFinishCallback([&] { mState = State::Closed; });
}

void ConsoleDevice::open()
{
    PLATFORM->setVirtualKeyboardText("");
	PLATFORM->showVirtualKeyboard();

	mState = State::Opening;
	mInterpolator.setStartValue(1.0f);
	mInterpolator.setDestinationValue(0.0f);
	mInterpolator.setPassed(mInterpolator.getDuration() - mInterpolator.getPassed());
	mInterpolator.setEasingFunction(Easing::ExponentialOut);
	mInterpolator.setFinishCallback([&] { mState = State::Opened; });
}
