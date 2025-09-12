#pragma once

#include <sky/task.h>
#include <platform/defines.h>
#include <functional>
#include <optional>
#include <string>

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
		Asset(const void* memory, size_t size);
		Asset(const std::string& path, Storage storage = Storage::Assets);
		Asset(const Asset& asset);
		~Asset();

	public:
		static void Write(const std::string& path, void* memory, size_t size, Storage storage = Storage::Assets);
		static bool Exists(const std::string& path, Storage storage = Storage::Assets);
		static std::string StoragePathToAbsolute(const std::string& path, Storage storage);
		static std::string FixSlashes(const std::string& path);

		struct FetchCallbacks
		{
			std::function<void(const Asset& asset)> onSuccess = nullptr;
			std::function<void(const std::string& reason)> onFail = nullptr;
			std::function<void(size_t current, size_t total)> onProgress = nullptr;
		};

		static void Fetch(const std::string& url, FetchCallbacks callbacks);
		static Task<std::optional<Asset>> FetchAsync(const std::string& url);

	public:
		auto getMemory() const { return mMemory; }
		auto getSize() const { return mSize; }

	private:
		void* mMemory = nullptr;
		size_t mSize = 0;
	};
}
