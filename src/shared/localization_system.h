#pragma once

#include <core/engine.h>
#include <map>
#include <tinyutf8.hpp>
#include <fmt/format.h>

#define LOCALIZATION ENGINE->getSystem<Shared::LocalizationSystem>()

#define LOCALIZE(KEY) LOCALIZATION->getString(KEY)
#define LOCALIZE_FMT(KEY, ...) LOCALIZATION->getStringFmt(KEY, __VA_ARGS__)

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
		static std::string getLanguageName(Language language);

	public:
		LocalizationSystem();

	public:
		void loadDicrionaries(const std::string& path);

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