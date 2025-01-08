#include "asset.h"

#include <platform/low_level_api.h>
#include <platform/system_android.h>
#include <fstream>
#include <cassert>
#include <filesystem>
#include <sys/stat.h>

using namespace Platform;

Asset::Asset(const std::string& path, Storage storage)
{
	assert(Exists(path, storage));
#if defined(PLATFORM_ANDROID)
	if (storage == Storage::Assets)
	{
		auto asset = AAssetManager_open(SystemAndroid::Instance->activity->assetManager, path.c_str(), AASSET_MODE_UNKNOWN);
		mSize = AAsset_getLength(asset);
		mMemory = std::malloc(mSize);
		AAsset_read(asset, mMemory, mSize);
		AAsset_close(asset);
	}
	else
	{
		auto p = StoragePathToAbsolute(path, storage);
		std::ifstream file(p, std::ios::in | std::ios::binary);
		file.seekg(0, file.end);
		mSize = static_cast<size_t>(file.tellg());
		mMemory = malloc(mSize);
		file.seekg(0, file.beg);
		file.read((char*)mMemory, mSize);
		file.close();
	}
#elif defined(PLATFORM_WINDOWS) | defined(PLATFORM_IOS) | defined(PLATFORM_MAC) | defined(PLATFORM_EMSCRIPTEN)
	auto p = StoragePathToAbsolute(path, storage);
	std::ifstream file(p, std::ios::in | std::ios::binary);
	file.seekg(0, file.end);
	mSize = static_cast<size_t>(file.tellg());
	mMemory = malloc(mSize);
	file.seekg(0, file.beg);
	file.read((char*)mMemory, mSize);
	file.close();
#endif
}

Asset::Asset(const Asset& asset)
{
	mSize = asset.getSize();
	mMemory = malloc(mSize);
	memcpy(mMemory, asset.getMemory(), mSize);
}

Asset::~Asset()
{
	free(mMemory);
}

void Asset::Write(const std::string& path, void* memory, size_t size, Storage storage)
{
#if defined(PLATFORM_WINDOWS) | defined(PLATFORM_IOS) | defined(PLATFORM_MAC) | defined(PLATFORM_EMSCRIPTEN)
#if defined(PLATFORM_IOS)
	assert(storage != Storage::Assets);
	if (storage == Storage::Assets)
		return;
#endif
	auto p = StoragePathToAbsolute(path, storage);

	auto dirs = std::filesystem::path(p).remove_filename().string();

	if (!dirs.empty())
		std::filesystem::create_directories(dirs);

	std::ofstream file(p, std::ios::out | std::ios::binary);
	file.write((char*)memory, size);
	file.close();
#elif defined(PLATFORM_ANDROID)
	assert(storage != Storage::Assets);
	if (storage != Storage::Assets)
	{
	//	std::experimental::filesystem::create_directories(std::experimental::filesystem::path(path).remove_filename().string());
		auto p = StoragePathToAbsolute(path, storage);
		std::ofstream file(p, std::ios::out | std::ios::binary);
		file.write((char*)memory, size);
		file.close();
	}
#endif
}

bool Asset::Exists(const std::string& path, Storage storage)
{
#if defined(PLATFORM_WINDOWS) | defined(PLATFORM_MAC) | defined(PLATFORM_EMSCRIPTEN)
	auto abs_path = StoragePathToAbsolute(path, storage);
	auto status = std::filesystem::status(abs_path);
	return status.type() == std::filesystem::file_type::regular;
#elif defined(PLATFORM_ANDROID)
	if (storage == Storage::Assets)
	{
		auto asset = AAssetManager_open(SystemAndroid::Instance->activity->assetManager, path.c_str(), AASSET_MODE_UNKNOWN);
		if (asset == nullptr)
			return false;
		AAsset_close(asset);
		return true;
	}
	else
	{
		auto p = StoragePathToAbsolute(path, storage);
		auto file = std::ifstream(p.c_str());
		return file.good(); // TODO: std::filesystem::exists() ?
	}
#elif defined(PLATFORM_IOS)
	auto p = StoragePathToAbsolute(path, storage);
	auto file = std::ifstream(p.c_str());
	return file.good(); // TODO: std::filesystem::exists() ?
#endif
}

std::string Asset::StoragePathToAbsolute(const std::string& path, Storage storage)
{
	if (storage == Storage::Assets)
	{
#if defined(PLATFORM_WINDOWS) | defined(PLATFORM_MAC) | defined(PLATFORM_EMSCRIPTEN)
		return AssetsFolder + "/" + path;
#elif defined(PLATFORM_IOS)
		return std::string([[[NSBundle mainBundle]bundlePath] UTF8String]) + "/assets/" + path;
#elif defined(PLATFORM_ANDROID)
		return path;
#endif
	}
	else if (storage == Storage::Bundle)
	{
#if defined(PLATFORM_WINDOWS)
		char* p;
		size_t len;
		errno_t err = _dupenv_s(&p, &len, "APPDATA");
		return p + ("\\" + PLATFORM->getAppName()) + "\\" + path;
#elif defined(PLATFORM_IOS)
		return std::string([NSHomeDirectory() UTF8String]) + "/Documents/" + path;
#elif defined(PLATFORM_ANDROID)
		return std::string(SystemAndroid::Instance->activity->internalDataPath) + "/" + path;
#endif
	}

	return path; // absolute
}
