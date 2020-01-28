#include "bitbuffer.h"

#include <cassert>
#include <string>

using namespace Common;

namespace
{
	uint32_t nextPowerOfTwo(uint32_t n)
	{
		--n;

		n |= n >> 1;
		n |= n >> 2;
		n |= n >> 4;
		n |= n >> 8;
		n |= n >> 16;

		return n + 1;
	}
}

using namespace Common;

BitBuffer::~BitBuffer()
{
	setCapacity(0);
}

void BitBuffer::clear()
{
	setSize(0);
	toStart();
}

void BitBuffer::toStart()
{
	seek(0, Origin::Begin);
}

void BitBuffer::toEnd()
{
	seek(0, Origin::End);
}

void BitBuffer::fill(uint8_t value)
{
	memset(mMemory, value, mSize);
}

void BitBuffer::ensureSpace(size_t value)
{
	if (mSize < mPosition + value)
	{
		setSize(mPosition + value);
	}
}

void BitBuffer::setSize(size_t value)
{
	if (value > getCapacity())
		setCapacity(value);

	mSize = value;
}

void BitBuffer::setCapacity(size_t value)
{
	value *= 2;

	if (mCapacity >= value)
		return;
	
	mCapacity = value;
	mMemory = realloc(mMemory, mCapacity);
}

uint32_t BitBuffer::readBits(int size)
{
	assert(size >= 0);
	assert(size <= 32);

	if (size == 0)
		return 0;

	if (size == 32)
		return read<uint32_t>();

	uint32_t result = ((1UL << size) - 1) &
		(*(uint32_t*)((size_t)(getMemory()) + getPosition()) >> mBitPosition);

	int bitCount = mBitPosition + size;
	int byteCount = bitCount >> 3;

	mBitPosition = bitCount & 7;
	seek(byteCount);

	return result;
}

void BitBuffer::writeBits(uint32_t value, int size)
{
	assert(size >= 0);
	assert(size <= 32);

	if (size == 0)
		return;

	if (size == 32)
	{
		write<uint32_t>(value);
		return;
	}

	uint32_t maxValue = (1UL << size) - 1;

	if (value > maxValue)
		value = maxValue;

	int bitCount = mBitPosition + size;
	int byteCount = bitCount >> 3;

	bitCount = bitCount & 7;

	if (bitCount == 0)
		byteCount--;

	ensureSpace(byteCount + 1);

	*(uint32_t*)((size_t)getMemory() + getPosition()) = (*(uint32_t*)((size_t)getMemory() +
		getPosition()) & ((1UL << mBitPosition) - 1)) | (value << mBitPosition);

	if (bitCount > 0)
		mBitPosition = bitCount;
	else
		mBitPosition = 8;

	seek(byteCount);

	normalizeBitPosition();
}

bool BitBuffer::readBit()
{
	return readBits(1) > 0;
}

void BitBuffer::writeBit(bool value)
{
	writeBits(value ? 1 : 0, 1);
}

int BitBuffer::bitsFor(uint32_t value)
{
	for (int i = 1; i < 32; i++)
	{
		if ((1UL << i) - 1 < value)
			continue;

		return i;
	}

	return 0;
}

uint32_t BitBuffer::readBitsFor(uint32_t max)
{
	return readBits(bitsFor(max));
}

void BitBuffer::writeBitsFor(uint32_t value, uint32_t max)
{
	writeBits(value, bitsFor(max));
}

uint32_t BitBuffer::readBitsVar()
{
	return readBits(readBitsFor(31)); // TODO: maybe we should write "32" in second argument ?
}

void BitBuffer::writeBitsVar(uint32_t value)
{
	writeBitsFor(bitsFor(value), 31); // TODO: maybe we should write "32" in second argument ?
	writeBitsFor(value, value);
}

void BitBuffer::alignByteBoundary()
{
	normalizeBitPosition();

	if (mBitPosition == 0)
		return;

	seek(1);
	mBitPosition = 0;
}

void BitBuffer::normalizeBitPosition()
{
	while (mBitPosition >= 8)
	{
		seek(1);
		mBitPosition -= 8;
	}

	while (mBitPosition < 0)
	{
		seek(-1);
		mBitPosition += 8;
	}
}

void BitBuffer::seek(int offset, Origin origin)
{
	switch (origin)
	{
	case Origin::Begin:
		mPosition = offset;
		mBitPosition = 0;
		break;

	case Origin::Current:
		mPosition += offset;
		break;

	case Origin::End:
		mPosition = mSize - offset;
		mBitPosition = 0;
		break;
	}
}

void BitBuffer::read(void* value, size_t size)
{
	for (size_t i = 0; i < size; i++)
		*(uint8_t*)((size_t)value + i) = (uint8_t)readBits(8);
}

void BitBuffer::write(void* value, size_t size)
{
	for (size_t i = 0; i < size; i++)
		writeBits((uint32_t)(*(uint8_t*)((size_t)value + i)), 8);
}