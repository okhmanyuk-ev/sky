#pragma once

#include <map>
#include <string>
#include <unordered_map>
#include <sky/scheduler.h>
#include <fmt/format.h>

#define STATS sky::Locator<Shared::StatsSystem>::Get()

#define STATS_INDICATE(KEY, VALUE) STATS->indicate(KEY, VALUE)
#define STATS_INDICATE_GROUP(GROUP, KEY, VALUE) STATS->indicate(KEY, VALUE, GROUP)

#define ENGINE_STATS(KEY, VALUE) STATS_INDICATE_GROUP("engine", KEY, VALUE)
#define GAME_STATS(KEY, VALUE) STATS_INDICATE_GROUP("game", KEY, VALUE)

namespace Shared
{
	class StatsSystem : public sky::Scheduler::Frameable
	{
	public:
		enum class Align
		{
			TopLeft,
			TopRight,
			BottomLeft,
			BottomRight
		};

	private:
		void onFrame() override;

	public:
		template <class T>
		void indicate(const std::string& key, const T& value, const std::string& group = "")
		{
			mGroups[group][key] = { fmt::format("{}", value), SCHEDULER->getUptime()};
		}

	public:
		auto getAlignment() const { return mAlignment; }
		void setAlignment(Align value) { mAlignment = value; }

		bool isEnabled() const { return mEnabled; }
		void setEnabled(bool value) { mEnabled = value; }

	private:
		sky::CVar<bool> mEnabled = sky::CVar<bool>("g_stats", true);
		Align mAlignment = Align::TopLeft;

	private:
		struct Value
		{
			std::string text;
			sky::Duration time;
		};

		using Indicators = std::unordered_map<std::string, Value>;
		using Groups = std::unordered_map<std::string, Indicators>;
		Groups mGroups;
	};
}
