#pragma once

#include <Core/engine.h>
#include <map>
#include <tinyutf8.hpp>
#include <fmt/format.h>

#define LOCALIZATION_SYSTEM_KEY "localization_system"
#define LOCALIZATION static_cast<Shared::LocalizationSystem*>(ENGINE->getCustomSystem(LOCALIZATION_SYSTEM_KEY))

#define LOCALIZE(KEY, ...) LOCALIZATION->getStringFmt(KEY, __VA_ARGS__)

namespace Shared
{
	class LocalizationSystem
	{
	public:
		enum class Language
		{
			English,
			Russian
		};

	public:
		LocalizationSystem(const std::string& path);
		~LocalizationSystem();

	public:
		static std::string getLanguageName(Language language);

	public:
		utf8_string getString(const std::string& key) const;
		
		auto getLanguage() const { return mLanguage; }
		void setLanguage(Language value) { mLanguage = value; }

	private:
		std::map<Language, std::map<std::string, utf8_string>> mDictionaries;
		Language mLanguage = Language::English;

	public:
		template <typename... Args> 
		utf8_string getStringFmt(const std::string& key, Args&&... args)
		{
			return fmt::format(getString(key).cpp_str(), args...);
		}
	};
}