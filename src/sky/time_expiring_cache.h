#pragma once

#include <sky/clock.h>
#include <sky/updatable.h>
#include <unordered_map>

namespace sky
{
	template <typename Key, typename Value>
	class TimeExpiringCache : public sky::Updatable
	{
	public:
		TimeExpiringCache(sky::Duration lifetime) : mLifetime(lifetime)
		{
		}

		void onFrame() override
		{
			const auto now = sky::Now();
			std::erase_if(mItems, [&](const auto& item) {
				const auto& [key, value] = item;
				return now - value.time >= mLifetime;
			});
		}

		bool has(Key key) const
		{
			return mItems.contains(key);
		}

		void insert(Key key, Value value)
		{
			mItems.insert({ key, Item(value, sky::Now()) });
		}

		const Value& get(Key key)
		{
			auto& item = mItems.at(key);
			item.time = sky::Now();
			return item.value;
		}

		const size_t size() const
		{
			return mItems.size();
		}

	private:
		struct Item
		{
			Item(Value value, sky::TimePoint time) : value(value), time(time) {}
			Value value;
			sky::TimePoint time;
		};

		std::unordered_map<Key, Item> mItems;
		sky::Duration mLifetime;
	};
}