#include "size_converter.h"

using namespace Common;

std::string SizeConverter::ToString(uint64_t value) // we need uint128_t here
{
	int q = 0;
	std::string l = "b";

	if (value < 1ULL << 10) {
		//
	}
	else if (value < 1ULL << 20) {
		q = 10;
		l = "kb";
	}
	else if (value < 1ULL << 30) {
		q = 20;
		l = "mb";
	}
	else if (value < 1ULL << 40) {
		q = 30;
		l = "gb";
	}
	else if (value < 1ULL << 50) {
		q = 40;
		l = "tb";
	}
	else if (value < 1ULL << 60) {
		q = 50;
		l = "pb";
	}
	/*else if (value < 1ULL << 70) {
	q = 60;
	l = "eb";
	}
	else if (value < 1ULL << 80) {
	q = 70;
	l = "zb";
	}
	else if (value < 1ULL << 90) {
	q = 80;
	l = "yb";
	}*/

	if (q == 0)
		return std::to_string(value) + " " + l;

	return std::to_string(value / (1ULL << q)) + "." + std::to_string(value % (1ULL << q) / ((1ULL << q) / 10 + 1)) + " " + l;
}