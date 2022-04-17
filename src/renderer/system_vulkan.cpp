#include "system_vulkan.h"

#if defined(RENDERER_VK)
#include <platform/system_windows.h>
#include <console/device.h>

#include <vulkan/vulkan.hpp>

using namespace Renderer;

static VKAPI_ATTR VkBool32 VKAPI_CALL debug_report(VkDebugReportFlagsEXT flags, VkDebugReportObjectTypeEXT objectType, uint64_t object, size_t location, int32_t messageCode, const char* pLayerPrefix, const char* pMessage, void* pUserData)
{
	LOGF("[vulkan] {}", pMessage);
	return VK_FALSE;
}

static vk::Instance gInstance;
static vk::PhysicalDevice gPhysicalDevice;
static uint32_t gQueueFamily = -1;
static vk::Queue gQueue;
static vk::Device gDevice;
static vk::SurfaceKHR gSurface;
static vk::SurfaceFormatKHR gSurfaceFormat;
static vk::SwapchainKHR gSwapchain;
static int gMinImageCount = 2; // wtf constant doing here ?
static uint32_t gSemaphoreIndex = 0;
static uint32_t gFrameIndex = 0;
static vk::RenderPass gRenderPass;

struct Frame
{
	vk::CommandPool command_pool;
	vk::CommandBuffer command_buffer;
	vk::Fence fence;
	vk::Framebuffer framebuffer;
	vk::Image backbuffer;
	vk::ImageView backbuffer_view;
	vk::Semaphore image_acquired_semaphore;
	vk::Semaphore render_complete_semaphore;
};

static std::vector<Frame> gFrames;

SystemVK::SystemVK()
{
	auto layers = { 
		"VK_LAYER_KHRONOS_validation" 
	};

	auto extensions = {
		VK_KHR_SURFACE_EXTENSION_NAME,
		VK_KHR_WIN32_SURFACE_EXTENSION_NAME
	};

	gInstance = vk::createInstance(vk::InstanceCreateInfo()
		.setPEnabledExtensionNames(extensions)
		.setPEnabledLayerNames(layers));

	auto devices = gInstance.enumeratePhysicalDevices();
	int device_index = 0;
	for (size_t i = 0; i < devices.size(); i++)
	{
		auto properties = devices.at(i).getProperties();
		if (properties.deviceType == vk::PhysicalDeviceType::eDiscreteGpu)
		{
			device_index = i;
			break;
		}
	}

	gPhysicalDevice = devices.at(device_index);

	auto properties = gPhysicalDevice.getQueueFamilyProperties();

	for (size_t i = 0; i < properties.size(); i++)
	{
		if (properties[i].queueFlags & vk::QueueFlagBits::eGraphics)
		{
			gQueueFamily = i;
			break;
		}
	}

	auto device_extensions = { "VK_KHR_swapchain" };
	auto queue_priority = { 1.0f };

	auto queue_info = vk::DeviceQueueCreateInfo()
		.setQueueFamilyIndex(gQueueFamily)
		.setQueuePriorities(queue_priority);

	auto create_info = vk::DeviceCreateInfo()
		.setQueueCreateInfoCount(1)
		.setPQueueCreateInfos(&queue_info)
		.setPEnabledExtensionNames(device_extensions);

	gDevice = gPhysicalDevice.createDevice(create_info);
	gQueue = gDevice.getQueue(gQueueFamily, 0);

	auto surface_info = vk::Win32SurfaceCreateInfoKHR()
		.setHinstance(Platform::SystemWindows::Instance)
		.setHwnd(Platform::SystemWindows::Window);

	gSurface = gInstance.createWin32SurfaceKHR(surface_info);

	auto formats = gPhysicalDevice.getSurfaceFormatsKHR(gSurface);

	if ((formats.size() == 1) && (formats.at(0).format == vk::Format::eUndefined))
	{
		gSurfaceFormat = {
			vk::Format::eB8G8R8A8Unorm,
			formats.at(0).colorSpace
		};
	}
	else
	{
		bool found = false;
		for (const auto& format : formats)
		{
			if (format.format == vk::Format::eB8G8R8A8Unorm)
			{
				gSurfaceFormat = format;
				found = true;
				break;
			}
		}
		if (!found)
		{
			gSurfaceFormat = formats.at(0);
		}
	}

	auto swapchain_info = vk::SwapchainCreateInfoKHR()
		.setSurface(gSurface)
		.setMinImageCount(gMinImageCount)
		.setImageFormat(gSurfaceFormat.format)
		.setImageColorSpace(gSurfaceFormat.colorSpace)
		.setImageExtent({ 800, 600 })
		.setImageUsage(vk::ImageUsageFlagBits::eColorAttachment)
		.setPreTransform(vk::SurfaceTransformFlagBitsKHR::eIdentity)
		.setImageArrayLayers(1)
		.setImageSharingMode(vk::SharingMode::eExclusive)
		.setQueueFamilyIndexCount(0)
		.setPresentMode(vk::PresentModeKHR::eFifo)
		.setClipped(true)
		.setCompositeAlpha(vk::CompositeAlphaFlagBitsKHR::eOpaque);

	gSwapchain = gDevice.createSwapchainKHR(swapchain_info);

	auto attachment = vk::AttachmentDescription()
		.setFormat(gSurfaceFormat.format)
		.setSamples(vk::SampleCountFlagBits::e1)
		.setLoadOp(vk::AttachmentLoadOp::eClear)
		.setStoreOp(vk::AttachmentStoreOp::eStore)
		.setStencilLoadOp(vk::AttachmentLoadOp::eDontCare)
		.setStencilStoreOp(vk::AttachmentStoreOp::eDontCare)
		.setInitialLayout(vk::ImageLayout::eUndefined)
		.setFinalLayout(vk::ImageLayout::ePresentSrcKHR);

	auto color_attachment = vk::AttachmentReference()
		.setAttachment(0)
		.setLayout(vk::ImageLayout::eColorAttachmentOptimal);

	auto subpass = vk::SubpassDescription()
		.setPipelineBindPoint(vk::PipelineBindPoint::eGraphics)
		.setColorAttachmentCount(1)
		.setPColorAttachments(&color_attachment);

	auto dependency = vk::SubpassDependency()
		.setSrcSubpass(VK_SUBPASS_EXTERNAL)
		.setDstSubpass(0)
		.setSrcStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput)
		.setDstStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput)
		.setSrcAccessMask(vk::AccessFlagBits::eNone)
		.setDstAccessMask(vk::AccessFlagBits::eColorAttachmentWrite);

	auto render_pass_info = vk::RenderPassCreateInfo()
		.setAttachmentCount(1)
		.setPAttachments(&attachment)
		.setSubpassCount(1)
		.setPSubpasses(&subpass)
		.setDependencyCount(1)
		.setPDependencies(&dependency);

	gRenderPass = gDevice.createRenderPass(render_pass_info);

	auto backbuffers = gDevice.getSwapchainImagesKHR(gSwapchain);
	gFrames.resize(backbuffers.size());

	for (size_t i = 0; i < backbuffers.size(); i++)
	{
		gFrames[i].backbuffer = backbuffers[i];
	}

	for (auto& frame : gFrames)
	{
		auto command_pool_info = vk::CommandPoolCreateInfo()
			.setFlags(vk::CommandPoolCreateFlagBits::eResetCommandBuffer)
			.setQueueFamilyIndex(gQueueFamily);

		frame.command_pool = gDevice.createCommandPool(command_pool_info);

		auto buffer_allocate_info = vk::CommandBufferAllocateInfo()
			.setCommandBufferCount(1)
			.setLevel(vk::CommandBufferLevel::ePrimary)
			.setCommandPool(frame.command_pool);

		frame.command_buffer = gDevice.allocateCommandBuffers(buffer_allocate_info).at(0);

		auto fence_info = vk::FenceCreateInfo()
			.setFlags(vk::FenceCreateFlagBits::eSignaled);

		frame.fence = gDevice.createFence(fence_info);

		frame.image_acquired_semaphore = gDevice.createSemaphore({});
		frame.render_complete_semaphore = gDevice.createSemaphore({});

		auto image_view_info = vk::ImageViewCreateInfo()
			.setViewType(vk::ImageViewType::e2D)
			.setFormat(gSurfaceFormat.format)
			.setComponents(vk::ComponentMapping()
				.setR(vk::ComponentSwizzle::eR)
				.setG(vk::ComponentSwizzle::eG)
				.setB(vk::ComponentSwizzle::eB)
				.setA(vk::ComponentSwizzle::eA)
			)
			.setSubresourceRange(vk::ImageSubresourceRange()
				.setAspectMask(vk::ImageAspectFlagBits::eColor)
				.setBaseMipLevel(0)
				.setLevelCount(1)
				.setBaseArrayLayer(0)
				.setLayerCount(1)
			)
			.setImage(frame.backbuffer);

		frame.backbuffer_view = gDevice.createImageView(image_view_info);

		auto framebuffer_info = vk::FramebufferCreateInfo()
			.setRenderPass(gRenderPass)
			.setAttachmentCount(1)
			.setPAttachments(&frame.backbuffer_view)
			.setWidth(800) // TODO
			.setHeight(600)
			.setLayers(1);

		frame.framebuffer = gDevice.createFramebuffer(framebuffer_info);
	}
}

SystemVK::~SystemVK()
{
	//
}

void SystemVK::onEvent(const Platform::System::ResizeEvent& e)
{
	//
}

void SystemVK::setTopology(const Renderer::Topology& value)
{
	//
}

void SystemVK::setViewport(const Viewport& value) 
{
	//
}

void SystemVK::setScissor(const Scissor& value)
{
	//
}

void SystemVK::setScissor(std::nullptr_t value)
{
	//
}

void SystemVK::setVertexBuffer(const Buffer& value)
{
	//
}

void SystemVK::setIndexBuffer(const Buffer& value)
{
	//
}

void SystemVK::setTexture(std::shared_ptr<Texture> value)
{
	//
}

void SystemVK::setRenderTarget(std::shared_ptr<RenderTarget> value)
{
	//
}

void SystemVK::setShader(std::shared_ptr<Shader> value)
{
	//
}

void SystemVK::setSampler(const Sampler& value)
{
	//
}

void SystemVK::setDepthMode(const DepthMode& value)
{
	//
}

void SystemVK::setStencilMode(const StencilMode& value)
{
	//
}

void SystemVK::setCullMode(const CullMode& value)
{
	//
}

void SystemVK::setBlendMode(const BlendMode& value)
{
	//
}

void SystemVK::setTextureAddressMode(const TextureAddress& value)
{
	//
}

void SystemVK::clear(const glm::vec4& color)
{
	//
}

void SystemVK::clearStencil()
{
	//
}

void SystemVK::draw(size_t vertexCount, size_t vertexOffset)
{
	//
}

void SystemVK::drawIndexed(size_t indexCount, size_t indexOffset, size_t vertexOffset)
{
	//
}

void SystemVK::readPixels(const glm::ivec2& pos, const glm::ivec2& size, void* memory)
{
	//
}

void SystemVK::present()
{
	auto image_acquired_semaphore = gFrames.at(gSemaphoreIndex).image_acquired_semaphore;
	auto render_complete_semaphore = gFrames.at(gSemaphoreIndex).render_complete_semaphore;

	auto acquire_result = gDevice.acquireNextImageKHR(gSwapchain, UINT64_MAX, image_acquired_semaphore, {}, &gFrameIndex);

	const auto& frame = gFrames.at(gFrameIndex);

	gDevice.waitForFences(1, &frame.fence, true, UINT64_MAX);
	gDevice.resetFences(1, &frame.fence); // TODO: it seems fences can be allocated and destroyed right here, we dont need to store it (see sascha willems examples)

	auto begin_info = vk::CommandBufferBeginInfo()
		.setFlags(vk::CommandBufferUsageFlagBits::eOneTimeSubmit);

	frame.command_buffer.begin(begin_info);

	auto clear_value = vk::ClearValue();
	clear_value.color.float32[0] = 0.2f;
	clear_value.color.float32[1] = 0.5f;
	clear_value.color.float32[2] = 0.2f;
	clear_value.color.float32[3] = 1.0f;

	auto render_pass_begin_info = vk::RenderPassBeginInfo()
		.setRenderPass(gRenderPass)
		.setFramebuffer(frame.framebuffer)
		.setRenderArea(vk::Rect2D({ 0, 0 }, { 800, 600 }))
		.setClearValueCount(1)
		.setPClearValues(&clear_value);

	frame.command_buffer.beginRenderPass(render_pass_begin_info, vk::SubpassContents::eInline);

	frame.command_buffer.endRenderPass();

	vk::PipelineStageFlags wait_dst_stage_mask = vk::PipelineStageFlagBits::eColorAttachmentOutput;

	auto submit_info = vk::SubmitInfo()
		.setWaitSemaphoreCount(1)
		.setPWaitSemaphores(&image_acquired_semaphore)
		.setPWaitDstStageMask(&wait_dst_stage_mask)
		.setCommandBufferCount(1)
		.setPCommandBuffers(&frame.command_buffer)
		.setSignalSemaphoreCount(1)
		.setPSignalSemaphores(&render_complete_semaphore);

	gQueue.submit(1, &submit_info, frame.fence);

	auto present_info = vk::PresentInfoKHR()
		.setWaitSemaphoreCount(1)
		.setPWaitSemaphores(&render_complete_semaphore)
		.setSwapchainCount(1)
		.setPSwapchains(&gSwapchain)
		.setPImageIndices(&gFrameIndex);

	gQueue.presentKHR(present_info);

	gSemaphoreIndex = (gSemaphoreIndex + 1) % gFrames.size();
}

Texture::Handler SystemVK::createTexture(int width, int height, bool mipmap)
{
	return 0;
}

void SystemVK::destroyTexture(Texture::Handler value)
{
	//
}

void SystemVK::textureWritePixels(Texture::Handler texture, int width, int height, int channels, void* data)
{
	//
}

RenderTarget::RenderTargetHandler SystemVK::createRenderTarget(Texture::Handler texture)
{
	return 0;
}

void SystemVK::destroyRenderTarget(RenderTarget::RenderTargetHandler value)
{
	//
}

#endif