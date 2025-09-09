#include "utils.h"
#include <sky/cache.h>
#include <sky/localization.h>
#include <codecvt>
#include <regex>
#ifdef EMSCRIPTEN
#include <emscripten.h>
#endif

Graphics::TexturePart sky::GetTexture(const std::string& name)
{
	return GetService<Cache>()->getTexture(name);
}

std::shared_ptr<Graphics::Font> sky::GetFont(const std::string& name)
{
	return GetService<Cache>()->getFont(name);
}

std::shared_ptr<sky::Audio::Sound> sky::GetSound(const std::string& name)
{
	return GetService<Cache>()->getSound(name);
}

std::shared_ptr<Graphics::Animation> sky::GetAnimation(const std::string& name)
{
	return GetService<Cache>()->getAnimation(name);
}

std::shared_ptr<Graphics::Atlas> sky::GetAtlas(const std::string& name)
{
	return GetService<Cache>()->getAtlas(name);
}

const nlohmann::json& sky::GetJson(const std::string& name)
{
	return GetService<Cache>()->getJson(name);
}

void sky::PrecacheTexture(const std::string& path, std::optional<std::string> name)
{
	GetService<Cache>()->loadTexture(path, name);
}

void sky::PrecacheFont(const std::string& path, std::optional<std::string> name)
{
	GetService<Cache>()->loadFont(path, name);
}

std::wstring sky::Localize(const std::string& key)
{
	return GetService<Localization>()->getString(key);
}

std::wstring sky::UnfoldLocaleTags(const std::wstring& str)
{
	std::wregex pattern(LR"(<loc=([^>]+)>)");
	std::wstring result;
	std::wsregex_iterator it(str.begin(), str.end(), pattern);
	std::wsregex_iterator end;
	size_t last_pos = 0;
	for (; it != end; ++it) {
		const auto& match = *it;
		auto start = match.position();
		auto length = match.length();
		auto key = match[1].str();
		result += str.substr(last_pos, start - last_pos);
		result += Localize(sky::to_string(key));
		last_pos = start + length;
	}
	result += str.substr(last_pos);

	if (std::regex_search(result, pattern))
		return UnfoldLocaleTags(result);

	return result;
}

void sky::PlaySound(std::shared_ptr<Audio::Sound> sound)
{
	if (!HasService<Audio>())
	{
		Log("PlaySound: cannot find audio system");
		return;
	}
	GetService<Audio>()->play(sound);
}

void sky::PlaySound(const std::string& name)
{
	PlaySound(GetSound(name));
}

void sky::Log(const std::string& text)
{
	if (!HasService<Console>())
		return;

	GetService<Console>()->writeLine(text);
}

void sky::Log(Console::Color color, const std::string& text)
{
	if (!HasService<Console>())
		return;

	GetService<Console>()->writeLine(text, color);
}

void sky::AddCommand(const std::string& name, CommandProcessor::Command command)
{
	GetService<CommandProcessor>()->addItem(name, command);
}

void sky::AddCVar(const std::string& name, CommandProcessor::CVar cvar)
{
	GetService<CommandProcessor>()->addItem(name, cvar);
}

void sky::ExecuteCommand(const std::string& str)
{
	GetService<CommandProcessor>()->execute(str);
}

void sky::OpenUrl(const std::string& url)
{
#if defined(EMSCRIPTEN)
	EM_ASM_({
		var url = UTF8ToString($0);
		window.open(url);
	}, url.c_str());
#elif defined(PLATFORM_WINDOWS)
	std::system(("start " + url).c_str());
#endif
}

void sky::Schedule(Action action)
{
	sky::Scheduler::Instance->run([](auto action) -> Task<> {
		ActionsPlayer player;
		player.add(std::move(action));
		while (player.hasActions())
		{
			player.update(sky::Scheduler::Instance->getTimeDelta());
			co_await std::suspend_always{};
		}
	}(action));
}

std::string sky::to_string(const std::wstring& wstr)
{
	std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
	return converter.to_bytes(wstr);
}

std::wstring sky::to_wstring(const std::string& str)
{
	std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
	return converter.from_bytes(str);
}

std::string sky::join(const std::vector<std::string>& vec, const std::string& delimiter)
{
	std::string result;
	for (size_t i = 0; i < vec.size(); i++)
	{
		result += vec.at(i);
		if (i != vec.size() - 1)
		{
			result += delimiter;
		}
	}
	return result;
}
