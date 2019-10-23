#pragma once

#include <cstdint>
#include <cstdio>

namespace Common
{
	class Buffer
	{
	public:
		enum class Origin
		{
			Begin,
			Current,
			End
		};

	public:
		virtual ~Buffer();

		void clear();

		virtual void seek(int offset, Origin origin = Origin::Current);
		
		void toStart();
		void toEnd();

		virtual void read(void* value, size_t size);
		virtual void write(void* value, size_t size);
		
		void fill(uint8_t value);

		template <typename T> T read() { T result; read(&result, sizeof(T)); return result; };
		template <typename T> void write(const T& value) { write((void*)&value, sizeof(T)); };

		void ensureSpace(size_t value);

	public:
		auto getMemory() const { return mMemory; }

		auto getSize() const { return mSize; }
		void setSize(size_t value);

		auto getPosition() const { return mPosition; }
		void setPosition(size_t value) { mPosition = value; };

		auto getRemaining() const { return mSize - mPosition; };
		bool hasRemaining() { return getRemaining() > 0; }

		auto getCapacity() const { return mCapacity; }
		void setCapacity(size_t value);

		auto getBlockSize() const { return mBlockSize; }
		void setBlockSize(size_t value) { mBlockSize = value; }

	private:
		void* mMemory = nullptr;
		size_t mSize = 0;
		size_t mPosition = 0;
		size_t mCapacity = 0;
		size_t mBlockSize = 1 << 13;
	};
}