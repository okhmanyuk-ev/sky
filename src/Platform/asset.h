#pragma once

#include <string>
#include "defines.h"

namespace Platform
{
	class Asset 
	{
	public:
		enum class Path // Storage
		{
			//Assets
			Relative,
			Absolute
		};

	public:
		Asset(const std::string& path, Path pathType = Path::Relative);
		Asset(const Asset& asset);
		~Asset();

	public:
		static void Write(const std::string& path, void* memory, size_t size, Path pathType = Path::Relative);
		static bool Exists(const std::string& path, Path pathType = Path::Relative);

	public:
		auto getMemory() const { return mMemory; }
		auto getSize() const { return mSize; }

	private:
		void* mMemory;
		size_t mSize;
	};
}