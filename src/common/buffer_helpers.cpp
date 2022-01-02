#include "buffer_helpers.h"

#include <fstream>
#include <cmath>
#include <cstdint>

using namespace Common;

void BufferHelpers::WriteString(BitBuffer& msg, std::string_view value)
{
	for (auto it = value.begin(); it != value.end(); ++it)
	{
		msg.write<char>(*it);
	}

	msg.write<uint8_t>(0);
}

std::string BufferHelpers::ReadString(BitBuffer& msg)
{
	std::string result;

	while (true)
	{
		char b = msg.read<char>();

		if (b == 0)
			break;

		result += b;
	}

	return result;
}

std::string BufferHelpers::ReadBytesToString(BitBuffer& msg, size_t size)
{
	std::string result;
	result.resize(size);
	msg.read(result.data(), size);
	return result;
}

void BufferHelpers::WriteToBuffer(BitBuffer& from, BitBuffer& to)
{
	int totalBits = static_cast<int>(from.getSize()) * 8;

	if (from.getBitPosition() > 0)
		totalBits -= (8 - from.getBitPosition());

	int byteCount = totalBits >> 3;
	int bitCount = totalBits & 7;

	from.toStart();

	for (int i = 0; i < byteCount; i++)
		to.write<uint8_t>(from.read<uint8_t>());

	to.writeBits(from.readBits(bitCount), bitCount);
}

int32_t BufferHelpers::ReadSBits(BitBuffer& msg, int size)
{
	bool b = msg.readBit();

	int32_t result = msg.readBits(size - 1);

	if (b)
		result = -result;

	return result;
}

void BufferHelpers::WriteSBits(BitBuffer& msg, int32_t value, int size)
{
	msg.writeBit(value < 0);
	msg.writeBits(std::abs(value), size - 1);
}

float BufferHelpers::ReadBitAngle(BitBuffer& msg, int size)
{
	uint32_t x = 1UL << size;

	if (x > 0)
		return msg.readBits(size) * 360.0f / x;
	else
	{
		msg.readBits(size);
		return 0;
	}
}

void BufferHelpers::WriteBitAngle(BitBuffer& msg, float angle, int size)
{
	uint32_t b = 1UL << size;
	msg.writeBits((b - 1) & (int)(std::trunc(b * angle) / 360), size);
}

float BufferHelpers::ReadAngle(BitBuffer& msg)
{
	return msg.read<int8_t>() * (360.0f / 65536);
}

void BufferHelpers::WriteAngle(BitBuffer& msg, float angle)
{
	msg.write<uint8_t>((int64_t)(fmod((double)angle, 360.0) * 256.0 / 360.0) & 0xFF);
}

float BufferHelpers::ReadHiResAngle(BitBuffer& msg)
{
	return msg.read<int16_t>() * (360.0f / 65536);
}

void BufferHelpers::WriteHiResAngle(BitBuffer& msg, float angle)
{
	msg.write<int16_t>((int64_t)(fmod((double)angle, 360.0) * 65536.0 / 360.0) & 0xFFFF);
}

float BufferHelpers::ReadCoord(BitBuffer& msg)
{
	return msg.read<int16_t>() / 8.0f;
}

float BufferHelpers::ReadBitCoord(BitBuffer& msg)
{
	int intData = msg.readBits(1);
	int fracData = msg.readBits(1);

	if (intData != 0 || fracData != 0)
	{
		bool sign = msg.readBit();

		if (intData != 0)
			intData = msg.readBits(12);

		if (fracData != 0)
			fracData = msg.readBits(3);

		float result = fracData * 0.125f + intData;

		if (sign)
			return -result;
		else
			return result;
	}
	else
		return 0.0;
}

void BufferHelpers::ReadBitVec3(BitBuffer& msg, float (&dst)[3])
{
	bool x = msg.readBit();
	bool y = msg.readBit();
	bool z = msg.readBit();

	if (x)
		dst[0] = ReadBitCoord(msg);

	if (y)
		dst[1] = ReadBitCoord(msg);

	if (z)
		dst[2] = ReadBitCoord(msg);
}
