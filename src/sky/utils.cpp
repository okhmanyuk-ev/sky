#include "utils.h"
#include <sky/cache.h>
#include <sky/localization.h>
#include <audio/system.h>

Graphics::TexCell sky::GetTexture(const std::string& name)
{
	return GetService<Cache>()->getTexture(name);
}

std::shared_ptr<Graphics::Font> sky::GetFont(const std::string& name)
{
	return GetService<Cache>()->getFont(name);
}

std::shared_ptr<Audio::Sound> sky::GetSound(const std::string& name)
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
	if (!sky::Locator<Audio::System>::HasService())
	{
		Log("PlaySound: cannot find audio system");
		return;
	}

	AUDIO->play(sound);
}

void sky::Log(const std::string& text)
{
	CONSOLE_DEVICE->writeLine(text);
}

void sky::Log(Console::Color color, const std::string& text)
{
	CONSOLE_DEVICE->writeLine(text, color);
}
