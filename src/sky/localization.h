#pragma once

#include <unordered_map>
#include <string>

namespace sky
{
	class Localization
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
		Localization(const std::string& path = "localization");

	public:
		void loadDictionaries(const std::string& path);

		std::wstring getString(const std::string& key) const;
		bool hasLocale(const std::string& key) const;

		auto getLanguage() const { return mLanguage; }
		void setLanguage(Language value) { mLanguage = value; }

	private:
		std::unordered_map<Language, std::unordered_map<std::string, std::wstring>> mDictionaries;
		Language mLanguage = Language::English;
	};
}
