#pragma once

#include <sky/locator.h>
#include <renderer/all.h>
#include <graphics/all.h>
#include <audio/sound.h>
#include <nlohmann/json.hpp>

namespace sky
{
	template<class T> T* GetService() { return Locator<T>::GetService(); }

	Graphics::TexCell GetTexture(const std::string& name);
	std::shared_ptr<Graphics::Font> GetFont(const std::string& name);
	std::shared_ptr<Audio::Sound> GetSound(const std::string& name);
	std::shared_ptr<Graphics::Animation> GetAnimation(const std::string& name);
	std::shared_ptr<Graphics::Atlas> GetAtlas(const std::string& name);
	const nlohmann::json& GetJson(const std::string& name);

	void PrecacheTexture(const std::string& name, std::optional<std::string> alias = std::nullopt);
	void PrecacheFont(const std::string& name, std::optional<std::string> alias = std::nullopt);

	std::wstring Localize(const std::string& key);
}
