#include "buffer.h"

#include <string>

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

	Buffer::~Buffer()
	{
		setCapacity(0);
	}

	void Buffer::clear()
	{
		setSize(0);
		toStart();
	}

	void Buffer::seek(int offset, Origin origin)
	{
		switch (origin)
		{
		case Origin::Begin:
			mPosition = offset;
			break;

		case Origin::Current:
			mPosition += offset;
			break;

		case Origin::End:
			mPosition = mSize - offset;
			break;
	}
}

void Buffer::toStart()
{
	seek(0, Origin::Begin);
}

void Buffer::toEnd()
{
	seek(0, Origin::End);
}

void Buffer::read(void* value, size_t size)
{
	memmove(value, (void*)(((size_t)mMemory) + mPosition), size);
	mPosition += size;
}

void Buffer::write(void* value, size_t size)
{
	ensureSpace(size);
	memmove((void*)(((size_t)mMemory) + mPosition), value, size);
	mPosition += size;
}

void Buffer::fill(uint8_t value)
{
	memset(mMemory, value, mSize);
}

void Buffer::ensureSpace(size_t value)
{
	if (mSize < mPosition + value)
	{
		setSize(mPosition + value);
	}
}

void Buffer::setSize(size_t value)
{
	if (value + (mBlockSize / 2) > mCapacity)
		setCapacity(value + (mBlockSize / 2));

	mSize = value;
}

void Buffer::setCapacity(size_t value)
{
	value = (value + (mBlockSize - 1)) & ~(mBlockSize - 1);
	if (mCapacity == value)
	{
		return;
	}
	mCapacity = value;
	mMemory = realloc(mMemory, mCapacity);
}