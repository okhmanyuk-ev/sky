#pragma once

#include "bitbuffer.h"

#include <string>
#include <string_view>

namespace Common::BufferHelpers
{
	void WriteString(Common::BitBuffer& msg, std::string_view value);
	std::string ReadString(Common::BitBuffer& msg);

	void WriteToBuffer(BitBuffer& from, BitBuffer& to);















	int32_t ReadSBits(Common::BitBuffer& msg, int size);
	void WriteSBits(Common::BitBuffer& msg, int32_t value, int size);

	float ReadBitAngle(Common::BitBuffer& msg, int size);
	void WriteBitAngle(Common::BitBuffer& msg, float angle, int size);

	float ReadAngle(Common::BitBuffer& msg);
	void WriteAngle(Common::BitBuffer& msg, float angle);

	float ReadHiResAngle(Common::BitBuffer& msg);
	void WriteHiResAngle(Common::BitBuffer& msg, float angle);

	float ReadCoord(Common::BitBuffer& msg);

	float ReadBitCoord(Common::BitBuffer& msg);

	void ReadBitVec3(Common::BitBuffer& msg, float (&dst)[3]);
}