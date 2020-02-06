#pragma once

#include <Platform/asset.h>

namespace Graphics
{
	class Image
	{
	public:
		Image(int width, int height, int channels);
		Image(void* data, size_t size);
		Image(const Platform::Asset& asset);

		Image(const Image& image);

		~Image();

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
