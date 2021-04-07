#pragma once

#include <cstdint>
#include <cstddef>

class BitBuffer
{
public:
	enum class Origin
	{
		Begin,
		Current,
		End
	};

public:
	BitBuffer();
	BitBuffer(const BitBuffer& bitbuffer);
	~BitBuffer();

public:
	void clear();
	void toStart();
	void toEnd();

	void fill(uint8_t value);
	void ensureSpace(size_t value);
	void ensureCapacity(size_t value);

	uint32_t readBits(uint32_t size);
	void writeBits(uint32_t value, uint32_t size);

	bool readBit();
	void writeBit(bool value);

	static uint32_t BitsFor(uint32_t value);

	uint32_t readBitsFor(uint32_t max);
	void writeBitsFor(uint32_t value, uint32_t max);

	uint32_t readBitsVar();
	void writeBitsVar(uint32_t value);

	void alignByteBoundary();
	void normalizeBitPosition();

	void seek(int offset, Origin origin = Origin::Current);

	void read(void* value, size_t size);
	void write(void* value, size_t size);

	template <typename T> T read() { T result; read(&result, sizeof(T)); return result; };
	template <typename T> void write(const T& value) { write((void*)&value, sizeof(T)); };

public:
	auto getMemory() const { return mMemory; }

	auto getSize() const { return mSize; }
	void setSize(size_t value);

	auto getPosition() const { return mPosition; }
	void setPosition(size_t value) { mPosition = value; };

	auto getRemaining() const { return mSize - mPosition; };
	bool hasRemaining() { return getRemaining() > 0; }

	auto getCapacity() const { return mCapacity; }

	int getBitPosition() const { return mBitPosition; }
	void setBitPosition(int value) { mBitPosition = value; }

	int getRemainingBits() { if (!hasRemaining()) return 0; return static_cast<int>(getRemaining()) * 8 - getBitPosition(); }

private:
	void* mMemory = nullptr;
	size_t mSize = 0;
	size_t mPosition = 0;
	size_t mCapacity = 0;
	int mBitPosition = 0;
};