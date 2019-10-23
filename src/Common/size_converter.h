#pragma once

#include <string>

namespace Common
{
	class SizeConverter
	{
	public:
		static std::string ToString(uint64_t value);

	private:
		SizeConverter();
		SizeConverter(const SizeConverter&) = delete;
		SizeConverter(SizeConverter&&) = delete;
		void operator=(const SizeConverter&) = delete;
		void operator=(SizeConverter&&) = delete;
	};
}