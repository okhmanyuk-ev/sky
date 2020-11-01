#include "image.h"
#include <stb_image_write.h>
#include <stb_image.h>

using namespace Graphics;

struct SaveImageContext
{
	std::string path;
	Platform::Asset::Path pathType;
};

void Image::SaveToFile(const std::string& path, const Image& image, Platform::Asset::Path pathType)
{
	auto writeFunc = [](void* context, void* memory, int size) {
		auto saveImageContext = static_cast<SaveImageContext*>(context);
		Platform::Asset::Write(saveImageContext->path + ".png", memory, size, saveImageContext->pathType);
	};

	SaveImageContext context;
	context.path = path;
	context.pathType = pathType;

	stbi_write_png_to_func(writeFunc, &context, image.getWidth(), image.getHeight(),
		image.getChannels(), image.getMemory(), image.getWidth() * image.getChannels());
}

Image::Image(int width, int height, int channels) : mWidth(width), mHeight(height), mChannels(channels)
{
	auto size = width * height * channels;
	mMemory = malloc(size);
	memset(mMemory, 0, size);
}

Image::Image(void* data, size_t size)
{
	mMemory = stbi_load_from_memory((uint8_t*)data, (int)size, &mWidth, &mHeight, nullptr, 4);
	mChannels = 4; // TODO: make adaptive channels
}

Image::Image(const Platform::Asset& asset) : Image(asset.getMemory(), asset.getSize())
{
	//
}

Image::Image(const Image& image)
{
	mWidth = image.getWidth();
	mHeight = image.getHeight();
	mChannels = image.getChannels();
	auto size = mWidth * mHeight * mChannels;
	mMemory = malloc(size);
	memcpy(mMemory, image.getMemory(), size);
}

Image::~Image() 
{
	free(mMemory);
}

uint8_t* Image::getPixel(int x, int y) const
{
	return &((uint8_t*)mMemory)[((y * mWidth) + x) * mChannels];
}

void Image::blur()
{
	for (int x = 0; x < mWidth; x++)
	{
		for (int y = 0; y < mHeight; y++)
		{
			auto pixel = getPixel(x, y); // TODO: implement
			pixel[0] = 255 - pixel[0];
			pixel[1] = 255 - pixel[1];
			pixel[2] = 255 - pixel[2];
		}
	}
}