#include "console_device.h"

#include <Platform/keyboard.h>
#include <Platform/mouse.h>
#include <Common/easing.h>
#include <algorithm>
#include <cassert>
#include <Shared/imgui_user.h>
#include <Platform/defines.h>
#include <Renderer/defines.h>
#include <Platform/system_android.h>

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
		ImGui::PushStyleVar(ImGuiStyleVar_Alpha, 0.0001f);
		ImGui::Begin("ConsoleButton", nullptr, ImGui::User::ImGuiWindowFlags_Overlay & ~ImGuiWindowFlags_NoInputs);
		ImGui::SetWindowPos(ImVec2((PLATFORM->getLogicalWidth()) - ImGui::GetWindowWidth() - 10, 10));

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

		showFastLogs();
		return;
	}

	ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowMinSize, ImVec2(0, 0));
	
	auto& style = ImGui::GetStyle();

	auto prevscrollsize = style.ScrollbarSize;

	style.ScrollbarSize = 2;

	ImGui::Begin("Console", nullptr, ImGuiWindowFlags_NoDecoration);
	
#if defined(PLATFORM_MOBILE) // TODO: make adaptive to keyboard
	ImGui::SetWindowSize(ImVec2(PLATFORM->getLogicalWidth(), PLATFORM->getLogicalHeight() * 0.25f));
#else
	ImGui::SetWindowSize(ImVec2(PLATFORM->getLogicalWidth(), PLATFORM->getLogicalHeight() * 0.75f));
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
	ImGui::TextDisabled(engineName.c_str());

	ImGui::SetCursorPosX(ImGui::GetWindowWidth() - ImGui::CalcTextSize(apiName.c_str()).x - margin);
	ImGui::TextDisabled(apiName.c_str());
	
	ImGui::SetCursorPosX(ImGui::GetWindowWidth() - ImGui::CalcTextSize(buildName.c_str()).x - margin);
	ImGui::TextDisabled(buildName.c_str());

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
		if (data->EventFlag == ImGuiInputTextFlags_::ImGuiInputTextFlags_CallbackCompletion)
		{
			((ConsoleDevice*)data->UserData)->handleInputCompletion(data);
		}
		else if (data->EventFlag == ImGuiInputTextFlags_::ImGuiInputTextFlags_CallbackHistory)
		{
			((ConsoleDevice*)data->UserData)->handleInputHistory(data);
		}
		else if (data->EventFlag == ImGuiInputTextFlags_::ImGuiInputTextFlags_CallbackCharFilter)
		{
			if (data->EventChar >= 256)
				return 1;

			if (data->EventChar == '`')
				return 1;

			if (data->EventChar == '~')
				return 1;

			((ConsoleDevice*)data->UserData)->mInputState = ConsoleDevice::InputState::Standart;
		}
		else if (data->EventFlag == ImGuiInputTextFlags_::ImGuiInputTextFlags_CallbackAlways)
		{
			if (((ConsoleDevice*)data->UserData)->mNeedToComplete)
			{
				((ConsoleDevice*)data->UserData)->handleInputCompletion(data);
				((ConsoleDevice*)data->UserData)->mNeedToComplete = false;
			}
		}

		return 0;
	};

	ImGui::PushItemWidth(-1);

	if (ImGui::InputText("Input", mInputText, sizeof(mInputText), ImGuiInputTextFlags_EnterReturnsTrue |
		ImGuiInputTextFlags_CallbackCharFilter | ImGuiInputTextFlags_CallbackCompletion |
		ImGuiInputTextFlags_CallbackHistory | ImGuiInputTextFlags_CallbackAlways, filterLetters, this))
	{
		if (mInputState == InputState::Candidates)
		{
			mNeedToComplete = true;
		}
		else
		{
			std::string line(mInputText);
			mInputText[0] = 0x00;
			mInputHistory.push_back(line);
			mInputHistoryPos = static_cast<int>(mInputHistory.size());
			writeLine("] " + line);
			EVENT->emit(ReadEvent({ line }));
		}
	}

	ImGui::PopItemWidth();
	ImGui::SetKeyboardFocusHere();
	ImGui::End();
	showCandidates(height, top);
	ImGui::PopStyleVar(2);

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
	ImGui::SetNextWindowSizeConstraints(ImVec2(0, 0), ImVec2(-1, PLATFORM->getLogicalHeight() - height - top - 10));
	ImGui::SetNextWindowFocus();

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
			mInputState = InputState::Candidates;
			mSelectedCandidate = i;
			mNeedToComplete = true;
		}

		if (auto args = candidate.args; !args.empty())
		{
			ImGui::SameLine();
			ImGui::TextDisabled(args.c_str());
		}

		if (auto description = candidate.description; !description.empty())
		{
			ImGui::SameLine();
			ImGui::TextDisabled(("- " + description).c_str());
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

	ImGui::SetWindowSize(ImVec2(PLATFORM->getLogicalWidth(), 0));
	ImGui::SetWindowPos(ImVec2(0, PLATFORM->getLogicalHeight() - ImGui::GetWindowHeight()));

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
	}

	auto color4 = glm::vec4(color, 1.0f);

	color4 *= colorMultiplier;

	ImGui::TextColored(ImVec4(color4.r, color4.g, color4.b, color4.a), text.text.c_str());

	if (!text.linebreak)
		ImGui::SameLine();
}

void ConsoleDevice::event(const Platform::Keyboard::Event& e)
{
	if (e.type == Platform::Keyboard::Event::Type::Pressed && e.key == Platform::Keyboard::Key::Tilde)
	{
		toggle();
	}
	else if (isOpened() && e.type == Platform::Keyboard::Event::Type::Pressed && e.key == Platform::Keyboard::Key::Escape)
	{
		close();
	}
}

void ConsoleDevice::event(const TouchEmulator::Event& e)
{
	if (mState == State::Closed)
		return;

	if (e.type == TouchEmulator::Event::Type::End)
	{
		mCheckMouseForCandidates = true;
	}
}

void ConsoleDevice::handleInputCompletion(ImGuiTextEditCallbackData* data)
{
	if (mInputState != InputState::Candidates)
		return;

	auto s = mCandidates[mSelectedCandidate].name;

	memcpy(data->Buf, s.data(), s.size());
	data->Buf[s.size()] = 0x00;
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

	for (auto& [name, value] : CONSOLE->getAliases()) // TODO: fix
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
	mInterpolator.setEasingFunction(Common::Easing::ExponentialIn);
	mInterpolator.setFinishCallback([&] { mState = State::Closed; });
}

void ConsoleDevice::open()
{
	PLATFORM->showVirtualKeyboard();

	mState = State::Opening;
	mInterpolator.setStartValue(1.0f);
	mInterpolator.setDestinationValue(0.0f);
	mInterpolator.setPassed(mInterpolator.getDuration() - mInterpolator.getPassed());
	mInterpolator.setEasingFunction(Common::Easing::ExponentialOut);
	mInterpolator.setFinishCallback([&] { mState = State::Opened; });
}