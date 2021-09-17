#pragma once

#include <renderer/low_level_api.h>

namespace Renderer
{
	class Texture
	{
		friend class SystemD3D11;
		friend class SystemGL;
	public:
		Texture(int width, int height, bool mipmap = false);
		Texture(int width, int height, int channels, void* data, bool mipmap = false);
		~Texture();

	public:
		void writePixels(int width, int height, int channels, void* data);
		
	public:
		auto getWidth() const { return mWidth; }
		auto getHeight() const { return mHeight; }
		
	private:
		int mWidth;
		int mHeight;
		bool mMipmap;

#if defined(RENDERER_D3D11)
	protected:
		ID3D11Texture2D* texture2d;
		ID3D11ShaderResourceView* shader_resource_view;
#elif defined(RENDERER_GL44) || defined(RENDERER_GLES3)
	protected:
		GLuint texture;
#endif
	};
}
