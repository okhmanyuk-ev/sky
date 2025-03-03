#pragma once

#include <string>
#include <platform/defines.h>

namespace sky
{
	class Asset
	{
	public:
		inline static std::string AssetsFolder = "assets";

	public:
		enum class Storage
		{
			Assets,
			Bundle
		};

	public:
		Asset(const std::string& path, Storage storage = Storage::Assets);
		Asset(const Asset& asset);
		~Asset();

	public:
		static void Write(const std::string& path, void* memory, size_t size, Storage storage = Storage::Assets);
		static bool Exists(const std::string& path, Storage storage = Storage::Assets);
		static std::string StoragePathToAbsolute(const std::string& path, Storage storage);
		static std::string FixSlashes(const std::string& path);

	public:
		auto getMemory() const { return mMemory; }
		auto getSize() const { return mSize; }

	private:
		void* mMemory = nullptr;
		size_t mSize = 0;
	};
}
