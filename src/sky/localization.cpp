#include "localization.h"
#include <sstream>
#include <cassert>
#include <sky/asset.h>
#include <sky/utils.h>
#include <common/helpers.h>

sky::Localization::Localization(const std::string& path)
{
	loadDictionaries(path);
}

void sky::Localization::loadDictionaries(const std::string& path)
{
	for (auto language : { Language::English, Language::Russian })
	{
		auto& dictionary = mDictionaries[language];
		auto _path = path + "/" + getLanguageName(language) + ".txt";

		if (!sky::Asset::Exists(_path))
			return;

		auto asset = sky::Asset(_path);

		auto s = sky::to_wstring(std::string((char*)asset.getMemory(), asset.getSize()));
		auto ss = std::wstringstream(s);

		auto trim = [](std::wstring& s) {
			auto ws = L" \t\n\r\f\v";
			s.erase(s.find_last_not_of(ws) + 1);
			s.erase(0, s.find_first_not_of(ws));
		};

		while (std::getline(ss, s))
		{
			auto separator_pos = s.find(L":");

			if (separator_pos == std::string::npos)
				continue;

			auto key = s.substr(0, separator_pos);
			auto value = s.substr(separator_pos + 1);

			trim(key);
			trim(value);

			if (key.empty() || value.empty())
				continue;

			dictionary[sky::to_string(key)] = value;
		}
	}
}

std::string sky::Localization::getLanguageName(Language language)
{
	if (language == Language::English)
		return "english";
	else if (language == Language::Russian)
		return "russian";

	assert(!"unknown language");
	return "";
}

std::wstring sky::Localization::getString(const std::string& key) const
{
	const auto& dictionary = mDictionaries.at(mLanguage);

	if (!dictionary.contains(key))
	{
		sky::Log(Console::Color::Red, "cannot find locale: " + key);
		return sky::to_wstring(key);
	}

	return dictionary.at(key);
}

bool sky::Localization::hasLocale(const std::string& key) const
{
	const auto& dictionary = mDictionaries.at(mLanguage);
	return dictionary.contains(key);
}
