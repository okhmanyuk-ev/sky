#include "asset.h"

#include <platform/low_level_api.h>
#include <platform/system_android.h>
#include <fstream>
#include <cassert>
#include <filesystem>
#include <sys/stat.h>
#ifdef EMSCRIPTEN
#include <emscripten/fetch.h>
#endif
#include <sky/utils.h>

sky::Asset::Asset(const void* memory, size_t size) : mSize(size)
{
	mMemory = malloc(mSize);
	memcpy(mMemory, memory, mSize);
}

sky::Asset::Asset(const std::string& path, Storage storage)
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
#elif defined(PLATFORM_WINDOWS) | defined(PLATFORM_IOS) | defined(PLATFORM_MAC) | defined(PLATFORM_EMSCRIPTEN) | defined(PLATFORM_LINUX)
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

sky::Asset::Asset(const Asset& asset)
{
	mSize = asset.getSize();
	mMemory = malloc(mSize);
	memcpy(mMemory, asset.getMemory(), mSize);
}

sky::Asset::~Asset()
{
	free(mMemory);
}

void sky::Asset::Write(const std::string& path, void* memory, size_t size, Storage storage)
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

bool sky::Asset::Exists(const std::string& path, Storage storage)
{
#if defined(PLATFORM_WINDOWS) | defined(PLATFORM_MAC) | defined(PLATFORM_EMSCRIPTEN) | defined(PLATFORM_LINUX)
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

std::string sky::Asset::StoragePathToAbsolute(const std::string& _path, Storage storage)
{
	auto path = FixSlashes(_path);
	if (storage == Storage::Assets)
	{
#if defined(PLATFORM_WINDOWS) | defined(PLATFORM_MAC) | defined(PLATFORM_EMSCRIPTEN)
		return AssetsFolder + "/" + path;
#elif defined(PLATFORM_IOS)
		return std::string([[[NSBundle mainBundle]bundlePath] UTF8String]) + "/" + AssetsFolder + "/" + path;
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

std::string sky::Asset::FixSlashes(const std::string& input)
{
	std::string result = input;
	for (char& ch : result)
	{
		if (ch == '\\')
		{
			ch = '/';
		}
	}
	return result;
}

void sky::Asset::Fetch(const std::string& url, FetchCallbacks callbacks)
{
#ifdef EMSCRIPTEN
	sky::Log("fetching {}", url);
	emscripten_fetch_attr_t attr;
	emscripten_fetch_attr_init(&attr);
	strcpy(attr.requestMethod, "GET");
	attr.attributes = EMSCRIPTEN_FETCH_LOAD_TO_MEMORY;
	attr.userData = new FetchCallbacks(std::move(callbacks));
	attr.onsuccess = [](emscripten_fetch_t* fetch) {
		sky::Log("fetch succeeded");
		auto callbacks = static_cast<FetchCallbacks*>(fetch->userData);
		if (callbacks->onSuccess)
		{
			callbacks->onSuccess(Asset((const void*)fetch->data, (size_t)fetch->numBytes));
		}
		delete callbacks;
		emscripten_fetch_close(fetch);
	};
	attr.onerror = [](emscripten_fetch_t* fetch) {
		sky::Log("fetch failed");
		auto callbacks = static_cast<FetchCallbacks*>(fetch->userData);
		if (callbacks->onFail)
		{
			callbacks->onFail(std::string(fetch->statusText));
		}
		delete callbacks;
		emscripten_fetch_close(fetch);
	};
	attr.onprogress = [](emscripten_fetch_t* fetch) {
		sky::Log("fetch progress {} of {}", fetch->dataOffset, fetch->totalBytes);
		auto callbacks = static_cast<FetchCallbacks*>(fetch->userData);
		if (callbacks->onProgress)
		{
			callbacks->onProgress((size_t)fetch->dataOffset, (size_t)fetch->totalBytes);
		}
	};
	emscripten_fetch(&attr, url.c_str());
#else
	if (callbacks.onFail)
		callbacks.onFail("fetch is unsupported on this platform");
#endif
}
