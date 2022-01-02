#pragma once

#include "bitbuffer.h"

#include <string>
#include <string_view>

namespace Common::BufferHelpers
{
	void WriteString(BitBuffer& msg, std::string_view value);
	std::string ReadString(BitBuffer& msg);
	std::string ReadBytesToString(BitBuffer& msg, size_t size);

	void WriteToBuffer(BitBuffer& from, BitBuffer& to);

	int32_t ReadSBits(BitBuffer& msg, int size);
	void WriteSBits(BitBuffer& msg, int32_t value, int size);

	float ReadBitAngle(BitBuffer& msg, int size);
	void WriteBitAngle(BitBuffer& msg, float angle, int size);

	float ReadAngle(BitBuffer& msg);
	void WriteAngle(BitBuffer& msg, float angle);

	float ReadHiResAngle(BitBuffer& msg);
	void WriteHiResAngle(BitBuffer& msg, float angle);

	float ReadCoord(BitBuffer& msg);

	float ReadBitCoord(BitBuffer& msg);

	void ReadBitVec3(BitBuffer& msg, float (&dst)[3]);
}