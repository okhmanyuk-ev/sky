#include "system_skygfx.h"

#if defined(RENDERER_SKYGFX)
#include <platform/system_windows.h>

using namespace Renderer;

struct Shader::Impl
{
	std::shared_ptr<skygfx::Shader> shader;
};

Shader::Shader(const Vertex::Layout& layout, const std::string& vertex_code, const std::string& fragment_code)
{
	mImpl = std::make_unique<Impl>();
	mImpl->shader = std::make_shared<skygfx::Shader>(*(skygfx::Vertex::Layout*)&layout, vertex_code, fragment_code);
}

Shader::~Shader()
{
}

void Shader::apply()
{
	this->update();
}

struct Texture::TextureImpl
{
	std::shared_ptr<skygfx::Texture> texture;
};

Texture::Texture(int width, int height, int channels, void* data, bool mipmap) :
	mWidth(width),
	mHeight(height),
	mMipmap(mipmap)
{
	mTextureImpl = std::make_unique<TextureImpl>();
	mTextureImpl->texture = std::make_shared<skygfx::Texture>(width, height, channels, data, mMipmap);
}

Texture::~Texture()
{
}

struct RenderTarget::RenderTargetImpl
{
	std::shared_ptr<skygfx::RenderTarget> render_target;
};

RenderTarget::RenderTarget(int width, int height) : Texture(width, height, 4, nullptr)
{
	mRenderTargetImpl = std::make_unique<RenderTargetImpl>();
	mRenderTargetImpl->render_target = std::make_shared<skygfx::RenderTarget>(width, height);
	mTextureImpl->texture = mRenderTargetImpl->render_target;
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
	mDevice->setTopology(*(skygfx::Topology*)&value);
}

void SystemSkygfx::setViewport(const Viewport& value)
{
	mDevice->setViewport(*(skygfx::Viewport*)&value);
}

void SystemSkygfx::setScissor(const Scissor& value)
{
	mDevice->setScissor(*(skygfx::Scissor*)&value);
}

void SystemSkygfx::setScissor(std::nullptr_t value)
{
	mDevice->setScissor(value);
}

void SystemSkygfx::setVertexBuffer(const Buffer& value)
{
	mDevice->setVertexBuffer(*(skygfx::Buffer*)&value);
}

void SystemSkygfx::setIndexBuffer(const Buffer& value)
{
	mDevice->setIndexBuffer(*(skygfx::Buffer*)&value);
}

void SystemSkygfx::setUniformBuffer(int slot, void* memory, size_t size)
{
	mDevice->setUniformBuffer(slot, memory, size);
}

void SystemSkygfx::setTexture(int binding, std::shared_ptr<Texture> value)
{
	if (!value)
		return;

	if (!value->mTextureImpl->texture)
		return;

	mDevice->setTexture(*value->mTextureImpl->texture);
}

void SystemSkygfx::setTexture(std::shared_ptr<Texture> value)
{
	setTexture(0, value);
}

void SystemSkygfx::setRenderTarget(std::shared_ptr<RenderTarget> value)
{
	if (value == nullptr)
		mDevice->setRenderTarget(nullptr);
	else
		mDevice->setRenderTarget(*value->mRenderTargetImpl->render_target);
}

void SystemSkygfx::setShader(std::shared_ptr<Shader> value)
{
	mDevice->setShader(*value->mImpl->shader);
	value->apply();
}

void SystemSkygfx::setSampler(const Sampler& value)
{
	mDevice->setSampler(*(skygfx::Sampler*)&value);
}

void SystemSkygfx::setDepthMode(const DepthMode& value)
{
	mDevice->setDepthMode(*(skygfx::DepthMode*)&value);
}

void SystemSkygfx::setStencilMode(const StencilMode& value)
{
	mDevice->setStencilMode(*(skygfx::StencilMode*)&value);
}

void SystemSkygfx::setCullMode(const CullMode& value)
{
	mDevice->setCullMode(*(skygfx::CullMode*)&value);
}

void SystemSkygfx::setBlendMode(const BlendMode& value)
{
	mDevice->setBlendMode(*(skygfx::BlendMode*)&value);
}

void SystemSkygfx::setTextureAddressMode(const TextureAddress& value)
{
	mDevice->setTextureAddressMode(*(skygfx::TextureAddress*)&value);
}

void SystemSkygfx::clear(std::optional<glm::vec4> color, std::optional<float> depth, std::optional<uint8_t> stencil)
{
	mDevice->clear(color, depth, stencil);
}

void SystemSkygfx::draw(size_t vertexCount, size_t vertexOffset)
{
	mDevice->draw(vertexCount, vertexOffset);
}

void SystemSkygfx::drawIndexed(size_t indexCount, size_t indexOffset, size_t vertexOffset)
{
	mDevice->drawIndexed(indexCount, indexOffset);
}

void SystemSkygfx::readPixels(const glm::ivec2& pos, const glm::ivec2& size, void* memory)
{
}

void SystemSkygfx::readPixels(const glm::ivec2& pos, const glm::ivec2& size, std::shared_ptr<Renderer::Texture> dst_texture)
{
}

void SystemSkygfx::present()
{
	mDevice->present();
}

#endif