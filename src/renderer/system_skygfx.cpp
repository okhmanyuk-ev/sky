#include "system_skygfx.h"

#if defined(RENDERER_SKYGFX)
#include <platform/system_windows.h>

using namespace Renderer;

struct Shader::Impl
{
};

Shader::Shader(const Vertex::Layout& layout, const std::string& vertex_code, const std::string& fragment_code)
{
	mImpl = std::make_unique<Impl>();
}

Shader::~Shader()
{
}

void Shader::apply()
{
}

struct Texture::TextureImpl
{
};

Texture::Texture(int width, int height, bool mipmap) :
	mWidth(width),
	mHeight(height),
	mMipmap(mipmap)
{
	mTextureImpl = std::make_unique<TextureImpl>();
}

Texture::Texture(int width, int height, int channels, void* data, bool mipmap) : Texture(width, height, mipmap)
{
	writePixels(width, height, channels, data);
}

Texture::~Texture()
{
}

void Texture::writePixels(int width, int height, int channels, void* data)
{
}

struct RenderTarget::RenderTargetImpl
{
};

RenderTarget::RenderTarget(int width, int height) : Texture(width, height)
{
	mRenderTargetImpl = std::make_unique<RenderTargetImpl>();
}

RenderTarget::~RenderTarget()
{
}

SystemSkygfx::SystemSkygfx()
{
	auto width = PLATFORM->getWidth();
	auto height = PLATFORM->getHeight();
	mDevice = std::make_shared<skygfx::Device>(skygfx::BackendType::D3D11, Platform::SystemWindows::Window, width, height);
}

SystemSkygfx::~SystemSkygfx()
{
}

void SystemSkygfx::onEvent(const Platform::System::ResizeEvent& e)
{
}

void SystemSkygfx::setTopology(const Renderer::Topology& value)
{
}

void SystemSkygfx::setViewport(const Viewport& value)
{
	mDevice->setViewport(*(skygfx::Viewport*)&value);
}

void SystemSkygfx::setScissor(const Scissor& value)
{
}

void SystemSkygfx::setScissor(std::nullptr_t value)
{
}

void SystemSkygfx::setVertexBuffer(const Buffer& value)
{
}

void SystemSkygfx::setIndexBuffer(const Buffer& value)
{
}

void SystemSkygfx::setUniformBuffer(int slot, void* memory, size_t size)
{
}

void SystemSkygfx::setTexture(int binding, std::shared_ptr<Texture> value)
{
}

void SystemSkygfx::setTexture(std::shared_ptr<Texture> value)
{
}

void SystemSkygfx::setRenderTarget(std::shared_ptr<RenderTarget> value)
{
}

void SystemSkygfx::setShader(std::shared_ptr<Shader> value)
{
}

void SystemSkygfx::setSampler(const Sampler& value)
{
}

void SystemSkygfx::setDepthMode(const DepthMode& value)
{
}

void SystemSkygfx::setStencilMode(const StencilMode& value)
{
}

void SystemSkygfx::setCullMode(const CullMode& value)
{
}

void SystemSkygfx::setBlendMode(const BlendMode& value)
{
}

void SystemSkygfx::setTextureAddressMode(const TextureAddress& value)
{
}

void SystemSkygfx::clear(std::optional<glm::vec4> color, std::optional<float> depth, std::optional<uint8_t> stencil)
{
	mDevice->clear(color, depth, stencil);
}

void SystemSkygfx::draw(size_t vertexCount, size_t vertexOffset)
{
}

void SystemSkygfx::drawIndexed(size_t indexCount, size_t indexOffset, size_t vertexOffset)
{
	//mDevice->drawIndexed(indexCount, indexOffset);
}

void SystemSkygfx::readPixels(const glm::ivec2& pos, const glm::ivec2& size, void* memory)
{
}

void SystemSkygfx::readPixels(const glm::ivec2& pos, const glm::ivec2& size, std::shared_ptr<Renderer::Texture> dst_texture)
{
}

void SystemSkygfx::present()
{
}

#endif