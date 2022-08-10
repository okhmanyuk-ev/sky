#pragma once

#include <memory>

namespace Renderer
{
	class Texture
	{
		friend class SystemSkygfx;

	public:
		Texture(uint32_t width, uint32_t height, uint32_t channels, void* data, bool mipmap = false);
		~Texture();

	public:
		auto getWidth() const { return mWidth; }
		auto getHeight() const { return mHeight; }
		auto isMipmap() const { return mMipmap; }

	private:
		int mWidth;
		int mHeight;
		bool mMipmap;

	protected:
		struct TextureImpl;
		std::unique_ptr<TextureImpl> mTextureImpl;
	};
}
