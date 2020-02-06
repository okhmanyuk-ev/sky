#include "localization_system.h"

#include <Platform/asset.h>

#include <sstream>
#include <cassert>

using namespace Shared;

void LocalizationSystem::loadDicrionaries(const std::string& path)
{
	auto loadDictionary = [this, path](Language language) {
		auto& dictionary = mDictionaries[language];

		auto asset = Platform::Asset(path + "/" + getLanguageName(language) + ".txt");

		auto s = std::string((char*)asset.getMemory(), asset.getSize());
		auto ss = std::stringstream(s);

		auto trim = [](std::string& s) {
			const char* ws = " \t\n\r\f\v";
			s.erase(s.find_last_not_of(ws) + 1);
			s.erase(0, s.find_first_not_of(ws));
		};

		while (std::getline(ss, s))
		{
			auto separator_pos = s.find(":");

			if (separator_pos == std::string::npos)
				continue;

			auto key = s.substr(0, separator_pos);
			auto value = s.substr(separator_pos + 1);

			trim(key);
			trim(value);

			if (key.empty() || value.empty())
				continue;

			dictionary[key] = value;
		}
	};

	loadDictionary(Language::English);
	loadDictionary(Language::Russian);
}

std::string LocalizationSystem::getLanguageName(Language language)
{
	if (language == Language::English)
		return "english";
	else if (language == Language::Russian)
		return "russian";

	assert(!"unknown language");
	return "";
}

utf8_string LocalizationSystem::getString(const std::string& key) const
{
	auto& dictionary = mDictionaries.at(mLanguage);

	if (dictionary.count(key) == 0)
		return key;

	return dictionary.at(key);
}