#pragma once

#include <optional>
#include <skygfx/skygfx.h>
#include <skygfx/vertex.h>
#include <skygfx/utils.h>
#include <sky/dispatcher.h>
#include <platform/all.h>

#define RENDERER sky::Locator<sky::Renderer>::GetService()

namespace sky
{
	class Renderer : public sky::Listenable<Platform::System::ResizeEvent>
	{
	public:
		Renderer(std::optional<skygfx::BackendType> type = std::nullopt,
			skygfx::Adapter adapter = skygfx::Adapter::HighPerformance);
		~Renderer();

	private:
		void onEvent(const Platform::System::ResizeEvent& e) override;

	public:
		void setRenderTarget(std::shared_ptr<skygfx::RenderTarget> value);

		void clear(std::optional<glm::vec4> color = glm::vec4{ 0.0f, 0.0f, 0.0f, 0.0f },
			std::optional<float> depth = 1.0f, std::optional<uint8_t> stencil = 0);

		void present();

	public:
		int getDrawcalls() const { return mDrawcalls; }

	private:
		int mDrawcalls = 0;
	};
}
