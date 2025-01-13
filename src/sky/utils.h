#pragma once

#include <sky/locator.h>
#include <graphics/tex_cell.h>
#include <graphics/font.h>
#include <graphics/animation.h>
#include <graphics/atlas.h>
#include <sky/audio.h>
#include <nlohmann/json.hpp>
#include <sky/dispatcher.h>
#include <console/device.h>

namespace sky
{
	template<class T>
	T* GetService()
	{
		return Locator<T>::GetService();
	}

	Graphics::TexCell GetTexture(const std::string& name);
	std::shared_ptr<Graphics::Font> GetFont(const std::string& name);
	std::shared_ptr<Audio::Sound> GetSound(const std::string& name);
	std::shared_ptr<Graphics::Animation> GetAnimation(const std::string& name);
	std::shared_ptr<Graphics::Atlas> GetAtlas(const std::string& name);
	const nlohmann::json& GetJson(const std::string& name);

	void PrecacheTexture(const std::string& path, std::optional<std::string> name = std::nullopt);
	void PrecacheFont(const std::string& path, std::optional<std::string> name = std::nullopt);

	std::wstring Localize(const std::string& key);

	template<class T>
	void Emit(const T& e)
	{
		GetService<Dispatcher>()->emit(e);
	}

	void PlaySound(std::shared_ptr<Audio::Sound> sound);
	void PlaySound(const std::string& name);

	void Log(const std::string& text);
	void Log(Console::Color color, const std::string& text);

	template<typename... Args>
	void Log(const std::string& text, Args&&... args)
	{
		Log(fmt::format(text, args...));
	}

	template<typename... Args>
	void Log(Console::Color color, const std::string& text, Args&&... args)
	{
		Log(color, fmt::format(text, args...));
	}
}
