#include "utils.h"
#include <sky/cache.h>

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

void sky::PrecacheTexture(const std::string& name, std::optional<std::string> alias)
{
	GetService<Cache>()->loadTexture(name, alias.value_or(name));
}

void sky::PrecacheFont(const std::string& name, std::optional<std::string> alias)
{
	GetService<Cache>()->loadFont(name, alias.value_or(name));
}
