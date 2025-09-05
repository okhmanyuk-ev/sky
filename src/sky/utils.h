#pragma once

#include <sky/locator.h>
#include <graphics/texture_part.h>
#include <graphics/font.h>
#include <graphics/animation.h>
#include <graphics/atlas.h>
#include <sky/audio.h>
#include <nlohmann/json.hpp>
#include <sky/dispatcher.h>
#include <sky/console.h>
#include <fmt/format.h>
#include <fmt/xchar.h>
#include <shared/stats_system.h>
#include <sky/actions.h>

namespace sky
{
	// clang-format off

	template <class T> const std::shared_ptr<T>& GetService() { return Locator<T>::Get(); }
	template <class T> bool HasService() { return Locator<T>::Exists(); }

	// clang-format on

	Graphics::TexturePart GetTexture(const std::string& name);
	std::shared_ptr<Graphics::Font> GetFont(const std::string& name);
	std::shared_ptr<Audio::Sound> GetSound(const std::string& name);
	std::shared_ptr<Graphics::Animation> GetAnimation(const std::string& name);
	std::shared_ptr<Graphics::Atlas> GetAtlas(const std::string& name);
	const nlohmann::json& GetJson(const std::string& name);

	void PrecacheTexture(const std::string& path, std::optional<std::string> name = std::nullopt);
	void PrecacheFont(const std::string& path, std::optional<std::string> name = std::nullopt);

	std::wstring Localize(const std::string& key);
	std::wstring UnfoldLocaleTags(const std::wstring& str);

	template <class T>
	void Emit(const T& e)
	{
		GetService<Dispatcher>()->emit(e);
	}

	void PlaySound(std::shared_ptr<Audio::Sound> sound);
	void PlaySound(const std::string& name);

	void Log(const std::string& text);
	void Log(Console::Color color, const std::string& text);

	void AddCommand(const std::string& name, CommandProcessor::Command command);
	void AddCVar(const std::string& name, CommandProcessor::CVar cvar);

	void ExecuteCommand(const std::string& str);

	void OpenUrl(const std::string& url);

	void Schedule(sky::Scheduler::StatusCallback callback);
	void Schedule(std::unique_ptr<Actions::Action> action);

	enum class ScheduleBehavior
	{
		Once,
		Everytime
	};

	void Schedule(ScheduleBehavior behavior, std::function<void()> callback);

	std::string to_string(const std::wstring& wstr);
	std::wstring to_wstring(const std::string& str);

	template <typename String, typename... Args>
	auto format(String&& str, Args&&... args)
	{
		return fmt::format(fmt::runtime(std::forward<String>(str)), std::forward<Args>(args)...);
	}

	template <typename... Args>
	void Log(const std::string& text, Args&&... args)
	{
		Log(format(text, args...));
	}

	template <typename... Args>
	void Log(Console::Color color, const std::string& text, Args&&... args)
	{
		Log(color, format(text, args...));
	}

	template <typename T>
	void Indicator(const std::string& group, const std::string& key, const T& value)
	{
		GetService<Shared::StatsSystem>()->indicator(group, key, format("{}", value));
	}

	template <typename T>
	void Indicator(const std::string& key, const T& value)
	{
		Indicator("common", key, value);
	}

	namespace ranges
	{
		inline auto wrap(const std::string& a, const std::string& b)
		{
			return std::views::transform([&](const auto& s) { return a + s + b; });
		};
	}

	std::string join(const std::vector<std::string>& vec, const std::string& delimiter);

	template <class... Ts>
	struct cases : Ts...
	{
		using Ts::operator()...;
	};
	template <class... Ts>
	cases(Ts...) -> cases<Ts...>;

	template <typename V, typename Callable>
	decltype(auto) match(V&& v, Callable&& c)
	{
		return std::visit(std::forward<Callable>(c), std::forward<V>(v));
	}
}
