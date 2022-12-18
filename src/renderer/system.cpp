#include "system.h"

#include <platform/system_windows.h>
#include <platform/system_ios.h>
#include <platform/system_mac.h>
#include <platform/system_emscripten.h>

using namespace Renderer;

System::System()
{
	auto width = PLATFORM->getWidth();
	auto height = PLATFORM->getHeight();

	void* window = nullptr;

#if defined(PLATFORM_WINDOWS)
	window = Platform::SystemWindows::Window;
#elif defined(PLATFORM_IOS)
	window = Platform::SystemIos::Window;
#elif defined(PLATFORM_MAC)
	window = Platform::SystemMac::Window;
#elif defined(PLATFORM_EMSCRIPTEN)
	window = Platform::SystemEmscripten::Window;
#endif

	assert(window != nullptr);

	skygfx::Initialize(window, width, height);
}

System::~System()
{
	skygfx::Finalize();
}

void System::onEvent(const Platform::System::ResizeEvent& e)
{
	skygfx::Resize(e.width, e.height);
}

void System::setTopology(skygfx::Topology value)
{
	skygfx::SetTopology(value);
}

void System::setViewport(std::optional<skygfx::Viewport> value)
{
	skygfx::SetViewport(value);
}

void System::setScissor(std::optional<skygfx::Scissor> value)
{
	skygfx::SetScissor(value);
}

void System::setVertexBuffer(const Buffer& value)
{
	skygfx::SetDynamicVertexBuffer(value.data, value.size, value.stride);
}

void System::setIndexBuffer(const Buffer& value)
{
	skygfx::SetDynamicIndexBuffer(value.data, value.size, value.stride);
}

void System::setUniformBuffer(uint32_t binding, void* memory, size_t size)
{
	skygfx::SetDynamicUniformBuffer(binding, memory, size);
}

void System::setTexture(uint32_t binding, const skygfx::Texture& value)
{
	skygfx::SetTexture(binding, value);
}

void System::setTexture(const skygfx::Texture& value)
{
	setTexture(0, value);
}

void System::setRenderTarget(std::shared_ptr<skygfx::RenderTarget> value)
{
	if (value == nullptr)
		skygfx::SetRenderTarget(std::nullopt);
	else
		skygfx::SetRenderTarget(*value);
}

void System::setShader(std::shared_ptr<Shader> value)
{
	skygfx::SetShader(*value);
	value->update();
}

void System::setSampler(skygfx::Sampler value)
{
	skygfx::SetSampler(value);
}

void System::setDepthMode(std::optional<skygfx::DepthMode> value)
{
	skygfx::SetDepthMode(value);
}

void System::setStencilMode(std::optional<skygfx::StencilMode> value)
{
	skygfx::SetStencilMode(value);
}

void System::setCullMode(skygfx::CullMode value)
{
	skygfx::SetCullMode(value);
}

void System::setBlendMode(const skygfx::BlendMode& value)
{
	skygfx::SetBlendMode(value);
}

void System::setTextureAddressMode(skygfx::TextureAddress value)
{
	skygfx::SetTextureAddress(value);
}

void System::clear(std::optional<glm::vec4> color, std::optional<float> depth, std::optional<uint8_t> stencil)
{
	skygfx::Clear(color, depth, stencil);
}

void System::draw(uint32_t vertexCount, uint32_t vertexOffset)
{
	mDrawcalls += 1;
	skygfx::Draw(vertexCount, vertexOffset);
}

void System::drawIndexed(uint32_t indexCount, uint32_t indexOffset, uint32_t vertexOffset)
{
	mDrawcalls += 1;
	skygfx::DrawIndexed(indexCount, indexOffset);
}

void System::readPixels(const glm::ivec2& pos, const glm::ivec2& size, std::shared_ptr<skygfx::Texture> dst_texture)
{
	skygfx::ReadPixels(pos, size, *dst_texture);
}

void System::present()
{
	mDrawcallsPublic = mDrawcalls;
	mDrawcalls = 0;
	skygfx::Present();
}
