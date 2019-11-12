#pragma once

#include <string>

namespace Common
{
	inline std::wstring ToWideString(const std::string& text)
	{
		return std::wstring(text.begin(), text.end());
	}

}