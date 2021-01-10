#include "bitbuffer.h"

#include <cassert>
#include <string>
#include <stdexcept>

using namespace Common;

BitBuffer::BitBuffer()
{
	//
}

BitBuffer::BitBuffer(BitBuffer& bitbuffer) : BitBuffer()
{
	write(bitbuffer.getMemory(), bitbuffer.getSize());
	setPosition(bitbuffer.getPosition());
	setBitPosition(bitbuffer.getBitPosition());
}

BitBuffer::~BitBuffer()
{
	if (mMemory != nullptr)
		free(mMemory);
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
	value += mPosition;

	if (mSize >= value)
		return;

	setSize(value);
}

void BitBuffer::setSize(size_t value)
{
	ensureCapacity(value + 4);
	mSize = value;
}

void BitBuffer::ensureCapacity(size_t value)
{
	if (mCapacity >= value)
		return;

	mCapacity = value * 2;
	mMemory = realloc(mMemory, mCapacity);
}

uint32_t BitBuffer::readBits(int size)
{
	if (!hasRemaining())
		throw std::runtime_error("readBits: no remaining space");

	assert(size >= 0);
	assert(size <= 32);

	if (size == 0)
		return 0;

	if (size == 32)
		return read<uint32_t>();

	auto& src_value = *(uint32_t*)((size_t)mMemory + mPosition);
	auto max_value = (1U << size) - 1;

	auto result = max_value & (src_value >> mBitPosition);

	int bit_count = mBitPosition + size;
	int byte_count = bit_count >> 3;

	mBitPosition = bit_count & 7;
	seek(byte_count);

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

	auto max_value = (1U << size) - 1;

	if (value > max_value)
		value = max_value;

	int bit_count = mBitPosition + size;
	int byte_count = bit_count >> 3;

	bit_count = bit_count & 7;

	if (bit_count == 0)
		byte_count--;

	ensureSpace(byte_count + 1);

	auto& src_value = *(uint32_t*)((size_t)mMemory + mPosition);
	auto row = (1U << mBitPosition) - 1;

	src_value = (src_value & row) | (value << mBitPosition);

	if (bit_count > 0)
		mBitPosition = bit_count;
	else
		mBitPosition = 8;

	seek(byte_count);

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
	return readBits(readBitsFor(31)); // TODO: 31 -> 32 ?
}

void BitBuffer::writeBitsVar(uint32_t value)
{
	writeBitsFor(bitsFor(value), 31); // TODO: 31 -> 32 ?
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
