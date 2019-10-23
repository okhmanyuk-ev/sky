#pragma once

#include "buffer.h"

namespace Common
{
	class BitBuffer : public Buffer
	{
	public:
		uint32_t readBits(int size);
		void writeBits(uint32_t value, int size);

		bool readBit();
		void writeBit(bool value);

		static int bitsFor(uint32_t value);

		uint32_t readBitsFor(uint32_t max);
		void writeBitsFor(uint32_t value, uint32_t max);

		uint32_t readBitsVar();
		void writeBitsVar(uint32_t value);

		void alignByteBoundary();
		void normalizeBitPosition();

		void seek(int offset, Buffer::Origin origin = Buffer::Origin::Current) override;

		void read(void* value, size_t size) override;
		void write(void* value, size_t size) override;

		template <typename T> T read() { T result; read(&result, sizeof(T)); return result; };
		template <typename T> void write(const T& value) { write((void*)&value, sizeof(T)); };

	public:
		int getBitPosition() { return m_BitPosition; }
		void setBitPosition(int value) { m_BitPosition = value; }

		int getRemainingBits() { if (!hasRemaining()) return 0; return static_cast<int>(getRemaining()) * 8 - getBitPosition(); }

	private:
		int m_BitPosition = 0;
		// m_BitSize;
	};
}