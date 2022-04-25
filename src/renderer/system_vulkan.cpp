#include "system_vulkan.h"

#if defined(RENDERER_VK)
#include <platform/system_windows.h>
#include <console/device.h>
#include <iostream>

using namespace Renderer;

static VKAPI_ATTR VkBool32 VKAPI_CALL debug_report(VkDebugReportFlagsEXT flags, VkDebugReportObjectTypeEXT objectType, uint64_t object, size_t location, int32_t messageCode, const char* pLayerPrefix, const char* pMessage, void* pUserData)
{
	LOGF("[vulkan] {}", pMessage);
	return VK_FALSE;
}

static vk::raii::Context gContext;
static vk::raii::Instance gInstance = nullptr;
static vk::raii::PhysicalDevice gPhysicalDevice = nullptr;
static vk::raii::Queue gQueue = nullptr;
static vk::raii::Device gDevice = nullptr;
static vk::raii::SurfaceKHR gSurface = nullptr;
static vk::raii::SwapchainKHR gSwapchain = nullptr;
static vk::raii::CommandPool gCommandPool = nullptr;
static vk::SurfaceFormatKHR gSurfaceFormat;
static uint32_t gMinImageCount = 2; // TODO: https://github.com/nvpro-samples/nvpro_core/blob/f2c05e161bba9ab9a8c96c0173bf0edf7c168dfa/nvvk/swapchain_vk.cpp#L143
static uint32_t gQueueFamilyIndex = -1;
static uint32_t gSemaphoreIndex = 0;
static uint32_t gFrameIndex = 0;
static uint32_t gWidth = 0;
static uint32_t gHeight = 0;

struct Frame
{
	vk::raii::CommandBuffer command_buffer = nullptr;
	vk::raii::Fence fence = nullptr;
	vk::raii::ImageView backbuffer_view = nullptr;
	vk::raii::Semaphore image_acquired_semaphore = nullptr;
	vk::raii::Semaphore render_complete_semaphore = nullptr;
};

static std::vector<Frame> gFrames;

SystemVK::SystemVK()
{
	auto all_extensions = gContext.enumerateInstanceExtensionProperties();

	for (auto extension : all_extensions)
	{
	//	std::cout << extension.extensionName << std::endl;
	}

	auto all_layers = gContext.enumerateInstanceLayerProperties();

	for (auto layer : all_layers)
	{
	//	std::cout << e.layerName << std::endl;
	}

	auto extensions = {
		VK_KHR_SURFACE_EXTENSION_NAME,
		VK_KHR_WIN32_SURFACE_EXTENSION_NAME
	};

	auto layers = {
		"VK_LAYER_KHRONOS_validation"
	};

	auto version = gContext.enumerateInstanceVersion();

	auto major_version = VK_API_VERSION_MAJOR(version);
	auto minor_version = VK_API_VERSION_MINOR(version);

	std::cout << "available vulkan version: " << major_version << "." << minor_version << std::endl;

	auto application_info = vk::ApplicationInfo()
		.setApiVersion(VK_API_VERSION_1_3);

	auto instance_info = vk::InstanceCreateInfo()
		.setPEnabledExtensionNames(extensions)
		.setPEnabledLayerNames(layers)
		.setPApplicationInfo(&application_info);

	gInstance = gContext.createInstance(instance_info);

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

	gPhysicalDevice = std::move(devices.at(device_index));

	auto properties = gPhysicalDevice.getQueueFamilyProperties();

	for (size_t i = 0; i < properties.size(); i++)
	{
		if (properties[i].queueFlags & vk::QueueFlagBits::eGraphics)
		{
			gQueueFamilyIndex = i;
			break;
		}
	}

	auto all_device_extensions = gPhysicalDevice.enumerateDeviceExtensionProperties();

	for (auto device_extension : all_device_extensions)
	{
	//	std::cout << device_extension.extensionName << std::endl;
	}

	auto device_extensions = {
		VK_KHR_SWAPCHAIN_EXTENSION_NAME,
	//	VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME
	};

	auto queue_priority = { 1.0f };

	auto queue_info = vk::DeviceQueueCreateInfo()
		.setQueueFamilyIndex(gQueueFamilyIndex)
		.setQueuePriorities(queue_priority);

	auto device_features = gPhysicalDevice.getFeatures2<vk::PhysicalDeviceFeatures2, vk::PhysicalDeviceVulkan13Features>();

	auto device_info = vk::DeviceCreateInfo()
		.setQueueCreateInfoCount(1)
		.setPQueueCreateInfos(&queue_info)
		.setPEnabledExtensionNames(device_extensions)
		.setPEnabledFeatures(nullptr)
		.setPNext(&device_features);

	gDevice = gPhysicalDevice.createDevice(device_info);
	gQueue = gDevice.getQueue(gQueueFamilyIndex, 0);

	auto surface_info = vk::Win32SurfaceCreateInfoKHR()
		.setHinstance(Platform::SystemWindows::Instance)
		.setHwnd(Platform::SystemWindows::Window);
	
	gSurface = vk::raii::SurfaceKHR(gInstance, surface_info);

	auto formats = gPhysicalDevice.getSurfaceFormatsKHR(*gSurface);

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

	auto command_pool_info = vk::CommandPoolCreateInfo()
		.setFlags(vk::CommandPoolCreateFlagBits::eResetCommandBuffer)
		.setQueueFamilyIndex(gQueueFamilyIndex);

	gCommandPool = gDevice.createCommandPool(command_pool_info);

	createSwapchain();
}

SystemVK::~SystemVK()
{
	//
}

void SystemVK::createSwapchain()
{
	gWidth = static_cast<uint32_t>(PLATFORM->getWidth());
	gHeight = static_cast<uint32_t>(PLATFORM->getHeight());

	auto swapchain_info = vk::SwapchainCreateInfoKHR()
		.setSurface(*gSurface)
		.setMinImageCount(gMinImageCount)
		.setImageFormat(gSurfaceFormat.format)
		.setImageColorSpace(gSurfaceFormat.colorSpace)
		.setImageExtent({ gWidth, gHeight })
		.setImageUsage(vk::ImageUsageFlagBits::eColorAttachment)
		.setPreTransform(vk::SurfaceTransformFlagBitsKHR::eIdentity)
		.setImageArrayLayers(1)
		.setImageSharingMode(vk::SharingMode::eExclusive)
		.setQueueFamilyIndexCount(1)
		.setPQueueFamilyIndices(&gQueueFamilyIndex)
		.setPresentMode(vk::PresentModeKHR::eFifo)
		.setClipped(true)
		.setCompositeAlpha(vk::CompositeAlphaFlagBitsKHR::eOpaque)
		.setOldSwapchain(*gSwapchain);

	gSwapchain = gDevice.createSwapchainKHR(swapchain_info);

	auto backbuffers = gSwapchain.getImages();

	gFrames.clear();

	for (auto& backbuffer : backbuffers)
	{
		auto frame = Frame();

		auto buffer_allocate_info = vk::CommandBufferAllocateInfo()
			.setCommandBufferCount(1)
			.setLevel(vk::CommandBufferLevel::ePrimary)
			.setCommandPool(*gCommandPool);

		auto command_buffers = gDevice.allocateCommandBuffers(buffer_allocate_info);
		frame.command_buffer = std::move(command_buffers.at(0));

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
			.setImage(backbuffer);

		frame.backbuffer_view = gDevice.createImageView(image_view_info);

		oneTimeSubmit(gDevice, gCommandPool, gQueue, [&](auto& cmd) {
			setImageLayout(cmd, backbuffer, gSurfaceFormat.format, vk::ImageLayout::eUndefined, 
				vk::ImageLayout::ePresentSrcKHR);
		});

		gFrames.push_back(std::move(frame));
	}
}

void SystemVK::onEvent(const Platform::System::ResizeEvent& e)
{
	createSwapchain();
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

void SystemVK::clear(std::optional<glm::vec4> color, std::optional<float> depth, std::optional<uint8_t> stencil)
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

void SystemVK::readPixels(const glm::ivec2& pos, const glm::ivec2& size, std::shared_ptr<Renderer::Texture> dst_texture)
{
	//
}

void SystemVK::present()
{
	const auto& image_acquired_semaphore = gFrames.at(gSemaphoreIndex).image_acquired_semaphore;
	const auto& render_complete_semaphore = gFrames.at(gSemaphoreIndex).render_complete_semaphore;

	auto [result, image_index] = gSwapchain.acquireNextImage(UINT64_MAX, *image_acquired_semaphore);

	gFrameIndex = image_index;

	const auto& frame = gFrames.at(gFrameIndex);

	gDevice.waitForFences({ *frame.fence }, true, UINT64_MAX);
	gDevice.resetFences({ *frame.fence });

	auto begin_info = vk::CommandBufferBeginInfo()
		.setFlags(vk::CommandBufferUsageFlagBits::eOneTimeSubmit);

	frame.command_buffer.begin(begin_info);

	auto clear_value = vk::ClearValue();
	clear_value.color.float32[0] = 0.2f;
	clear_value.color.float32[1] = 0.5f;
	clear_value.color.float32[2] = 0.2f;
	clear_value.color.float32[3] = 1.0f;
	
	auto color_attachment = vk::RenderingAttachmentInfo()
		.setImageView(*frame.backbuffer_view)
		.setImageLayout(vk::ImageLayout::eAttachmentOptimal)
		.setLoadOp(vk::AttachmentLoadOp::eClear)
		.setStoreOp(vk::AttachmentStoreOp::eStore)
		.setClearValue(clear_value);

	auto rendering_info = vk::RenderingInfo()
		.setRenderArea({ { 0, 0 }, { gWidth, gHeight } })
		.setLayerCount(1)
		.setColorAttachmentCount(1)
		.setPColorAttachments(&color_attachment);

	frame.command_buffer.beginRendering(rendering_info);
	frame.command_buffer.endRendering();

	frame.command_buffer.end();

	vk::PipelineStageFlags wait_dst_stage_mask = vk::PipelineStageFlagBits::eColorAttachmentOutput;

	auto submit_info = vk::SubmitInfo()
		.setPWaitDstStageMask(&wait_dst_stage_mask)
		.setWaitSemaphoreCount(1)
		.setPWaitSemaphores(&*image_acquired_semaphore)
		.setCommandBufferCount(1)
		.setPCommandBuffers(&*frame.command_buffer)
		.setSignalSemaphoreCount(1)
		.setPSignalSemaphores(&*render_complete_semaphore);

	gQueue.submit({ submit_info }, *frame.fence); // TODO: can be called with no fence, check it out

	auto present_info = vk::PresentInfoKHR()
		.setWaitSemaphoreCount(1)
		.setPWaitSemaphores(&*render_complete_semaphore)
		.setSwapchainCount(1)
		.setPSwapchains(&*gSwapchain)
		.setPImageIndices(&gFrameIndex);

	gQueue.presentKHR(present_info);
	gQueue.waitIdle();

	gSemaphoreIndex = (gSemaphoreIndex + 1) % gFrames.size(); // TODO: maybe gFrameIndex can be used for both
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

void SystemVK::setImageLayout(vk::raii::CommandBuffer const& commandBuffer, vk::Image image, 
	vk::Format format, vk::ImageLayout oldImageLayout, vk::ImageLayout newImageLayout)
{
	vk::AccessFlags sourceAccessMask;
	switch (oldImageLayout)
	{
	case vk::ImageLayout::eTransferDstOptimal: sourceAccessMask = vk::AccessFlagBits::eTransferWrite; break;
	case vk::ImageLayout::ePreinitialized: sourceAccessMask = vk::AccessFlagBits::eHostWrite; break;
	case vk::ImageLayout::eGeneral:  // sourceAccessMask is empty
	case vk::ImageLayout::eUndefined: break;
	default: assert(false); break;
	}

	vk::PipelineStageFlags sourceStage;
	switch (oldImageLayout)
	{
	case vk::ImageLayout::eGeneral:
	case vk::ImageLayout::ePreinitialized: sourceStage = vk::PipelineStageFlagBits::eHost; break;
	case vk::ImageLayout::eTransferDstOptimal: sourceStage = vk::PipelineStageFlagBits::eTransfer; break;
	case vk::ImageLayout::eUndefined: sourceStage = vk::PipelineStageFlagBits::eTopOfPipe; break;
	default: assert(false); break;
	}

	vk::AccessFlags destinationAccessMask;
	switch (newImageLayout)
	{
	case vk::ImageLayout::eColorAttachmentOptimal: destinationAccessMask = vk::AccessFlagBits::eColorAttachmentWrite; break;
	case vk::ImageLayout::eDepthStencilAttachmentOptimal:
		destinationAccessMask = vk::AccessFlagBits::eDepthStencilAttachmentRead | vk::AccessFlagBits::eDepthStencilAttachmentWrite;
		break;
	case vk::ImageLayout::eGeneral:  // empty destinationAccessMask
	case vk::ImageLayout::ePresentSrcKHR: break;
	case vk::ImageLayout::eShaderReadOnlyOptimal: destinationAccessMask = vk::AccessFlagBits::eShaderRead; break;
	case vk::ImageLayout::eTransferSrcOptimal: destinationAccessMask = vk::AccessFlagBits::eTransferRead; break;
	case vk::ImageLayout::eTransferDstOptimal: destinationAccessMask = vk::AccessFlagBits::eTransferWrite; break;
	default: assert(false); break;
	}

	vk::PipelineStageFlags destinationStage;
	switch (newImageLayout)
	{
	case vk::ImageLayout::eColorAttachmentOptimal: destinationStage = vk::PipelineStageFlagBits::eColorAttachmentOutput; break;
	case vk::ImageLayout::eDepthStencilAttachmentOptimal: destinationStage = vk::PipelineStageFlagBits::eEarlyFragmentTests; break;
	case vk::ImageLayout::eGeneral: destinationStage = vk::PipelineStageFlagBits::eHost; break;
	case vk::ImageLayout::ePresentSrcKHR: destinationStage = vk::PipelineStageFlagBits::eBottomOfPipe; break;
	case vk::ImageLayout::eShaderReadOnlyOptimal: destinationStage = vk::PipelineStageFlagBits::eFragmentShader; break;
	case vk::ImageLayout::eTransferDstOptimal:
	case vk::ImageLayout::eTransferSrcOptimal: destinationStage = vk::PipelineStageFlagBits::eTransfer; break;
	default: assert(false); break;
	}

	vk::ImageAspectFlags aspectMask;
	if (newImageLayout == vk::ImageLayout::eDepthStencilAttachmentOptimal)
	{
		aspectMask = vk::ImageAspectFlagBits::eDepth;
		if (format == vk::Format::eD32SfloatS8Uint || format == vk::Format::eD24UnormS8Uint)
		{
			aspectMask |= vk::ImageAspectFlagBits::eStencil;
		}
	}
	else
	{
		aspectMask = vk::ImageAspectFlagBits::eColor;
	}

	vk::ImageSubresourceRange imageSubresourceRange(aspectMask, 0, 1, 0, 1);
	vk::ImageMemoryBarrier    imageMemoryBarrier(sourceAccessMask,
		destinationAccessMask,
		oldImageLayout,
		newImageLayout,
		VK_QUEUE_FAMILY_IGNORED,
		VK_QUEUE_FAMILY_IGNORED,
		image,
		imageSubresourceRange);
	return commandBuffer.pipelineBarrier(sourceStage, destinationStage, {}, nullptr, nullptr, imageMemoryBarrier);
}

#endif