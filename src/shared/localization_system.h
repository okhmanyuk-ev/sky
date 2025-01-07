#pragma once

#include <map>
#include <fmt/format.h>
#include <sky/singleton.h>

#define LOCALIZATION sky::Singleton<Shared::LocalizationSystem>::GetInstance()
#define LOCALIZE(KEY) LOCALIZATION->getString(KEY)

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
		LocalizationSystem(const std::string& path = "localization");

	public:
		void loadDicrionaries(const std::string& path);

		std::wstring getString(const std::string& key) const;

		auto getLanguage() const { return mLanguage; }
		void setLanguage(Language value) { mLanguage = value; }

	private:
		std::map<Language, std::map<std::string, std::wstring>> mDictionaries;
		Language mLanguage = Language::English;
	};
}