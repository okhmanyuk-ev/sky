#include "system.h"

#include <platform/system_windows.h>
#include <platform/system_ios.h>

using namespace Renderer;

System::System()
{
	auto width = PLATFORM->getWidth();
	auto height = PLATFORM->getHeight();

	auto type = skygfx::BackendType::D3D11;

#if defined(PLATFORM_IOS)
	//type = skygfx::BackendType::Metal;
	type = skygfx::BackendType::OpenGL44;
#endif

	void* window = nullptr;

#if defined(PLATFORM_WINDOWS)
	window = Platform::SystemWindows::Window;
#elif defined(PLATFORM_IOS)
	window = Platform::SystemIos::Window; // TODO: maybe we should send rootView here
#endif

	mDevice = std::make_shared<skygfx::Device>(type, window, width, height);
}

System::~System()
{
}

void System::onEvent(const Platform::System::ResizeEvent& e)
{
	mDevice->resize(e.width, e.height);
}

void System::setTopology(skygfx::Topology value)
{
	mDevice->setTopology(value);
}

void System::setViewport(std::optional<skygfx::Viewport> value)
{
	mDevice->setViewport(value);
}

void System::setScissor(std::optional<skygfx::Scissor> value)
{
	mDevice->setScissor(value);
}

void System::setVertexBuffer(const Buffer& value)
{
	assert(value.size > 0);

	size_t size = 0;

	if (mVertexBuffer)
		size = mVertexBuffer->getSize();

	if (size < value.size)
		mVertexBuffer = std::make_shared<skygfx::VertexBuffer>(value.data, value.size, value.stride);
	else
		mVertexBuffer->write(value.data, value.size, value.stride);

	mDevice->setVertexBuffer(*mVertexBuffer);
}

void System::setIndexBuffer(const Buffer& value)
{
	assert(value.size > 0);

	size_t size = 0;

	if (mIndexBuffer)
		size = mIndexBuffer->getSize();

	if (size < value.size)
		mIndexBuffer = std::make_shared<skygfx::IndexBuffer>(value.data, value.size, value.stride);
	else
		mIndexBuffer->write(value.data, value.size, value.stride);

	mDevice->setIndexBuffer(*mIndexBuffer);
}

void System::setUniformBuffer(uint32_t binding, void* memory, size_t size)
{
	assert(size > 0);

	std::shared_ptr<skygfx::UniformBuffer> uniform_buffer = nullptr;

	if (mUniformBuffers.contains(binding))
		uniform_buffer = mUniformBuffers.at(binding);

	size_t uniform_buffer_size = 0;

	if (uniform_buffer)
		uniform_buffer_size = uniform_buffer->getSize();

	if (uniform_buffer_size < size)
	{
		uniform_buffer = std::make_shared<skygfx::UniformBuffer>(memory, size);
		mUniformBuffers[binding] = uniform_buffer;
	}
	else
	{
		uniform_buffer->write(memory, size);
	}

	mDevice->setUniformBuffer(binding, *uniform_buffer);
}

void System::setTexture(uint32_t binding, const skygfx::Texture& value)
{
	mDevice->setTexture(binding, value);
}

void System::setTexture(const skygfx::Texture& value)
{
	setTexture(0, value);
}

void System::setRenderTarget(std::shared_ptr<skygfx::RenderTarget> value)
{
	if (value == nullptr)
		mDevice->setRenderTarget(nullptr);
	else
		mDevice->setRenderTarget(*value);
}

void System::setShader(std::shared_ptr<Shader> value)
{
	mDevice->setShader(*value->mShader);
	value->update();
}

void System::setSampler(skygfx::Sampler value)
{
	mDevice->setSampler(value);
}

void System::setDepthMode(std::optional<skygfx::DepthMode> value)
{
	mDevice->setDepthMode(value);
}

void System::setStencilMode(std::optional<skygfx::StencilMode> value)
{
	mDevice->setStencilMode(value);
}

void System::setCullMode(skygfx::CullMode value)
{
	mDevice->setCullMode(value);
}

void System::setBlendMode(const skygfx::BlendMode& value)
{
	mDevice->setBlendMode(value);
}

void System::setTextureAddressMode(skygfx::TextureAddress value)
{
	mDevice->setTextureAddress(value);
}

void System::clear(std::optional<glm::vec4> color, std::optional<float> depth, std::optional<uint8_t> stencil)
{
	mDevice->clear(color, depth, stencil);
}

void System::draw(size_t vertexCount, size_t vertexOffset)
{
	mDrawcalls += 1;
	mDevice->draw(vertexCount, vertexOffset);
}

void System::drawIndexed(size_t indexCount, size_t indexOffset, size_t vertexOffset)
{
	mDrawcalls += 1;
	mDevice->drawIndexed(indexCount, indexOffset);
}

void System::readPixels(const glm::ivec2& pos, const glm::ivec2& size, std::shared_ptr<skygfx::Texture> dst_texture)
{
	mDevice->readPixels(pos, size, *dst_texture);
}

void System::present()
{
	mDrawcallsPublic = mDrawcalls;
	mDrawcalls = 0;
	mDevice->present();
}