#include "renderer.h"
#include <platform/system_glfw.h>
#include <platform/system_ios.h>
#include <platform/system_emscripten.h>

using namespace sky;

Renderer::Renderer(std::optional<skygfx::BackendType> type, skygfx::Adapter adapter)
{
	auto width = PLATFORM->getWidth();
	auto height = PLATFORM->getHeight();
	auto window = PLATFORM->getNativeWindowHandle();
	skygfx::Initialize(window, width, height, type, adapter);
	skygfx::SetVsync(true);
}

Renderer::~Renderer()
{
	skygfx::Finalize();
}

void Renderer::onEvent(const Platform::System::ResizeEvent& e)
{
	skygfx::Resize(e.width, e.height);
}

void Renderer::setRenderTarget(std::shared_ptr<skygfx::RenderTarget> value)
{
	if (value == nullptr)
		skygfx::SetRenderTarget(std::nullopt);
	else
		skygfx::SetRenderTarget(*value);
}

void Renderer::clear(std::optional<glm::vec4> color, std::optional<float> depth, std::optional<uint8_t> stencil)
{
	skygfx::Clear(color, depth, stencil);
}

void Renderer::present()
{
	auto result = skygfx::Present();
	mDrawcalls = result.drawcalls;
}
