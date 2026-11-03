#include "renderer.h"
#include <sky/utils.h>
#include <magic_enum/magic_enum.hpp>
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

	if (!mPendingBackendType.has_value())
		return;

	auto type = mPendingBackendType.value();
	mPendingBackendType.reset();

	try
	{
		skygfx::SetBackendType(type);
		sky::Log("render api switched to {}", magic_enum::enum_name(type));
	}
	catch (const std::exception& e)
	{
		sky::Log(sky::Console::Color::Red, "cannot switch render api to {}: {}", magic_enum::enum_name(type), e.what());
	}
}

void Renderer::setBackendType(skygfx::BackendType type)
{
	mPendingBackendType = type;
}
