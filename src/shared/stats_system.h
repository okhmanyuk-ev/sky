#pragma once

#include <string>
#include <unordered_map>
#include <sky/scheduler.h>

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
		void indicator(const std::string& group, const std::string& key, const std::string& value);

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
