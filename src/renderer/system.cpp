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

void System::present()
{
	auto result = skygfx::Present();
	mDrawcalls = result.drawcalls;
}
