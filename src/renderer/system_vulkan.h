#pragma once

#include <renderer/system.h>

#if defined(RENDERER_VK)
#include <platform/system.h>
#include <common/event_system.h>
#include <common/hash.h>
#include <renderer/low_level_api.h>
#include <map>
#include <vulkan/vulkan_raii.hpp>

namespace Renderer
{
	class SystemVK;
	
	using SystemCrossplatform = SystemVK;

	class SystemVK : public System,
		Common::Event::Listenable<Platform::System::ResizeEvent>
	{
		friend Texture;
	public:
		SystemVK();
		~SystemVK();

	private:
		void createSwapchain();

	private:
		void onEvent(const Platform::System::ResizeEvent& e) override;

	public:
		void setTopology(const Topology& value) override;
		void setViewport(const Viewport& value) override;
		void setScissor(const Scissor& value) override;
		void setScissor(std::nullptr_t value) override;
		void setVertexBuffer(const Buffer& value) override;
		void setIndexBuffer(const Buffer& value) override;
		void setUniformBuffer(int slot, void* memory, size_t size) override;
		void setTexture(int binding, std::shared_ptr<Texture> value) override;
		void setTexture(std::shared_ptr<Texture> value) override;
		void setRenderTarget(std::shared_ptr<RenderTarget> value) override;
		void setShader(std::shared_ptr<Shader> value) override;
		void setSampler(const Sampler& value) override;
		void setDepthMode(const DepthMode& value) override;
		void setStencilMode(const StencilMode& value) override;
		void setCullMode(const CullMode& value) override;
		void setBlendMode(const BlendMode& value) override;
		void setTextureAddressMode(const TextureAddress& value) override;

		void clear(std::optional<glm::vec4> color = glm::vec4{ 0.0f, 0.0f, 0.0f, 0.0f },
			std::optional<float> depth = 1.0f, std::optional<uint8_t> stencil = 0) override;

		void draw(size_t vertexCount, size_t vertexOffset = 0) override;
		void drawIndexed(size_t indexCount, size_t indexOffset = 0, size_t vertexOffset = 0) override;

		void readPixels(const glm::ivec2& pos, const glm::ivec2& size, void* memory) override;
		void readPixels(const glm::ivec2& pos, const glm::ivec2& size, std::shared_ptr<Renderer::Texture> dst_texture) override;

		void present() override;

		bool isVsync() const override { return false; }
		void setVsync(bool value) override { }

	private:
		static inline vk::raii::Context mContext;
		static inline vk::raii::Instance mInstance = nullptr;
		static inline vk::raii::PhysicalDevice mPhysicalDevice = nullptr;
		static inline vk::raii::Queue mQueue = nullptr;
		static inline vk::raii::Device mDevice = nullptr;
		vk::raii::SurfaceKHR mSurface = nullptr;
		vk::raii::SwapchainKHR mSwapchain = nullptr;
		static inline vk::raii::CommandPool mCommandPool = nullptr;
		vk::raii::CommandBuffer mCommandBuffer = nullptr;
		vk::raii::Sampler mSampler = nullptr;
		vk::raii::DescriptorSetLayout mDescriptorSetLayout = nullptr;
		vk::raii::PipelineLayout mPipelineLayout = nullptr;
		vk::raii::Pipeline mPipeline = nullptr;

		struct DeviceBuffer
		{
			vk::raii::Buffer buffer = nullptr;
			vk::raii::DeviceMemory memory = nullptr;
			vk::DeviceSize size = 0;
		};

		std::vector<DeviceBuffer> mVertexBuffers;
		std::vector<DeviceBuffer> mIndexBuffers;

		int mVertexBufferIndex = 0;
		int mIndexBufferIndex = 0;

		vk::SurfaceFormatKHR mSurfaceFormat;
		uint32_t mMinImageCount = 2; // TODO: https://github.com/nvpro-samples/nvpro_core/blob/f2c05e161bba9ab9a8c96c0173bf0edf7c168dfa/nvvk/swapchain_vk.cpp#L143
		uint32_t mQueueFamilyIndex = -1;
		uint32_t mSemaphoreIndex = 0;
		uint32_t mFrameIndex = 0;
		uint32_t mWidth = 0;
		uint32_t mHeight = 0;
		bool mWorking = false;

		struct Frame
		{
			vk::raii::CommandBuffer command_buffer = nullptr;
			vk::raii::Fence fence = nullptr;
			vk::raii::ImageView backbuffer_view = nullptr;
			vk::raii::Semaphore image_acquired_semaphore = nullptr;
			vk::raii::Semaphore render_complete_semaphore = nullptr;
		};

		std::vector<Frame> mFrames;

	private:
		void begin();
		void end();

	private: // utils
		static void setImageLayout(vk::raii::CommandBuffer const& commandBuffer, vk::Image image,
			vk::Format format, vk::ImageLayout oldImageLayout, vk::ImageLayout newImageLayout);

		template <typename Func>
		static void oneTimeSubmit(vk::raii::CommandBuffer const& commandBuffer, vk::raii::Queue const& queue, Func const& func)
		{
			commandBuffer.begin(vk::CommandBufferBeginInfo(vk::CommandBufferUsageFlagBits::eOneTimeSubmit));
			func(commandBuffer);
			commandBuffer.end();
			vk::SubmitInfo submitInfo(nullptr, nullptr, *commandBuffer);
			queue.submit(submitInfo, nullptr);
			queue.waitIdle();
		}

		template <typename Func>
		static void oneTimeSubmit(vk::raii::Device const& device, vk::raii::CommandPool const& commandPool, vk::raii::Queue const& queue, Func const& func)
		{
			vk::raii::CommandBuffers commandBuffers(device, { *commandPool, vk::CommandBufferLevel::ePrimary, 1 });
			oneTimeSubmit(commandBuffers.front(), queue, func);
		}

	private:
		void drawTest(); // TODO: del
	};
}
#endif