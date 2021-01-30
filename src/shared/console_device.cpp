#include "console_device.h"

#include <common/easing.h>
#include <algorithm>
#include <cassert>
#include <shared/imgui_user.h>
#include <platform/defines.h>
#include <renderer/defines.h>
#include <platform/system_android.h>
#include <shared/imgui_system.h>

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

void ConsoleDevice::frame()
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
			ImGui::SetWindowPos(ImVec2((IMGUI_SYSTEM->getLogicalWidth()) - ImGui::GetWindowWidth() - 10, 10));

			if (ImGui::Button("Console"))
			{
				mButtonAttempts += 1;

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
	
#if defined(PLATFORM_MOBILE)
	ImGui::SetWindowSize(ImVec2(IMGUI_SYSTEM->getLogicalWidth(), IMGUI_SYSTEM->getLogicalHeight() * 0.25f));
#else
	ImGui::SetWindowSize(ImVec2(IMGUI_SYSTEM->getLogicalWidth(), IMGUI_SYSTEM->getLogicalHeight() * 0.75f));
#endif

	ImGui::SetWindowPos(ImVec2(0, -ImGui::GetWindowHeight() * mInterpolator.getValue()));

	auto height = ImGui::GetWindowHeight();
	auto top = ImGui::GetWindowPos().y;

	auto engineName = std::string("Sky Engine");
	auto apiName = std::string(RENDERER_NAME_LONG);
	auto buildName = std::string(__DATE__) + " " + std::string(__TIME__);
	
	const float margin = 17.0f;
	
	auto savedCursor = ImGui::GetCursorPos();
	
	ImGui::SetCursorPosX(ImGui::GetWindowWidth() - ImGui::CalcTextSize(engineName.c_str()).x - margin);
	ImGui::TextDisabled("%s", engineName.c_str());

	ImGui::SetCursorPosX(ImGui::GetWindowWidth() - ImGui::CalcTextSize(apiName.c_str()).x - margin);
	ImGui::TextDisabled("%s", apiName.c_str());
	
	ImGui::SetCursorPosX(ImGui::GetWindowWidth() - ImGui::CalcTextSize(buildName.c_str()).x - margin);
	ImGui::TextDisabled("%s", buildName.c_str());

	ImGui::SetCursorPos(savedCursor);

	ImGui::BeginChild("ScrollingRegion", ImVec2(0, -ImGui::GetFrameHeightWithSpacing()), false);
	ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 1));

	for (auto& item : mBuffer)
	{
		drawText(item);
	}

	ImGui::PopStyleVar();

	if (mScrollToBack)
	{
		ImGui::SetScrollHere();
		mScrollToBack = false;
	}

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
	showCandidates(height, top);
	ImGui::PopStyleVar(2);

	showCloseButton(closeButtonY);

	style.ScrollbarSize = prevscrollsize;
}

void ConsoleDevice::showCandidates(float height, float top)
{
	if (mInputState != InputState::Candidates)
		mSelectedCandidate = -1;

	if (!(mInputText[0] != 0x00 && mInputState != InputState::Completion && mInputState != InputState::History))
	{
		mCandidates.clear();
		return;
	}

	mCandidates = getCandidates(mInputText);

	if (mCandidates.empty())
		return;

	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(4, 4));
	ImGui::SetNextWindowSizeConstraints(ImVec2(0, 0), ImVec2(-1, IMGUI_SYSTEM->getLogicalHeight() - height - top - 10));
	
	ImGui::Begin("ConsoleCandidates", nullptr, ImGui::User::ImGuiWindowFlags_ControlPanel & ~ImGuiWindowFlags_NoBringToFrontOnFocus);
	ImGui::SetWindowPos(ImVec2(8, 4 + top + height));

	if (mSelectedCandidate == -1)
		ImGui::SetScrollHere();

	if (mSelectedCandidate > static_cast<int>(mCandidates.size()) - 1)
		mSelectedCandidate = static_cast<int>(mCandidates.size()) - 1;

	for (int i = 0; i < static_cast<int>(mCandidates.size()); i++)
	{
		bool selected = mSelectedCandidate == i && mInputState == InputState::Candidates;

		auto& candidate = mCandidates[i];

		ImGui::Selectable(candidate.name.c_str(), selected);

		if (mCheckScrollForCandidates && selected)
		{
			ImGui::SetScrollHere();
			mCheckScrollForCandidates = false;
		}

		if (mCheckMouseForCandidates && ImGui::IsItemHovered(ImGuiHoveredFlags_RectOnly))
		{
		#if defined(PLATFORM_MOBILE)
			PLATFORM->setVirtualKeyboardText(candidate.name);
			mInputState = InputState::Completion;
		#else
			mInputState = InputState::Candidates;
			mSelectedCandidate = i;
			mNeedToComplete = true;
		#endif
		}

		if (auto args = candidate.args; !args.empty())
		{
			ImGui::SameLine();
			ImGui::TextDisabled("%s", args.c_str());
		}

		if (auto description = candidate.description; !description.empty())
		{
			ImGui::SameLine();
			ImGui::TextDisabled("%s", ("- " + description).c_str());
		}
	}

	mCheckMouseForCandidates = false;

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
	ImGui::SetWindowPos(ImVec2(0, IMGUI_SYSTEM->getLogicalHeight() - ImGui::GetWindowHeight()));

	auto now = Clock::Now();

	for (auto& item : mBuffer)
	{
		const float MaxLifetime = 5.0f;
		
		auto lifetime = Clock::ToSeconds(now - item.time);

		if (lifetime >= MaxLifetime)
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
	ImGui::SetWindowPos(ImVec2((IMGUI_SYSTEM->getLogicalWidth()) - ImGui::GetWindowWidth() - 10.0f, pos_y + 4.0f));

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
		//color = Graphics::Color::DarkYellow;
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
	if (mInputState == InputState::Candidates)
	{
		mNeedToComplete = true;
	}
	else
	{
		std::string line(mInputText);
		PLATFORM->setVirtualKeyboardText("");
		mInputText[0] = 0x00;
		mInputHistory.push_back(line);
		mInputHistoryPos = static_cast<int>(mInputHistory.size());
		writeLine("] " + line);
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
		mCheckMouseForCandidates = true;
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
	if (mInputState != InputState::Candidates)
		return;

	auto s = mCandidates[mSelectedCandidate].name;

	strcpy(data->Buf, s.c_str());
	data->CursorPos = data->SelectionStart = data->SelectionEnd = data->BufTextLen = static_cast<int>(s.size());
	data->BufDirty = true;
	mInputState = InputState::Completion;
}

void ConsoleDevice::handleInputHistory(ImGuiTextEditCallbackData* data)
{
	if (mCandidates.size() > 0 && mInputState != InputState::History)
	{
		if (mInputState != InputState::Candidates)
		{
			if (data->EventKey == ImGuiKey_::ImGuiKey_UpArrow)
				mSelectedCandidate = static_cast<int>(mCandidates.size()) - 1;
			else if (data->EventKey == ImGuiKey_::ImGuiKey_DownArrow)
				mSelectedCandidate = 0;
		}
		else
		{
			if (data->EventKey == ImGuiKey_::ImGuiKey_UpArrow)
				mSelectedCandidate--;
			else if (data->EventKey == ImGuiKey_::ImGuiKey_DownArrow)
				mSelectedCandidate++;

			if (mSelectedCandidate < 0)
				mSelectedCandidate = static_cast<int>(mCandidates.size()) - 1;
			else if (mSelectedCandidate > static_cast<int>(mCandidates.size()) - 1)
				mSelectedCandidate = 0;
		}

		mCheckScrollForCandidates = true;
		mInputState = InputState::Candidates;

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

std::vector<ConsoleDevice::Candidate> ConsoleDevice::getCandidates(const std::string& match) const
{
	std::vector<Candidate> result = {};

	for (auto& [name, cvar] : CONSOLE->getCVars())
	{
		if (name.find(match) == std::string::npos)
			continue;

		Candidate candidate;
		candidate.name = name;
		candidate.description = cvar.getDescription();
		candidate.args = cvar.getArgsAsString();
		candidate.type = Candidate::Type::CVar;
		result.push_back(candidate);
	}

	for (auto& [name, command] : CONSOLE->getCommands())
	{
		if (name.find(match) == std::string::npos)
			continue;

		Candidate candidate;
		candidate.name = name;
		candidate.description = command.getDescription();		
		candidate.type = Candidate::Type::Command;
		candidate.args = command.getArgsAsString();
		result.push_back(candidate);
	}

	for (auto& [name, value] : CONSOLE->getAliases())
	{
		if (name.find(match) == std::string::npos)
			continue;

		Candidate candidate;
		candidate.type = Candidate::Type::Alias;
		candidate.name = name;
		candidate.description = "alias";
		candidate.alias_value = value;
		candidate.args = Console::System::MakeStringFromTokens(value);
		result.push_back(candidate);
	}

	std::sort(result.begin(), result.end(), [](const Candidate& left, const Candidate& right) {
		return left.name < right.name;
	});

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