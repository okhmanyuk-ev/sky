#include "system.h"

#include <platform/system_glfw.h>
#include <platform/system_ios.h>
#include <platform/system_emscripten.h>

using namespace Renderer;

System::System(std::optional<skygfx::BackendType> type, skygfx::Adapter adapter)
{
	auto width = PLATFORM->getWidth();
	auto height = PLATFORM->getHeight();
	auto window = PLATFORM->getWindow();
	skygfx::Initialize(window, width, height, type, adapter);
	skygfx::SetVsync(true);
}

System::~System()
{
	skygfx::Finalize();
}

void System::onEvent(const Platform::System::ResizeEvent& e)
{
	skygfx::Resize(e.width, e.height);
}

void System::setVertexBuffer(const Buffer& value)
{
	skygfx::SetVertexBuffer(value.data, value.size, value.stride);
}

void System::setIndexBuffer(const Buffer& value)
{
	skygfx::SetIndexBuffer(value.data, value.size, value.stride);
}

void System::setTexture(const skygfx::Texture& value)
{
	skygfx::SetTexture(0, value);
}

void System::setRenderTarget(std::shared_ptr<skygfx::RenderTarget> value)
{
	if (value == nullptr)
		skygfx::SetRenderTarget(std::nullopt);
	else
		skygfx::SetRenderTarget(*value);
}

void System::clear(std::optional<glm::vec4> color, std::optional<float> depth, std::optional<uint8_t> stencil)
{
	skygfx::Clear(color, depth, stencil);
}

void System::draw(uint32_t vertexCount, uint32_t vertexOffset)
{
	skygfx::Draw(vertexCount, vertexOffset);
}

void System::drawIndexed(uint32_t indexCount, uint32_t indexOffset)
{
	skygfx::DrawIndexed(indexCount, indexOffset);
}

void System::present()
{
	auto result = skygfx::Present();
	mDrawcalls = result.drawcalls;
}
