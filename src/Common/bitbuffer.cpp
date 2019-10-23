#include "bitbuffer.h"

#include <cassert>

using namespace Common;

uint32_t BitBuffer::readBits(int size)
{
	assert(size >= 0);
	assert(size <= 32);

	if (size == 0)
		return 0;

	if (size == 32)
		return read<uint32_t>();

	uint32_t result = ((1UL << size) - 1) &
		(*(uint32_t*)((size_t)(getMemory()) + getPosition()) >> m_BitPosition);

	int bitCount = m_BitPosition + size;
	int byteCount = bitCount >> 3;

	m_BitPosition = bitCount & 7;
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

	int bitCount = m_BitPosition + size;
	int byteCount = bitCount >> 3;

	bitCount = bitCount & 7;

	if (bitCount == 0)
		byteCount--;

	ensureSpace(byteCount + 1);

	*(uint32_t*)((size_t)getMemory() + getPosition()) = (*(uint32_t*)((size_t)getMemory() +
		getPosition()) & ((1UL << m_BitPosition) - 1)) | (value << m_BitPosition);

	if (bitCount > 0)
		m_BitPosition = bitCount;
	else
		m_BitPosition = 8;

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

	if (m_BitPosition == 0)
		return;

	seek(1);
	m_BitPosition = 0;
}

void BitBuffer::normalizeBitPosition()
{
	while (m_BitPosition >= 8)
	{
		seek(1);
		m_BitPosition -= 8;
	}

	while (m_BitPosition < 0)
	{
		seek(-1);
		m_BitPosition += 8;
	}
}

void BitBuffer::seek(int offset, Buffer::Origin origin)
{
	switch (origin)
	{
	case Origin::Begin:
		m_BitPosition = 0;
		break;

	case Origin::End:
		m_BitPosition = 0;
		break;

	default:
		break;
	}

	Buffer::seek(offset, origin);
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