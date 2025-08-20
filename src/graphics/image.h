#pragma once

#include <sky/asset.h>
#include <cstdint>

namespace Graphics
{
	class Image
	{
	public:
		static void SaveToFile(const std::string& path, const Image& image,
			sky::Asset::Storage storage = sky::Asset::Storage::Assets);

	public:
		Image(int width, int height, int channels);
		Image(void* data, size_t size);
		Image(const sky::Asset& asset);
		Image(const Image& image);
		~Image();

	public:
		uint8_t* getPixel(int x, int y) const;

	public:
		auto getMemory() const { return mMemory; }
		auto getWidth() const { return mWidth; }
		auto getHeight() const { return mHeight; }
		auto getChannels() const { return mChannels; }

	private:
		void* mMemory;
		int mWidth;
		int mHeight;
		int mChannels;
	};
}
