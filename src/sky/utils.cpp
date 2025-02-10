#include "utils.h"
#include <sky/cache.h>
#include <sky/localization.h>
#include <codecvt>

Graphics::TexCell sky::GetTexture(const std::string& name)
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
	GetService<Console>()->writeLine(text);
}

void sky::Log(Console::Color color, const std::string& text)
{
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
