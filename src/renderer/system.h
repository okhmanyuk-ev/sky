#pragma once

#include <renderer/shader.h>
#include <optional>
#include <skygfx/skygfx.h>
#include <skygfx/vertex.h>
#include <skygfx/utils.h>
#include <common/event_system.h>
#include <platform/all.h>

#define RENDERER sky::Singleton<Renderer::System>::GetInstance()

namespace Renderer
{
	struct Buffer
	{
		Buffer() {}
		template<typename T> Buffer(T* memory, size_t count) : data((void*)memory), size(count * sizeof(T)), stride(sizeof(T)) {}
		template<typename T> Buffer(const std::vector<T>& values) : Buffer(values.data(), values.size()) {}

		void* data = nullptr;
		size_t size = 0;
		size_t stride = 0;
	};

	class System : public Common::Event::Listenable<Platform::System::ResizeEvent>
	{
	public:
		System(std::optional<skygfx::BackendType> type = std::nullopt,
			skygfx::Adapter adapter = skygfx::Adapter::HighPerformance);
		~System();

	private:
		void onEvent(const Platform::System::ResizeEvent& e) override;

	public:
		void setVertexBuffer(const Buffer& value);
		void setIndexBuffer(const Buffer& value);
		void setTexture(const skygfx::Texture& value);
		void setRenderTarget(std::shared_ptr<skygfx::RenderTarget> value);
		void setShader(std::shared_ptr<Shader> value);

		void clear(std::optional<glm::vec4> color = glm::vec4{ 0.0f, 0.0f, 0.0f, 0.0f }, 
			std::optional<float> depth = 1.0f, std::optional<uint8_t> stencil = 0);

		void draw(uint32_t vertexCount, uint32_t vertexOffset = 0);
		void drawIndexed(uint32_t indexCount, uint32_t indexOffset = 0);

		void present();

	public:
		int getDrawcalls() const { return mDrawcalls; }

	private:
		int mDrawcalls = 0;
	};
}
