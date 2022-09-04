#include "helpers.h"

#include <cstddef>
#include <cstdint>
#include <common/bitbuffer.h>

using namespace Common;

std::string Helpers::SecondsToFmtString(int seconds)
{
	int h = seconds / 60 / 60;
	int m = (seconds / 60) % 60;
	int s = seconds % 60;

	return fmt::format("{:02}:{:02}:{:02}", h, m, s);
}

bool Helpers::Chance(float normalized_percent)
{
	return glm::linearRand(0.0f, 1.0f) <= normalized_percent;
}

std::string Helpers::BytesToNiceString(uint64_t value) // we need uint128_t here
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

std::string Helpers::BytesArrayToNiceString(void* mem, size_t size)
{
	BitBuffer buf;
	buf.write(mem, size);
	buf.toStart();
	std::string result;
	while (buf.hasRemaining())
	{
		auto b = buf.read<uint8_t>();
		//if (b > 30)
		if (std::isprint(b))
			result += (unsigned char)b;
		else
			result += "#" + std::to_string(b);
	}
	return result;
}

/*uint32_t Helpers::crc32c(const uint8_t* mem, size_t size, uint32_t crc)
{
	int k;

	// CRC-32C (iSCSI) polynomial in reversed bit order. 
	const auto POLY = 0x82f63b78;

	crc = ~crc;
	while (size--) {
		crc ^= *mem++;
		for (k = 0; k < 8; k++)
			crc = crc & 1 ? (crc >> 1) ^ POLY : crc >> 1;
	}
	return ~crc;
}*/

uint32_t crc32_for_byte(uint32_t r) 
{
	for (int j = 0; j < 8; ++j)
		r = (r & 1 ? 0 : (uint32_t)0xEDB88320L) ^ r >> 1;

	return r ^ (uint32_t)0xFF000000L;
}

uint32_t Helpers::crc32(void* data, size_t size, uint32_t initial) 
{
	static uint32_t table[0x100];

	if (!*table)
	{
		for (size_t i = 0; i < 0x100; ++i)
			table[i] = crc32_for_byte(i);
	}

	uint32_t crc = initial;
		
	for (size_t i = 0; i < size; ++i)
		crc = table[(uint8_t)crc ^ ((uint8_t*)data)[i]] ^ crc >> 8;

	return crc;
}

nlohmann::json Helpers::LoadJsonFromAsset(const Platform::Asset& asset)
{
	auto json_string = std::string((char*)asset.getMemory(), asset.getSize());
	return nlohmann::json::parse(json_string);
}

nlohmann::json Helpers::LoadBsonFromAsset(const Platform::Asset& asset)
{
	return nlohmann::json::from_bson(std::string((char*)asset.getMemory(), asset.getSize()));
}

float Helpers::SmoothRotationAssign(float src_radians, float dst_radians, Clock::Duration dTime, float friction)
{
	auto src_deg = glm::degrees(src_radians);
	auto dst_deg = glm::degrees(dst_radians);

	auto distance = glm::distance(src_deg, dst_deg);

	if (distance > glm::distance(src_deg - 360.0f, dst_deg))
		src_deg -= 360.0f;
	else if (distance > glm::distance(src_deg + 360.0f, dst_deg))
		src_deg += 360.0f;

	src_radians = glm::radians(src_deg);

	return SmoothValueAssign(src_radians, dst_radians, dTime, friction);
}