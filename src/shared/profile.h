#pragma once

#include <nlohmann/json.hpp>
#include <mutex>

namespace Shared
{
	class Profile
	{
	public:
		struct ProfileSavedEvent { };
		struct ProfileClearedEvent { };

	public:
		virtual void read(const nlohmann::json& json) = 0;
		virtual void write(nlohmann::json& json) = 0;
	
	protected:
		virtual void makeDefault() = 0;

	public:
		void load();
		void save();
		void clear();

	private:
		std::mutex mSaveMutex;
	};
}
