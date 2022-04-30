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

SystemVK::SystemVK()
{
	auto all_extensions = mContext.enumerateInstanceExtensionProperties();

	for (auto extension : all_extensions)
	{
	//	std::cout << extension.extensionName << std::endl;
	}

	auto all_layers = mContext.enumerateInstanceLayerProperties();

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

	auto version = mContext.enumerateInstanceVersion();

	auto major_version = VK_API_VERSION_MAJOR(version);
	auto minor_version = VK_API_VERSION_MINOR(version);

	std::cout << "available vulkan version: " << major_version << "." << minor_version << std::endl;

	auto application_info = vk::ApplicationInfo()
		.setApiVersion(VK_API_VERSION_1_3);

	auto instance_info = vk::InstanceCreateInfo()
		.setPEnabledExtensionNames(extensions)
		.setPEnabledLayerNames(layers)
		.setPApplicationInfo(&application_info);

	mInstance = mContext.createInstance(instance_info);

	auto devices = mInstance.enumeratePhysicalDevices();
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

	mPhysicalDevice = std::move(devices.at(device_index));

	auto properties = mPhysicalDevice.getQueueFamilyProperties();

	for (size_t i = 0; i < properties.size(); i++)
	{
		if (properties[i].queueFlags & vk::QueueFlagBits::eGraphics)
		{
			mQueueFamilyIndex = i;
			break;
		}
	}

	auto all_device_extensions = mPhysicalDevice.enumerateDeviceExtensionProperties();

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
		.setQueueFamilyIndex(mQueueFamilyIndex)
		.setQueuePriorities(queue_priority);

	auto device_features = mPhysicalDevice.getFeatures2<vk::PhysicalDeviceFeatures2, vk::PhysicalDeviceVulkan13Features>();

	auto device_info = vk::DeviceCreateInfo()
		.setQueueCreateInfoCount(1)
		.setPQueueCreateInfos(&queue_info)
		.setPEnabledExtensionNames(device_extensions)
		.setPEnabledFeatures(nullptr)
		.setPNext(&device_features);

	mDevice = mPhysicalDevice.createDevice(device_info);
	mQueue = mDevice.getQueue(mQueueFamilyIndex, 0);

	auto surface_info = vk::Win32SurfaceCreateInfoKHR()
		.setHinstance(Platform::SystemWindows::Instance)
		.setHwnd(Platform::SystemWindows::Window);
	
	mSurface = vk::raii::SurfaceKHR(mInstance, surface_info);

	auto formats = mPhysicalDevice.getSurfaceFormatsKHR(*mSurface);

	if ((formats.size() == 1) && (formats.at(0).format == vk::Format::eUndefined))
	{
		mSurfaceFormat = {
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
				mSurfaceFormat = format;
				found = true;
				break;
			}
		}
		if (!found)
		{
			mSurfaceFormat = formats.at(0);
		}
	}

	auto command_pool_info = vk::CommandPoolCreateInfo()
		.setFlags(vk::CommandPoolCreateFlagBits::eResetCommandBuffer)
		.setQueueFamilyIndex(mQueueFamilyIndex);

	mCommandPool = mDevice.createCommandPool(command_pool_info);

	auto command_buffer_allocate_info = vk::CommandBufferAllocateInfo()
		.setCommandBufferCount(1)
		.setLevel(vk::CommandBufferLevel::eSecondary)
		.setCommandPool(*mCommandPool);

	auto command_buffers = mDevice.allocateCommandBuffers(command_buffer_allocate_info);
	mCommandBuffer = std::move(command_buffers.at(0));

	createSwapchain();

	begin();
}

SystemVK::~SystemVK()
{
	end();
}

void SystemVK::createSwapchain()
{
	mWidth = static_cast<uint32_t>(PLATFORM->getWidth());
	mHeight = static_cast<uint32_t>(PLATFORM->getHeight());

	auto swapchain_info = vk::SwapchainCreateInfoKHR()
		.setSurface(*mSurface)
		.setMinImageCount(mMinImageCount)
		.setImageFormat(mSurfaceFormat.format)
		.setImageColorSpace(mSurfaceFormat.colorSpace)
		.setImageExtent({ mWidth, mHeight })
		.setImageUsage(vk::ImageUsageFlagBits::eColorAttachment)
		.setPreTransform(vk::SurfaceTransformFlagBitsKHR::eIdentity)
		.setImageArrayLayers(1)
		.setImageSharingMode(vk::SharingMode::eExclusive)
		.setQueueFamilyIndexCount(1)
		.setPQueueFamilyIndices(&mQueueFamilyIndex)
		.setPresentMode(vk::PresentModeKHR::eFifo)
		.setClipped(true)
		.setCompositeAlpha(vk::CompositeAlphaFlagBitsKHR::eOpaque)
		.setOldSwapchain(*mSwapchain);

	mSwapchain = mDevice.createSwapchainKHR(swapchain_info);

	auto backbuffers = mSwapchain.getImages();

	mFrames.clear();

	for (auto& backbuffer : backbuffers)
	{
		auto frame = Frame();

		auto buffer_allocate_info = vk::CommandBufferAllocateInfo()
			.setCommandBufferCount(1)
			.setLevel(vk::CommandBufferLevel::ePrimary)
			.setCommandPool(*mCommandPool);

		auto command_buffers = mDevice.allocateCommandBuffers(buffer_allocate_info);
		frame.command_buffer = std::move(command_buffers.at(0));

		auto fence_info = vk::FenceCreateInfo()
			.setFlags(vk::FenceCreateFlagBits::eSignaled);

		frame.fence = mDevice.createFence(fence_info);

		frame.image_acquired_semaphore = mDevice.createSemaphore({});
		frame.render_complete_semaphore = mDevice.createSemaphore({});

		auto image_view_info = vk::ImageViewCreateInfo()
			.setViewType(vk::ImageViewType::e2D)
			.setFormat(mSurfaceFormat.format)
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

		frame.backbuffer_view = mDevice.createImageView(image_view_info);

		oneTimeSubmit(mDevice, mCommandPool, mQueue, [&](auto& cmd) {
			setImageLayout(cmd, backbuffer, mSurfaceFormat.format, vk::ImageLayout::eUndefined, 
				vk::ImageLayout::ePresentSrcKHR);
		});

		mFrames.push_back(std::move(frame));
	}
}

void SystemVK::onEvent(const Platform::System::ResizeEvent& e)
{
	createSwapchain();
}

void SystemVK::setTopology(const Renderer::Topology& value)
{
	static const std::map<Renderer::Topology, vk::PrimitiveTopology> TopologyMap = {
		{ Renderer::Topology::PointList, vk::PrimitiveTopology::ePointList },
		{ Renderer::Topology::LineList, vk::PrimitiveTopology::eLineList },
		{ Renderer::Topology::LineStrip, vk::PrimitiveTopology::eLineStrip },
		{ Renderer::Topology::TriangleList, vk::PrimitiveTopology::eTriangleList },
		{ Renderer::Topology::TriangleStrip, vk::PrimitiveTopology::eTriangleStrip },
	};

	mCommandBuffer.setPrimitiveTopology(TopologyMap.at(value));
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
	auto clear_rect = vk::ClearRect()
		.setBaseArrayLayer(0)
		.setLayerCount(1)
		.setRect({ { 0, 0 }, { mWidth, mHeight } });

	if (color.has_value())
	{
		auto value = color.value();

		auto clear_color_value = vk::ClearColorValue()
			.setFloat32({ value.r, value.g, value.b, value.a });

		auto clear_value = vk::ClearValue()
			.setColor(clear_color_value);
		
		auto attachment = vk::ClearAttachment()
			.setAspectMask(vk::ImageAspectFlagBits::eColor)
			.setColorAttachment(0)
			.setClearValue(clear_value);

		mCommandBuffer.clearAttachments({ attachment }, { clear_rect });
	}

	if (depth.has_value() || stencil.has_value())
	{
		auto clear_depth_stencil_value = vk::ClearDepthStencilValue()
			.setDepth(depth.value_or(1.0f))
			.setStencil((uint32_t)stencil.value_or(0)); // TODO: maybe we should change argument uint8_t -> uint32_t

		auto clear_value = vk::ClearValue()
			.setDepthStencil(clear_depth_stencil_value);

		auto aspect_mask = vk::ImageAspectFlags();

		if (depth.has_value())
			aspect_mask |= vk::ImageAspectFlagBits::eDepth;

		if (stencil.has_value())
			aspect_mask |= vk::ImageAspectFlagBits::eStencil;

		auto attachment = vk::ClearAttachment()
			.setAspectMask(aspect_mask)
			.setColorAttachment(0)
			.setClearValue(clear_value);

		mCommandBuffer.clearAttachments({ attachment }, { clear_rect });
	}
}

void SystemVK::draw(size_t vertexCount, size_t vertexOffset)
{
	//mCommandBuffer.draw(vertexCount, 0, vertexOffset, 0);
}

void SystemVK::drawIndexed(size_t indexCount, size_t indexOffset, size_t vertexOffset)
{
	//mCommandBuffer.drawIndexed(indexCount, 0, indexOffset, vertexOffset, 0);
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
	end();

	const auto& image_acquired_semaphore = mFrames.at(mSemaphoreIndex).image_acquired_semaphore;

	auto [result, image_index] = mSwapchain.acquireNextImage(UINT64_MAX, *image_acquired_semaphore);

	mFrameIndex = image_index;

	const auto& frame = getFrame();

	mDevice.waitForFences({ *frame.fence }, true, UINT64_MAX);
	mDevice.resetFences({ *frame.fence });

	auto begin_info = vk::CommandBufferBeginInfo()
		.setFlags(vk::CommandBufferUsageFlagBits::eOneTimeSubmit);

	const auto& cmd = getCommandBuffer();

	cmd.begin(begin_info);

	auto color_attachment = vk::RenderingAttachmentInfo()
		.setImageView(*frame.backbuffer_view)
		.setImageLayout(vk::ImageLayout::eAttachmentOptimal)
		.setLoadOp(vk::AttachmentLoadOp::eDontCare)
		.setStoreOp(vk::AttachmentStoreOp::eStore);

	auto rendering_info = vk::RenderingInfo()
		.setRenderArea({ { 0, 0 }, { mWidth, mHeight } })
		.setLayerCount(1)
		.setColorAttachmentCount(1)
		.setPColorAttachments(&color_attachment)
		.setFlags(vk::RenderingFlagBits::eContentsSecondaryCommandBuffers);

	cmd.beginRendering(rendering_info);
	cmd.executeCommands({ *mCommandBuffer });
	cmd.endRendering();
	cmd.end();

	const auto& render_complete_semaphore = mFrames.at(mSemaphoreIndex).render_complete_semaphore;

	vk::PipelineStageFlags wait_dst_stage_mask = vk::PipelineStageFlagBits::eColorAttachmentOutput;

	auto submit_info = vk::SubmitInfo()
		.setPWaitDstStageMask(&wait_dst_stage_mask)
		.setWaitSemaphoreCount(1)
		.setPWaitSemaphores(&*image_acquired_semaphore)
		.setCommandBufferCount(1)
		.setPCommandBuffers(&*frame.command_buffer)
		.setSignalSemaphoreCount(1)
		.setPSignalSemaphores(&*render_complete_semaphore);

	mQueue.submit({ submit_info }, *frame.fence); // TODO: can be called with no fence, check it out

	auto present_info = vk::PresentInfoKHR()
		.setWaitSemaphoreCount(1)
		.setPWaitSemaphores(&*render_complete_semaphore)
		.setSwapchainCount(1)
		.setPSwapchains(&*mSwapchain)
		.setPImageIndices(&mFrameIndex);

	mQueue.presentKHR(present_info);
	mQueue.waitIdle();

	mSemaphoreIndex = (mSemaphoreIndex + 1) % mFrames.size(); // TODO: maybe gFrameIndex can be used for both

	begin();
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

void SystemVK::begin()
{
	assert(!mWorking);
	mWorking = true;

	auto inheritance_rendering_info = vk::CommandBufferInheritanceRenderingInfo()
		.setColorAttachmentCount(1)
		.setPColorAttachmentFormats(&mSurfaceFormat.format)
		.setRasterizationSamples(vk::SampleCountFlagBits::e1);

	auto inheritance_info = vk::CommandBufferInheritanceInfo()
		.setPNext(&inheritance_rendering_info);

	auto begin_info = vk::CommandBufferBeginInfo()
		.setFlags(vk::CommandBufferUsageFlagBits::eRenderPassContinue)
		.setPInheritanceInfo(&inheritance_info);

	mCommandBuffer.begin(begin_info);
}

void SystemVK::end()
{
	assert(mWorking);
	mWorking = false;

	mCommandBuffer.end();
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