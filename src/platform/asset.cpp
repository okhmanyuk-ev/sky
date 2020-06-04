#include "asset.h"

#include <platform/low_level_api.h>
#include <platform/system_android.h>
#include <fstream>
#include <cassert>
#if !defined(PLATFORM_IOS)
#include <filesystem>
#endif
#include <sys/stat.h>

using namespace Platform;

Asset::Asset(const std::string& path, Path pathType)
{
	assert(Exists(path, pathType));
#if defined(PLATFORM_ANDROID)
	if (pathType == Path::Absolute)
	{
		std::ifstream file(path, std::ios::in | std::ios::binary);
		file.seekg(0, file.end);
		mSize = static_cast<size_t>(file.tellg());
		mMemory = malloc(mSize);
		file.seekg(0, file.beg);
		file.read((char*)mMemory, mSize);
		file.close();
	}
	else
	{
		auto asset = AAssetManager_open(SystemAndroid::Instance->activity->assetManager, path.c_str(), AASSET_MODE_UNKNOWN);
		mSize = AAsset_getLength(asset);
		mMemory = std::malloc(mSize);
		AAsset_read(asset, mMemory, mSize);
		AAsset_close(asset);
	}
#elif defined(PLATFORM_WINDOWS)
	auto p = path;

	if (pathType == Path::Relative)
		p = "assets/" + p;

	std::ifstream file(p, std::ios::in | std::ios::binary);
	file.seekg(0, file.end);
	mSize = static_cast<size_t>(file.tellg());
	mMemory = malloc(mSize);
	file.seekg(0, file.beg);
	file.read((char*)mMemory, mSize);
	file.close();
#elif defined(PLATFORM_IOS)
    auto p = path;
    
    if (pathType == Path::Relative)
    {
        auto ios_path = std::string([[[NSBundle mainBundle] bundlePath] UTF8String]);
        p = ios_path + "/assets/" + p;
    }

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

void Asset::Write(const std::string& path, void* memory, size_t size, Path pathType)
{
#if defined(PLATFORM_WINDOWS)
	auto p = path;

	if (pathType == Path::Relative)
		p = "assets/" + p;
	
	std::filesystem::create_directories(std::filesystem::path(p).remove_filename().string());

	std::ofstream file(p, std::ios::out | std::ios::binary);
	file.write((char*)memory, size);
	file.close();
#elif defined(PLATFORM_ANDROID)
	if (pathType == Path::Absolute)
	{
	//	std::experimental::filesystem::create_directories(std::experimental::filesystem::path(path).remove_filename().string());
		std::ofstream file(path, std::ios::out | std::ios::binary);
		file.write((char*)memory, size);
		file.close();
	}
	else
	{
		assert(false);
	}
#elif defined(PLATFORM_IOS)
    assert(pathType == Path::Absolute);
    
    auto p = path;

    std::ofstream file(p, std::ios::out | std::ios::binary);
    file.write((char*)memory, size);
    file.close();
#endif
}

bool Asset::Exists(const std::string& path, Path pathType)
{
#if defined(PLATFORM_WINDOWS)
	auto p = path;

	if (pathType == Path::Relative)
		p = "assets/" + p;
	
	return std::filesystem::exists(p);
#elif defined(PLATFORM_ANDROID)
	if (pathType == Path::Absolute)
	{
		auto file = std::ifstream(path.c_str());
		return file.good();
	//	return std::experimental::filesystem::exists(path);
	}
	else
	{
		auto asset = AAssetManager_open(SystemAndroid::Instance->activity->assetManager, path.c_str(), AASSET_MODE_UNKNOWN);
		if (asset == nullptr)
			return false;
		AAsset_close(asset);
		return true;
	}
#elif defined(PLATFORM_IOS)
    auto p = path;
    
    if (pathType == Path::Relative)
    {
        auto ios_path = std::string([[[NSBundle mainBundle] bundlePath] UTF8String]);
        p = ios_path + "/assets/" + p;
    }
    
    auto file = std::ifstream(p.c_str());
    return file.good();
#endif
}
