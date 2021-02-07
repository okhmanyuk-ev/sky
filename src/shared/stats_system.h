#pragma once

#include <core/engine.h>
#include <map>
#include <string>
#include <unordered_map>
#include <common/frame_system.h>

#define STATS ENGINE->getSystem<Shared::StatsSystem>()

#define STATS_INDICATE(KEY, VALUE) STATS->indicate(KEY, VALUE)
#define STATS_INDICATE_GROUP(GROUP, KEY, VALUE) STATS->indicate(KEY, VALUE, GROUP)

#define ENGINE_STATS(KEY, VALUE) STATS_INDICATE_GROUP("engine", KEY, VALUE)
#define GAME_STATS(KEY, VALUE) STATS_INDICATE_GROUP("game", KEY, VALUE)

namespace Shared
{
	class StatsSystem : public Common::FrameSystem::Frameable
	{
	public:
		enum class Align
		{
			TopLeft,
			TopRight,
			BottomLeft,
			BottomRight
		};

	public:
		StatsSystem();
		~StatsSystem();

	private:
		void frame() override;

	public:
		void indicate(const std::string& key, const std::string& value, const std::string& group = "");
		void indicate(const std::string& key, int value, const std::string& group = "");
		void indicate(const std::string& key, float value, const std::string& group = "");
		void indicate(const std::string& key, size_t value, const std::string& group = "");
		void indicate(const std::string& key, uint32_t value, const std::string& group = "");

	public:
		auto isEnabled() const { return mEnabled; }
		void setEnabled(bool value) { mEnabled = value; }

		auto getAlignment() const { return mAlignment; }
		void setAlignment(Align value) { mAlignment = value; }

	private:
		bool mEnabled = true;
		Align mAlignment = Align::TopLeft;

	private:
		struct Value
		{
			std::string text;
			Clock::Duration time;
		};

		using Indicators = std::unordered_map<std::string, Value>;
		using Groups = std::unordered_map<std::string, Indicators>;
		Groups mGroups;
	};
}
