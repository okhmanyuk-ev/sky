#pragma once

#include <memory>

namespace Renderer
{
	class Texture
	{
		friend class SystemD3D11;
		friend class SystemGL;
		friend class SystemVK;

	public:
		Texture(int width, int height, bool mipmap = false);
		Texture(int width, int height, int channels, void* data, bool mipmap = false);
		~Texture();

	public:
		void writePixels(int width, int height, int channels, void* data);

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
