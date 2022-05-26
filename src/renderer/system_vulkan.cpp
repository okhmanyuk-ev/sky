#include "system_vulkan.h"

#if defined(RENDERER_VK)
#include <platform/system_windows.h>
#include <console/device.h>
#include <iostream>

using namespace Renderer;

struct Texture::TextureImpl
{

};

Texture::Texture(int width, int height, bool mipmap) :
	mWidth(width),
	mHeight(height),
	mMipmap(mipmap)
{
	mTextureImpl = std::make_unique<TextureImpl>();
}

Texture::Texture(int width, int height, int channels, void* data, bool mipmap) : Texture(width, height, mipmap)
{
	writePixels(width, height, channels, data);
}

Texture::~Texture()
{

}

void Texture::writePixels(int width, int height, int channels, void* data)
{
	assert(width == mWidth);
	assert(height == mHeight);
	assert(data);
}

struct RenderTarget::RenderTargetImpl
{

};

RenderTarget::RenderTarget(int width, int height) : Texture(width, height)
{
	mRenderTargetImpl = std::make_unique<RenderTargetImpl>();
}

RenderTarget::~RenderTarget()
{

}

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
	auto viewport = vk::Viewport()
		.setX(value.position.x)
		.setY(value.position.y)
		.setWidth(value.size.x)
		.setHeight(value.size.y)
		.setMinDepth(value.minDepth)
		.setMaxDepth(value.maxDepth);

	mCommandBuffer.setViewport(0, { viewport });
}

void SystemVK::setScissor(const Scissor& value)
{
	auto rect = vk::Rect2D()
		.setOffset({ static_cast<int32_t>(value.position.x), static_cast<int32_t>(value.position.y) })
		.setExtent({ static_cast<uint32_t>(value.size.x), static_cast<uint32_t>(value.size.y) });

	mCommandBuffer.setScissor(0, { rect });
}

void SystemVK::setScissor(std::nullptr_t value)
{
	setScissor({ { 0.0f, 0.0f }, { static_cast<float>(mWidth), static_cast<float>(mHeight) } });
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

const static std::unordered_map<ComparisonFunc, vk::CompareOp> CompareOpMap = {
	{ ComparisonFunc::Always, vk::CompareOp::eAlways },
	{ ComparisonFunc::Never, vk::CompareOp::eNever },
	{ ComparisonFunc::Less, vk::CompareOp::eLess },
	{ ComparisonFunc::Equal, vk::CompareOp::eEqual },
	{ ComparisonFunc::NotEqual, vk::CompareOp::eNotEqual },
	{ ComparisonFunc::LessEqual, vk::CompareOp::eLessOrEqual },
	{ ComparisonFunc::Greater, vk::CompareOp::eGreater },
	{ ComparisonFunc::GreaterEqual, vk::CompareOp::eGreaterOrEqual }
};

void SystemVK::setDepthMode(const DepthMode& value)
{
	mCommandBuffer.setDepthTestEnable(value.enabled);
	mCommandBuffer.setDepthCompareOp(CompareOpMap.at(value.func));
	mCommandBuffer.setDepthWriteEnable(true);
}

void SystemVK::setStencilMode(const StencilMode& value)
{
	mCommandBuffer.setStencilTestEnable(value.enabled);
}

void SystemVK::setCullMode(const CullMode& value)
{
	const static std::unordered_map<CullMode, vk::CullModeFlags> CullModeMap = {
		{ CullMode::None, vk::CullModeFlagBits::eNone },
		{ CullMode::Front, vk::CullModeFlagBits::eFront },
		{ CullMode::Back, vk::CullModeFlagBits::eBack },
	};

	mCommandBuffer.setCullMode(CullModeMap.at(value));
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
	// TODO: confirmed, just uncomment
	//mCommandBuffer.drawIndexed(indexCount, 1, indexOffset, vertexOffset, 0);
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
	drawTest();

	end();

	const auto& image_acquired_semaphore = mFrames.at(mSemaphoreIndex).image_acquired_semaphore;

	auto [result, image_index] = mSwapchain.acquireNextImage(UINT64_MAX, *image_acquired_semaphore);

	mFrameIndex = image_index;

	const auto& frame = mFrames.at(mFrameIndex);

	mDevice.waitForFences({ *frame.fence }, true, UINT64_MAX);
	mDevice.resetFences({ *frame.fence });

	auto begin_info = vk::CommandBufferBeginInfo()
		.setFlags(vk::CommandBufferUsageFlagBits::eOneTimeSubmit);

	const auto& cmd = frame.command_buffer;

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





















bool pipeline_created = false;

//-----------------------------------------------------------------------------
// SHADERS
//-----------------------------------------------------------------------------

// glsl_shader.vert, compiled with:
// # glslangValidator -V -x -o glsl_shader.vert.u32 glsl_shader.vert
/*
#version 450 core
layout(location = 0) in vec2 aPos;
layout(location = 1) in vec2 aUV;
layout(location = 2) in vec4 aColor;
layout(push_constant) uniform uPushConstant { vec2 uScale; vec2 uTranslate; } pc;

out gl_PerVertex { vec4 gl_Position; };
layout(location = 0) out struct { vec4 Color; vec2 UV; } Out;

void main()
{
	Out.Color = aColor;
	Out.UV = aUV;
	gl_Position = vec4(aPos * pc.uScale + pc.uTranslate, 0, 1);
}
*/
static uint32_t __glsl_shader_vert_spv[] =
{
	0x07230203,0x00010000,0x00080001,0x0000002e,0x00000000,0x00020011,0x00000001,0x0006000b,
	0x00000001,0x4c534c47,0x6474732e,0x3035342e,0x00000000,0x0003000e,0x00000000,0x00000001,
	0x000a000f,0x00000000,0x00000004,0x6e69616d,0x00000000,0x0000000b,0x0000000f,0x00000015,
	0x0000001b,0x0000001c,0x00030003,0x00000002,0x000001c2,0x00040005,0x00000004,0x6e69616d,
	0x00000000,0x00030005,0x00000009,0x00000000,0x00050006,0x00000009,0x00000000,0x6f6c6f43,
	0x00000072,0x00040006,0x00000009,0x00000001,0x00005655,0x00030005,0x0000000b,0x0074754f,
	0x00040005,0x0000000f,0x6c6f4361,0x0000726f,0x00030005,0x00000015,0x00565561,0x00060005,
	0x00000019,0x505f6c67,0x65567265,0x78657472,0x00000000,0x00060006,0x00000019,0x00000000,
	0x505f6c67,0x7469736f,0x006e6f69,0x00030005,0x0000001b,0x00000000,0x00040005,0x0000001c,
	0x736f5061,0x00000000,0x00060005,0x0000001e,0x73755075,0x6e6f4368,0x6e617473,0x00000074,
	0x00050006,0x0000001e,0x00000000,0x61635375,0x0000656c,0x00060006,0x0000001e,0x00000001,
	0x61725475,0x616c736e,0x00006574,0x00030005,0x00000020,0x00006370,0x00040047,0x0000000b,
	0x0000001e,0x00000000,0x00040047,0x0000000f,0x0000001e,0x00000002,0x00040047,0x00000015,
	0x0000001e,0x00000001,0x00050048,0x00000019,0x00000000,0x0000000b,0x00000000,0x00030047,
	0x00000019,0x00000002,0x00040047,0x0000001c,0x0000001e,0x00000000,0x00050048,0x0000001e,
	0x00000000,0x00000023,0x00000000,0x00050048,0x0000001e,0x00000001,0x00000023,0x00000008,
	0x00030047,0x0000001e,0x00000002,0x00020013,0x00000002,0x00030021,0x00000003,0x00000002,
	0x00030016,0x00000006,0x00000020,0x00040017,0x00000007,0x00000006,0x00000004,0x00040017,
	0x00000008,0x00000006,0x00000002,0x0004001e,0x00000009,0x00000007,0x00000008,0x00040020,
	0x0000000a,0x00000003,0x00000009,0x0004003b,0x0000000a,0x0000000b,0x00000003,0x00040015,
	0x0000000c,0x00000020,0x00000001,0x0004002b,0x0000000c,0x0000000d,0x00000000,0x00040020,
	0x0000000e,0x00000001,0x00000007,0x0004003b,0x0000000e,0x0000000f,0x00000001,0x00040020,
	0x00000011,0x00000003,0x00000007,0x0004002b,0x0000000c,0x00000013,0x00000001,0x00040020,
	0x00000014,0x00000001,0x00000008,0x0004003b,0x00000014,0x00000015,0x00000001,0x00040020,
	0x00000017,0x00000003,0x00000008,0x0003001e,0x00000019,0x00000007,0x00040020,0x0000001a,
	0x00000003,0x00000019,0x0004003b,0x0000001a,0x0000001b,0x00000003,0x0004003b,0x00000014,
	0x0000001c,0x00000001,0x0004001e,0x0000001e,0x00000008,0x00000008,0x00040020,0x0000001f,
	0x00000009,0x0000001e,0x0004003b,0x0000001f,0x00000020,0x00000009,0x00040020,0x00000021,
	0x00000009,0x00000008,0x0004002b,0x00000006,0x00000028,0x00000000,0x0004002b,0x00000006,
	0x00000029,0x3f800000,0x00050036,0x00000002,0x00000004,0x00000000,0x00000003,0x000200f8,
	0x00000005,0x0004003d,0x00000007,0x00000010,0x0000000f,0x00050041,0x00000011,0x00000012,
	0x0000000b,0x0000000d,0x0003003e,0x00000012,0x00000010,0x0004003d,0x00000008,0x00000016,
	0x00000015,0x00050041,0x00000017,0x00000018,0x0000000b,0x00000013,0x0003003e,0x00000018,
	0x00000016,0x0004003d,0x00000008,0x0000001d,0x0000001c,0x00050041,0x00000021,0x00000022,
	0x00000020,0x0000000d,0x0004003d,0x00000008,0x00000023,0x00000022,0x00050085,0x00000008,
	0x00000024,0x0000001d,0x00000023,0x00050041,0x00000021,0x00000025,0x00000020,0x00000013,
	0x0004003d,0x00000008,0x00000026,0x00000025,0x00050081,0x00000008,0x00000027,0x00000024,
	0x00000026,0x00050051,0x00000006,0x0000002a,0x00000027,0x00000000,0x00050051,0x00000006,
	0x0000002b,0x00000027,0x00000001,0x00070050,0x00000007,0x0000002c,0x0000002a,0x0000002b,
	0x00000028,0x00000029,0x00050041,0x00000011,0x0000002d,0x0000001b,0x0000000d,0x0003003e,
	0x0000002d,0x0000002c,0x000100fd,0x00010038
};

// glsl_shader.frag, compiled with:
// # glslangValidator -V -x -o glsl_shader.frag.u32 glsl_shader.frag
/*
#version 450 core
layout(location = 0) out vec4 fColor;
layout(set=0, binding=0) uniform sampler2D sTexture;
layout(location = 0) in struct { vec4 Color; vec2 UV; } In;
void main()
{
	fColor = In.Color * texture(sTexture, In.UV.st);
}
*/
static uint32_t __glsl_shader_frag_spv[] =
{
	0x07230203,0x00010000,0x00080001,0x0000001e,0x00000000,0x00020011,0x00000001,0x0006000b,
	0x00000001,0x4c534c47,0x6474732e,0x3035342e,0x00000000,0x0003000e,0x00000000,0x00000001,
	0x0007000f,0x00000004,0x00000004,0x6e69616d,0x00000000,0x00000009,0x0000000d,0x00030010,
	0x00000004,0x00000007,0x00030003,0x00000002,0x000001c2,0x00040005,0x00000004,0x6e69616d,
	0x00000000,0x00040005,0x00000009,0x6c6f4366,0x0000726f,0x00030005,0x0000000b,0x00000000,
	0x00050006,0x0000000b,0x00000000,0x6f6c6f43,0x00000072,0x00040006,0x0000000b,0x00000001,
	0x00005655,0x00030005,0x0000000d,0x00006e49,0x00050005,0x00000016,0x78655473,0x65727574,
	0x00000000,0x00040047,0x00000009,0x0000001e,0x00000000,0x00040047,0x0000000d,0x0000001e,
	0x00000000,0x00040047,0x00000016,0x00000022,0x00000000,0x00040047,0x00000016,0x00000021,
	0x00000000,0x00020013,0x00000002,0x00030021,0x00000003,0x00000002,0x00030016,0x00000006,
	0x00000020,0x00040017,0x00000007,0x00000006,0x00000004,0x00040020,0x00000008,0x00000003,
	0x00000007,0x0004003b,0x00000008,0x00000009,0x00000003,0x00040017,0x0000000a,0x00000006,
	0x00000002,0x0004001e,0x0000000b,0x00000007,0x0000000a,0x00040020,0x0000000c,0x00000001,
	0x0000000b,0x0004003b,0x0000000c,0x0000000d,0x00000001,0x00040015,0x0000000e,0x00000020,
	0x00000001,0x0004002b,0x0000000e,0x0000000f,0x00000000,0x00040020,0x00000010,0x00000001,
	0x00000007,0x00090019,0x00000013,0x00000006,0x00000001,0x00000000,0x00000000,0x00000000,
	0x00000001,0x00000000,0x0003001b,0x00000014,0x00000013,0x00040020,0x00000015,0x00000000,
	0x00000014,0x0004003b,0x00000015,0x00000016,0x00000000,0x0004002b,0x0000000e,0x00000018,
	0x00000001,0x00040020,0x00000019,0x00000001,0x0000000a,0x00050036,0x00000002,0x00000004,
	0x00000000,0x00000003,0x000200f8,0x00000005,0x00050041,0x00000010,0x00000011,0x0000000d,
	0x0000000f,0x0004003d,0x00000007,0x00000012,0x00000011,0x0004003d,0x00000014,0x00000017,
	0x00000016,0x00050041,0x00000019,0x0000001a,0x0000000d,0x00000018,0x0004003d,0x0000000a,
	0x0000001b,0x0000001a,0x00050057,0x00000007,0x0000001c,0x00000017,0x0000001b,0x00050085,
	0x00000007,0x0000001d,0x00000012,0x0000001c,0x0003003e,0x00000009,0x0000001d,0x000100fd,
	0x00010038
};

static uint32_t GetMemoryType(vk::MemoryPropertyFlags properties, uint32_t type_bits, vk::raii::PhysicalDevice& physical_device)
{
	auto prop = physical_device.getMemoryProperties();

	for (uint32_t i = 0; i < prop.memoryTypeCount; i++)
		if ((prop.memoryTypes[i].propertyFlags & properties) == properties && type_bits & (1 << i))
			return i;

	return 0xFFFFFFFF; // Unable to find memoryType
}

void SystemVK::drawTest()
{
	struct Vertex
	{
		glm::vec2 pos;
		glm::vec2 uv;
		glm::vec4 col;
	};

	if (!pipeline_created)
	{
		auto sampler_create_info = vk::SamplerCreateInfo()
			.setMagFilter(vk::Filter::eLinear)
			.setMinFilter(vk::Filter::eLinear)
			.setMipmapMode(vk::SamplerMipmapMode::eLinear)
			.setAddressModeU(vk::SamplerAddressMode::eRepeat)
			.setAddressModeV(vk::SamplerAddressMode::eRepeat)
			.setAddressModeW(vk::SamplerAddressMode::eRepeat)
			.setMinLod(-1000)
			.setMaxLod(1000)
			.setMaxAnisotropy(1.0f);

		mSampler = mDevice.createSampler(sampler_create_info);

		auto descriptor_set_layout_binding = vk::DescriptorSetLayoutBinding()
			.setDescriptorType(vk::DescriptorType::eCombinedImageSampler)
			.setDescriptorCount(1)
			.setStageFlags(vk::ShaderStageFlagBits::eFragment)
			.setPImmutableSamplers(&*mSampler);

		auto descriptor_set_layout_create_info = vk::DescriptorSetLayoutCreateInfo()
			.setBindingCount(1)
			.setPBindings(&descriptor_set_layout_binding);

		mDescriptorSetLayout = mDevice.createDescriptorSetLayout(descriptor_set_layout_create_info);

		auto push_constant_range = vk::PushConstantRange()
			.setStageFlags(vk::ShaderStageFlagBits::eVertex)
			.setOffset(0)
			.setSize(sizeof(float) * 4);

		auto pipeline_layout_create_info = vk::PipelineLayoutCreateInfo()
			.setSetLayoutCount(1)
			.setPSetLayouts(&*mDescriptorSetLayout)
			.setPushConstantRangeCount(1)
			.setPPushConstantRanges(&push_constant_range);

		mPipelineLayout = mDevice.createPipelineLayout(pipeline_layout_create_info);

		auto vertex_shader_module_create_info = vk::ShaderModuleCreateInfo()
			.setCodeSize(sizeof(__glsl_shader_vert_spv))
			.setPCode((uint32_t*)__glsl_shader_vert_spv);

		auto vertex_shader_module = mDevice.createShaderModule(vertex_shader_module_create_info);

		auto fragment_shader_module_create_info = vk::ShaderModuleCreateInfo()
			.setCodeSize(sizeof(__glsl_shader_frag_spv))
			.setPCode((uint32_t*)__glsl_shader_frag_spv);

		auto fragment_shader_module = mDevice.createShaderModule(fragment_shader_module_create_info);

		auto pipeline_shader_stage_create_info = {
			vk::PipelineShaderStageCreateInfo()
				.setStage(vk::ShaderStageFlagBits::eVertex)
				.setModule(*vertex_shader_module)
				.setPName("main"),

			vk::PipelineShaderStageCreateInfo()
				.setStage(vk::ShaderStageFlagBits::eFragment)
				.setModule(*fragment_shader_module)
				.setPName("main")
		};

		auto vertex_input_binding_description = vk::VertexInputBindingDescription()
			.setStride(sizeof(Vertex))
			.setInputRate(vk::VertexInputRate::eVertex)
			.setBinding(0);
		
		auto vertex_input_attribute_descriptions = {
			vk::VertexInputAttributeDescription()
				.setLocation(0)
				.setBinding(vertex_input_binding_description.binding)
				.setFormat(vk::Format::eR32G32Sfloat)
				.setOffset(offsetof(Vertex, pos)), // TODO: change IM_OFFSETOF -> offsetof

			vk::VertexInputAttributeDescription()
				.setLocation(1)
				.setBinding(vertex_input_binding_description.binding)
				.setFormat(vk::Format::eR32G32Sfloat)
				.setOffset(offsetof(Vertex, uv)),

			vk::VertexInputAttributeDescription()
				.setLocation(2)
				.setBinding(vertex_input_binding_description.binding)
				.setFormat(vk::Format::eR32G32B32A32Sfloat)
				.setOffset(offsetof(Vertex, col))
		};

		auto pipeline_vertex_input_state_create_info = vk::PipelineVertexInputStateCreateInfo()
			.setVertexBindingDescriptionCount(1)
			.setPVertexBindingDescriptions(&vertex_input_binding_description)
			.setVertexAttributeDescriptions(vertex_input_attribute_descriptions);

		auto pipeline_input_assembly_state_create_info = vk::PipelineInputAssemblyStateCreateInfo()
			.setTopology(vk::PrimitiveTopology::eTriangleList);

		auto pipeline_viewport_state_create_info = vk::PipelineViewportStateCreateInfo()
			.setViewportCount(1)
			.setScissorCount(1);

		auto pipeline_rasterization_state_create_info = vk::PipelineRasterizationStateCreateInfo()
			.setPolygonMode(vk::PolygonMode::eFill);

		auto pipeline_multisample_state_create_info = vk::PipelineMultisampleStateCreateInfo()
			.setRasterizationSamples(vk::SampleCountFlagBits::e1);

		auto pipeline_depth_stencil_state_create_info = vk::PipelineDepthStencilStateCreateInfo();

		auto pipeline_color_blent_attachment_state = vk::PipelineColorBlendAttachmentState()
			.setBlendEnable(true)
			.setSrcColorBlendFactor(vk::BlendFactor::eSrcAlpha)
			.setDstColorBlendFactor(vk::BlendFactor::eOneMinusSrcAlpha)
			.setColorBlendOp(vk::BlendOp::eAdd)
			.setSrcAlphaBlendFactor(vk::BlendFactor::eOne)
			.setDstAlphaBlendFactor(vk::BlendFactor::eOneMinusSrcAlpha)
			.setAlphaBlendOp(vk::BlendOp::eAdd)
			.setColorWriteMask(vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG |
				vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA);

		auto pipeline_color_blend_state_create_info = vk::PipelineColorBlendStateCreateInfo()
			.setAttachmentCount(1)
			.setPAttachments(&pipeline_color_blent_attachment_state);

		auto dynamic_states = {
			vk::DynamicState::eViewport,
			vk::DynamicState::eScissor,
			vk::DynamicState::ePrimitiveTopology,
			vk::DynamicState::eLineWidth,
			vk::DynamicState::eCullMode,
			vk::DynamicState::eFrontFace
		};

		auto pipeline_dynamic_state_create_info = vk::PipelineDynamicStateCreateInfo()
			.setDynamicStates(dynamic_states);
		
		auto pipeline_rendering_create_info = vk::PipelineRenderingCreateInfo()
			.setColorAttachmentCount(1)
			.setColorAttachmentFormats(mSurfaceFormat.format);

		auto graphics_pipeline_create_info = vk::GraphicsPipelineCreateInfo()
			.setLayout(*mPipelineLayout)
			.setFlags(vk::PipelineCreateFlagBits())
			.setStages(pipeline_shader_stage_create_info)
			.setPVertexInputState(&pipeline_vertex_input_state_create_info)
			.setPInputAssemblyState(&pipeline_input_assembly_state_create_info)
			.setPViewportState(&pipeline_viewport_state_create_info)
			.setPRasterizationState(&pipeline_rasterization_state_create_info)
			.setPMultisampleState(&pipeline_multisample_state_create_info)
			.setPDepthStencilState(&pipeline_depth_stencil_state_create_info)
			.setPColorBlendState(&pipeline_color_blend_state_create_info)
			.setPDynamicState(&pipeline_dynamic_state_create_info)
			.setRenderPass(nullptr)
			.setPNext(&pipeline_rendering_create_info);
			
		mPipeline = mDevice.createGraphicsPipeline(nullptr, graphics_pipeline_create_info);

		// vertex buffer

		auto vertex_buffer_size = 1024;

		auto vertex_buffer_create_info = vk::BufferCreateInfo()
			.setSize(vertex_buffer_size)
			.setUsage(vk::BufferUsageFlagBits::eVertexBuffer)
			.setSharingMode(vk::SharingMode::eExclusive);

		mVertexBuffer = mDevice.createBuffer(vertex_buffer_create_info);

		auto vertex_buffer_req = mVertexBuffer.getMemoryRequirements();

		auto vertex_buffer_memory_allocate_info = vk::MemoryAllocateInfo()
			.setAllocationSize(vertex_buffer_req.size)
			.setMemoryTypeIndex(GetMemoryType(vk::MemoryPropertyFlagBits::eHostVisible, vertex_buffer_req.memoryTypeBits, mPhysicalDevice));

		mVertexBufferMemory = mDevice.allocateMemory(vertex_buffer_memory_allocate_info);
		mVertexBufferSize = vertex_buffer_req.size;

		mVertexBuffer.bindMemory(*mVertexBufferMemory, 0);

		// index buffer

		auto index_buffer_size = 1024;

		auto index_buffer_create_info = vk::BufferCreateInfo()
			.setSize(index_buffer_size)
			.setUsage(vk::BufferUsageFlagBits::eIndexBuffer)
			.setSharingMode(vk::SharingMode::eExclusive);

		mIndexBuffer = mDevice.createBuffer(index_buffer_create_info);

		auto index_buffer_req = mIndexBuffer.getMemoryRequirements();

		auto index_buffer_memory_allocate_info = vk::MemoryAllocateInfo()
			.setAllocationSize(index_buffer_req.size)
			.setMemoryTypeIndex(GetMemoryType(vk::MemoryPropertyFlagBits::eHostVisible, index_buffer_req.memoryTypeBits, mPhysicalDevice));

		mIndexBufferMemory = mDevice.allocateMemory(index_buffer_memory_allocate_info);
		mIndexBufferSize = index_buffer_req.size;

		mIndexBuffer.bindMemory(*mIndexBufferMemory, 0);

		//



		// descriptor pool

		std::vector<vk::DescriptorPoolSize> pool_sizes = {
			{ vk::DescriptorType::eSampler, 1000 },
			{ vk::DescriptorType::eCombinedImageSampler, 1000 },
			{ vk::DescriptorType::eSampledImage, 1000 },
			{ vk::DescriptorType::eStorageImage, 1000 },
			{ vk::DescriptorType::eUniformTexelBuffer, 1000 },
			{ vk::DescriptorType::eStorageTexelBuffer, 1000 },
			{ vk::DescriptorType::eUniformBuffer, 1000 },
			{ vk::DescriptorType::eStorageBuffer, 1000 },
			{ vk::DescriptorType::eUniformBufferDynamic, 1000 },
			{ vk::DescriptorType::eStorageBufferDynamic, 1000 },
			{ vk::DescriptorType::eInputAttachment, 1000 }
		};
		
		auto descriptor_pool_create_info = vk::DescriptorPoolCreateInfo()
			.setFlags(vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet)
			.setMaxSets(uint32_t(1000 * pool_sizes.size()))
			.setPoolSizes(pool_sizes);

		mDescriptorPool = mDevice.createDescriptorPool(descriptor_pool_create_info);

		// texture
		{
			std::vector<uint32_t> tex_pixels = { 0xFFFFFFFF };
			uint32_t tex_width = 1;
			uint32_t tex_height = 1;
			auto tex_pixels_size = tex_pixels.size() * sizeof(uint32_t);

			auto descriptor_set_allocate_info = vk::DescriptorSetAllocateInfo()
				.setDescriptorPool(*mDescriptorPool)
				.setDescriptorSetCount(1)
				.setPSetLayouts(&*mDescriptorSetLayout);

			mTempDescriptorSet = std::move(mDevice.allocateDescriptorSets(descriptor_set_allocate_info).at(0));

			auto image_create_info = vk::ImageCreateInfo()
				.setImageType(vk::ImageType::e2D)
				.setFormat(vk::Format::eR8G8B8A8Unorm)
				.setExtent({ tex_width, tex_height, 1 })
				.setMipLevels(1)
				.setArrayLayers(1)
				.setSamples(vk::SampleCountFlagBits::e1)
				.setTiling(vk::ImageTiling::eOptimal)
				.setUsage(vk::ImageUsageFlagBits::eSampled | vk::ImageUsageFlagBits::eTransferDst)
				.setSharingMode(vk::SharingMode::eExclusive)
				.setInitialLayout(vk::ImageLayout::eUndefined);

			mTempImage = mDevice.createImage(image_create_info);

			{
				auto req = mTempImage.getMemoryRequirements();

				auto memory_allocate_info = vk::MemoryAllocateInfo()
					.setAllocationSize(req.size)
					.setMemoryTypeIndex(GetMemoryType(vk::MemoryPropertyFlagBits::eDeviceLocal, req.memoryTypeBits, mPhysicalDevice));

				mTempMemory = mDevice.allocateMemory(memory_allocate_info);
			}
			mTempImage.bindMemory(*mTempMemory, 0);

			auto image_subresource_range = vk::ImageSubresourceRange()
				.setAspectMask(vk::ImageAspectFlagBits::eColor)
				.setLevelCount(1)
				.setLayerCount(1);

			auto image_view_create_info = vk::ImageViewCreateInfo()
				.setImage(*mTempImage)
				.setViewType(vk::ImageViewType::e2D)
				.setFormat(vk::Format::eR8G8B8A8Unorm)
				.setSubresourceRange(image_subresource_range);

			mTempImageView = mDevice.createImageView(image_view_create_info);

			auto descriptor_image_info = vk::DescriptorImageInfo()
				.setSampler(*mSampler)
				.setImageView(*mTempImageView)
				.setImageLayout(vk::ImageLayout::eShaderReadOnlyOptimal);

			auto write_descriptor_set = vk::WriteDescriptorSet()
				.setDstSet(*mTempDescriptorSet)
				.setDescriptorCount(1)
				.setDescriptorType(vk::DescriptorType::eCombinedImageSampler)
				.setPImageInfo(&descriptor_image_info);

			mDevice.updateDescriptorSets({ write_descriptor_set }, {});

			{
				auto buffer_create_info = vk::BufferCreateInfo()
					.setSize(tex_pixels_size)
					.setUsage(vk::BufferUsageFlagBits::eTransferSrc)
					.setSharingMode(vk::SharingMode::eExclusive);

				auto upload_buffer = mDevice.createBuffer(buffer_create_info);
			
				auto req = upload_buffer.getMemoryRequirements();

				//bd->BufferMemoryAlignment = (bd->BufferMemoryAlignment > req.alignment) ? bd->BufferMemoryAlignment : req.alignment;

				auto memory_allocate_info = vk::MemoryAllocateInfo()
					.setAllocationSize(req.size)
					.setMemoryTypeIndex(GetMemoryType(vk::MemoryPropertyFlagBits::eHostVisible, req.memoryTypeBits, mPhysicalDevice));

				auto upload_buffer_memory = mDevice.allocateMemory(memory_allocate_info);

				upload_buffer.bindMemory(*upload_buffer_memory, 0);

				auto map = upload_buffer_memory.mapMemory(0, tex_pixels_size);
				memcpy(map, tex_pixels.data(), tex_pixels_size);
				upload_buffer_memory.unmapMemory();

				oneTimeSubmit(mDevice, mCommandPool, mQueue, [&](auto& cmd) {
					setImageLayout(cmd, *mTempImage, vk::Format::eUndefined, vk::ImageLayout::eUndefined,
						vk::ImageLayout::eTransferDstOptimal);

					auto image_subresource_layers = vk::ImageSubresourceLayers()
						.setAspectMask(vk::ImageAspectFlagBits::eColor)
						.setLayerCount(1);

					auto region = vk::BufferImageCopy()
						.setImageSubresource(image_subresource_layers)
						.setImageExtent({ tex_width, tex_height, 1 });

					cmd.copyBufferToImage(*upload_buffer, *mTempImage, vk::ImageLayout::eTransferDstOptimal, { region });

					setImageLayout(cmd, *mTempImage, vk::Format::eUndefined, vk::ImageLayout::eTransferDstOptimal,
						vk::ImageLayout::eShaderReadOnlyOptimal);
				});
			}
		}
		//

		pipeline_created = true;
	}

	std::vector<Vertex> vertices = {
		Vertex{ { 0.0f, -0.5f }, { 0.0f, 0.0f }, { 0.0f, 1.0f, 0.0f, 1.0f } },
		Vertex{ { -0.5f, 0.5f }, { 0.0f, 0.0f }, { 1.0f, 0.0f, 0.0f, 1.0f } },
		Vertex{ { 0.5f, 0.5f }, { 0.0f, 0.0f }, { 0.0f, 0.0f, 1.0f, 1.0f } },
	};

	std::vector<uint32_t> indices = { 0, 1, 2 };

	auto vertices_mem = mVertexBufferMemory.mapMemory(0, VK_WHOLE_SIZE);
	memcpy(vertices_mem, vertices.data(), vertices.size() * sizeof(Vertex));

	auto indices_mem = mIndexBufferMemory.mapMemory(0, VK_WHOLE_SIZE);
	memcpy(indices_mem, indices.data(), indices.size() * sizeof(uint32_t));

	/*auto vertex_buffer_mapped_memory_range = vk::MappedMemoryRange()
		.setMemory(*mVertexBufferMemory)
		.setSize(VK_WHOLE_SIZE);

	auto index_buffer_mapped_memory_range = vk::MappedMemoryRange()
		.setMemory(*mIndexBufferMemory)
		.setSize(VK_WHOLE_SIZE);

	mDevice.flushMappedMemoryRanges({ vertex_buffer_mapped_memory_range, index_buffer_mapped_memory_range });*/
	mVertexBufferMemory.unmapMemory();
	mIndexBufferMemory.unmapMemory();

	struct PushConstants
	{
		glm::vec2 scale;
		glm::vec2 translate;
	};

	auto push_constants = PushConstants{ { 1.0f, 1.0f }, { 0.0f, 0.0f } };

	mCommandBuffer.pushConstants<PushConstants>(*mPipelineLayout, vk::ShaderStageFlagBits::eVertex, 0, { push_constants });
	mCommandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, *mPipeline);
	mCommandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, *mPipelineLayout, 0, { *mTempDescriptorSet }, {});
	mCommandBuffer.bindIndexBuffer(*mIndexBuffer, 0, vk::IndexType::eUint32);
	mCommandBuffer.bindVertexBuffers(0, { *mVertexBuffer }, { 0 });
	mCommandBuffer.setFrontFace(vk::FrontFace::eCounterClockwise);
	mCommandBuffer.setLineWidth(1.0f);
	setTopology(Renderer::Topology::TriangleList);
	setCullMode(Renderer::CullMode::None);
	setViewport(Renderer::Viewport());
	setScissor(nullptr);
	mCommandBuffer.drawIndexed(3, 1, 0, 0, 0);

	std::vector<Vertex> vertices2 = {
		Vertex{ { -0.5f, 0.0f }, { 0.0f, 0.0f }, { 1.0f, 1.0f, 0.0f, 0.5f } },
		Vertex{ { 0.5f, -0.5f }, { 0.0f, 0.0f }, { 1.0f, 0.0f, 1.0f, 0.5f } },
		Vertex{ { 0.5f, 0.5f }, { 0.0f, 0.0f }, { 0.0f, 1.0f, 1.0f, 0.5f } },
	};

	/*auto vertices_mem2 = mVertexBufferMemory.mapMemory(0, VK_WHOLE_SIZE);
	memcpy(vertices_mem2, vertices2.data(), vertices2.size() * sizeof(Vertex));

	auto vertex_buffer_mapped_memory_range2 = vk::MappedMemoryRange()
		.setMemory(*mVertexBufferMemory)
		.setSize(VK_WHOLE_SIZE);

	mDevice.flushMappedMemoryRanges({ vertex_buffer_mapped_memory_range2 });
	mVertexBufferMemory.unmapMemory();

	mCommandBuffer.drawIndexed(3, 1, 0, 0, 0);*/
}

#endif