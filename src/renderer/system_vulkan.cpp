#include "system_vulkan.h"

#if defined(RENDERER_VK)
#include <platform/system_windows.h>
#include <iostream>
#include <console/device.h>
#include <skygfx/shader_compiler.h>

using namespace Renderer;

bool pipeline_created = false;

static uint32_t GetMemoryType(vk::MemoryPropertyFlags properties, uint32_t type_bits, vk::raii::PhysicalDevice& physical_device)
{
	auto prop = physical_device.getMemoryProperties();

	for (uint32_t i = 0; i < prop.memoryTypeCount; i++)
		if ((prop.memoryTypes[i].propertyFlags & properties) == properties && type_bits & (1 << i))
			return i;

	return 0xFFFFFFFF; // Unable to find memoryType
}

struct Shader::Impl
{
};

Shader::Shader(const Vertex::Layout& layout, const std::string& vertex_code, const std::string& fragment_code)
{
}

Shader::~Shader()
{
}

void Shader::apply()
{
}

struct Texture::TextureImpl
{
	vk::raii::Image image = nullptr;
	vk::raii::ImageView image_view = nullptr;
	vk::raii::DeviceMemory memory = nullptr;
};

Texture::Texture(int width, int height, int channels, void* data, bool mipmap) : // TODO: make unit32_t
	mWidth(width),
	mHeight(height),
	mMipmap(mipmap)
{
	mTextureImpl = std::make_unique<TextureImpl>();

	auto image_create_info = vk::ImageCreateInfo()
		.setImageType(vk::ImageType::e2D)
		.setFormat(vk::Format::eR8G8B8A8Unorm)
		.setExtent({ static_cast<uint32_t>(width), static_cast<uint32_t>(height), 1 })
		.setMipLevels(1)
		.setArrayLayers(1)
		.setSamples(vk::SampleCountFlagBits::e1)
		.setTiling(vk::ImageTiling::eOptimal)
		.setUsage(vk::ImageUsageFlagBits::eSampled | vk::ImageUsageFlagBits::eTransferDst)
		.setSharingMode(vk::SharingMode::eExclusive)
		.setInitialLayout(vk::ImageLayout::eUndefined);

	auto image = SystemVK::mDevice.createImage(image_create_info);

	auto memory_requirements = image.getMemoryRequirements();

	auto memory_allocate_info = vk::MemoryAllocateInfo()
		.setAllocationSize(memory_requirements.size)
		.setMemoryTypeIndex(GetMemoryType(vk::MemoryPropertyFlagBits::eDeviceLocal, memory_requirements.memoryTypeBits, SystemVK::mPhysicalDevice));

	auto memory = SystemVK::mDevice.allocateMemory(memory_allocate_info);
	
	image.bindMemory(*memory, 0);

	auto image_subresource_range = vk::ImageSubresourceRange()
		.setAspectMask(vk::ImageAspectFlagBits::eColor)
		.setLevelCount(1)
		.setLayerCount(1);

	auto image_view_create_info = vk::ImageViewCreateInfo()
		.setImage(*image)
		.setViewType(vk::ImageViewType::e2D)
		.setFormat(vk::Format::eR8G8B8A8Unorm)
		.setSubresourceRange(image_subresource_range);

	auto image_view = SystemVK::mDevice.createImageView(image_view_create_info);

	mTextureImpl->image = std::move(image);
	mTextureImpl->image_view = std::move(image_view);
	mTextureImpl->memory = std::move(memory);

	if (data)
	{
		auto size = width * height * channels;

		auto buffer_create_info = vk::BufferCreateInfo()
			.setSize(size)
			.setUsage(vk::BufferUsageFlagBits::eTransferSrc)
			.setSharingMode(vk::SharingMode::eExclusive);

		auto upload_buffer = SystemVK::mDevice.createBuffer(buffer_create_info);

		auto req = upload_buffer.getMemoryRequirements();

		//bd->BufferMemoryAlignment = (bd->BufferMemoryAlignment > req.alignment) ? bd->BufferMemoryAlignment : req.alignment;

		auto memory_allocate_info = vk::MemoryAllocateInfo()
			.setAllocationSize(req.size)
			.setMemoryTypeIndex(GetMemoryType(vk::MemoryPropertyFlagBits::eHostVisible, req.memoryTypeBits, SystemVK::mPhysicalDevice));

		auto upload_buffer_memory = SystemVK::mDevice.allocateMemory(memory_allocate_info);

		upload_buffer.bindMemory(*upload_buffer_memory, 0);

		auto map = upload_buffer_memory.mapMemory(0, size);
		memcpy(map, data, size);
		upload_buffer_memory.unmapMemory();

		SystemVK::oneTimeSubmit(SystemVK::mDevice, SystemVK::mCommandPool, SystemVK::mQueue, [&](auto& cmd) {
			SystemVK::setImageLayout(cmd, *mTextureImpl->image, vk::Format::eUndefined, vk::ImageLayout::eUndefined,
				vk::ImageLayout::eTransferDstOptimal);

			auto image_subresource_layers = vk::ImageSubresourceLayers()
				.setAspectMask(vk::ImageAspectFlagBits::eColor)
				.setLayerCount(1);

			auto region = vk::BufferImageCopy()
				.setImageSubresource(image_subresource_layers)
				.setImageExtent({ static_cast<uint32_t>(width), static_cast<uint32_t>(height), 1 });

			cmd.copyBufferToImage(*upload_buffer, *mTextureImpl->image, vk::ImageLayout::eTransferDstOptimal, { region });

			SystemVK::setImageLayout(cmd, *mTextureImpl->image, vk::Format::eUndefined, vk::ImageLayout::eTransferDstOptimal,
				vk::ImageLayout::eShaderReadOnlyOptimal);
		});
	}
}

Texture::~Texture()
{

}

struct RenderTarget::RenderTargetImpl
{

};

RenderTarget::RenderTarget(int width, int height) : Texture(width, height, 4, nullptr)
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
	//	std::cout << layer.layerName << std::endl;
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
		VK_KHR_PUSH_DESCRIPTOR_EXTENSION_NAME,
		VK_EXT_VERTEX_INPUT_DYNAMIC_STATE_EXTENSION_NAME,
	};

	auto queue_priority = { 1.0f };

	auto queue_info = vk::DeviceQueueCreateInfo()
		.setQueueFamilyIndex(mQueueFamilyIndex)
		.setQueuePriorities(queue_priority);

	auto device_features = mPhysicalDevice.getFeatures2<vk::PhysicalDeviceFeatures2, 
		vk::PhysicalDeviceVulkan13Features,
		vk::PhysicalDeviceVertexInputDynamicStateFeaturesEXT>();

	//auto device_properties = mPhysicalDevice.getProperties2<vk::PhysicalDeviceProperties2, 
	//	vk::PhysicalDeviceVulkan13Properties>(); // TODO: unused

	auto device_info = vk::DeviceCreateInfo()
		.setQueueCreateInfoCount(1)
		.setPQueueCreateInfos(&queue_info)
		.setPEnabledExtensionNames(device_extensions)
		.setPEnabledFeatures(nullptr)
		.setPNext(&device_features.get<vk::PhysicalDeviceFeatures2>());

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

	auto descriptor_set_layout_binding = vk::DescriptorSetLayoutBinding()
		.setDescriptorType(vk::DescriptorType::eCombinedImageSampler)
		.setDescriptorCount(1)
		.setStageFlags(vk::ShaderStageFlagBits::eFragment);

	auto descriptor_set_layout_create_info = vk::DescriptorSetLayoutCreateInfo()
		.setBindingCount(1)
		.setFlags(vk::DescriptorSetLayoutCreateFlagBits::ePushDescriptorKHR)
		.setPBindings(&descriptor_set_layout_binding);

	mDescriptorSetLayout = mDevice.createDescriptorSetLayout(descriptor_set_layout_create_info);

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
	assert(value.size > 0);

	auto create_buffer = [&] {
		DeviceBuffer buffer;

		auto buffer_create_info = vk::BufferCreateInfo()
			.setSize(value.size)
			.setUsage(vk::BufferUsageFlagBits::eVertexBuffer)
			.setSharingMode(vk::SharingMode::eExclusive);

		buffer.buffer = mDevice.createBuffer(buffer_create_info);

		auto memory_requirements = buffer.buffer.getMemoryRequirements();

		auto memory_allocate_info = vk::MemoryAllocateInfo()
			.setAllocationSize(memory_requirements.size)
			.setMemoryTypeIndex(GetMemoryType(vk::MemoryPropertyFlagBits::eHostVisible, memory_requirements.memoryTypeBits, mPhysicalDevice));

		buffer.memory = mDevice.allocateMemory(memory_allocate_info);
		buffer.size = memory_requirements.size;

		buffer.buffer.bindMemory(*buffer.memory, 0);

		return buffer;
	};

	if (mVertexBuffers.size() < mVertexBufferIndex + 1)
	{
		assert(mVertexBuffers.size() == mVertexBufferIndex);
		mVertexBuffers.push_back(create_buffer());
	}

	auto& buffer = mVertexBuffers[mVertexBufferIndex];

	if (buffer.size < value.size)
	{
		buffer = create_buffer();
	}

	auto mem = buffer.memory.mapMemory(0, VK_WHOLE_SIZE);
	memcpy(mem, value.data, value.size);
	buffer.memory.unmapMemory();

	mCommandBuffer.bindVertexBuffers2(0, { *buffer.buffer }, { 0 }, nullptr, { value.stride });
	
	mVertexBufferIndex += 1;
}

void SystemVK::setIndexBuffer(const Buffer& value)
{
	assert(value.size > 0);

	auto create_buffer = [&] {
		DeviceBuffer buffer;

		auto buffer_create_info = vk::BufferCreateInfo()
			.setSize(value.size)
			.setUsage(vk::BufferUsageFlagBits::eIndexBuffer)
			.setSharingMode(vk::SharingMode::eExclusive);

		buffer.buffer = mDevice.createBuffer(buffer_create_info);

		auto memory_requirements = buffer.buffer.getMemoryRequirements();

		auto memory_allocate_info = vk::MemoryAllocateInfo()
			.setAllocationSize(memory_requirements.size)
			.setMemoryTypeIndex(GetMemoryType(vk::MemoryPropertyFlagBits::eHostVisible, memory_requirements.memoryTypeBits, mPhysicalDevice));

		buffer.memory = mDevice.allocateMemory(memory_allocate_info);
		buffer.size = memory_requirements.size;

		buffer.buffer.bindMemory(*buffer.memory, 0);

		return buffer;
	};

	if (mIndexBuffers.size() < mIndexBufferIndex + 1)
	{
		assert(mIndexBuffers.size() == mIndexBufferIndex);
		mIndexBuffers.push_back(create_buffer());
	}

	auto& buffer = mIndexBuffers[mIndexBufferIndex];

	if (buffer.size < value.size)
	{
		buffer = create_buffer();
	}

	auto mem = buffer.memory.mapMemory(0, VK_WHOLE_SIZE);
	memcpy(mem, value.data, value.size);
	buffer.memory.unmapMemory();

	mCommandBuffer.bindIndexBuffer(*buffer.buffer, 0, value.stride == 2 ? vk::IndexType::eUint16 : vk::IndexType::eUint32);
	
	mIndexBufferIndex += 1;
}

void SystemVK::setUniformBuffer(int slot, void* memory, size_t size)
{
	//
}

void SystemVK::setTexture(int binding, std::shared_ptr<Texture> value)
{
	if (value == nullptr)
		return;

	if (!pipeline_created) // TODO: remove when pipeline layout will be nice constructed 
		return;

	auto descriptor_image_info = vk::DescriptorImageInfo()
		.setSampler(*mSampler)
		.setImageView(*value->mTextureImpl->image_view)
		.setImageLayout(vk::ImageLayout::eShaderReadOnlyOptimal);

	auto write_descriptor_set = vk::WriteDescriptorSet()
		.setDescriptorCount(1)
		.setDescriptorType(vk::DescriptorType::eCombinedImageSampler)
		.setPImageInfo(&descriptor_image_info);

	mCommandBuffer.pushDescriptorSetKHR(vk::PipelineBindPoint::eGraphics, *mPipelineLayout, binding, { write_descriptor_set });
}

void SystemVK::setTexture(std::shared_ptr<Texture> value)
{
	setTexture(0, value);
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

void SystemVK::readPixels(const glm::ivec2& pos, const glm::ivec2& size, std::shared_ptr<Renderer::Texture> dst_texture)
{
	//
}

void SystemVK::present()
{
	try
	{
		drawTest();
	}
	catch (const std::exception& e)
	{
		PLATFORM->alert(e.what());
	}

	size_t total_vertex_buffers_size = 0;

	for (const auto& vertex_buffer : mVertexBuffers)
	{
		total_vertex_buffers_size += vertex_buffer.size;
	}

	size_t total_index_buffers_size = 0;

	for (const auto& index_buffer : mIndexBuffers)
	{
		total_index_buffers_size += index_buffer.size;
	}

	std::cout << "vertex buffers: " << mVertexBuffers.size() << ", total size: " << total_vertex_buffers_size << std::endl;
	std::cout << "index buffers: " << mIndexBuffers.size() << ", total size: " << total_index_buffers_size<< std::endl;

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

	mVertexBufferIndex = 0;
	mIndexBufferIndex = 0;

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























static std::string vertex_shader_code = R"(
#version 450 core

layout(location = 0) in vec3 aPosition;
layout(location = 1) in vec2 aTexCoord;
layout(location = 2) in vec4 aColor;

layout(push_constant) uniform constants
{
	mat4 projection;
	mat4 view;
	mat4 model;
} pc;

layout(location = 0) out struct 
{
	vec4 Color;
	vec2 TexCoord;
} Out;

out gl_PerVertex 
{
	vec4 gl_Position;
};

void main()
{
	Out.Color = aColor;
	Out.TexCoord = aTexCoord;
	gl_Position = pc.projection * pc.view * pc.model * vec4(aPosition, 1.0);
}
)";

static std::string fragment_shader_code = R"(
#version 450 core

layout(location = 0) out vec4 result;
layout(set=0, binding=0) uniform sampler2D sTexture;

layout(location = 0) in struct 
{
	vec4 Color;
	vec2 TexCoord;
} In;

void main()
{
	result = In.Color * texture(sTexture, In.TexCoord.st);
}
)";

void SystemVK::drawTest()
{
	struct PushConstants
	{
		glm::mat4 projection = glm::mat4(1.0f);
		glm::mat4 view = glm::mat4(1.0f);
		glm::mat4 model = glm::mat4(1.0f);
	};

	if (!pipeline_created)
	{
		auto push_constant_range = vk::PushConstantRange()
			.setStageFlags(vk::ShaderStageFlagBits::eVertex)
			.setOffset(0)
			.setSize(sizeof(PushConstants));

		auto pipeline_layout_create_info = vk::PipelineLayoutCreateInfo()
			.setSetLayoutCount(1)
			.setPSetLayouts(&*mDescriptorSetLayout)
			.setPushConstantRangeCount(1)
			.setPPushConstantRanges(&push_constant_range);

		mPipelineLayout = mDevice.createPipelineLayout(pipeline_layout_create_info);
		
		auto vertex_shader_spirv = skygfx::CompileGlslToSpirv(skygfx::ShaderStage::Vertex, vertex_shader_code);
		auto fragment_shader_spirv = skygfx::CompileGlslToSpirv(skygfx::ShaderStage::Fragment, fragment_shader_code);

		auto vertex_shader_module_create_info = vk::ShaderModuleCreateInfo()
			.setCode(vertex_shader_spirv);

		auto fragment_shader_module_create_info = vk::ShaderModuleCreateInfo()
			.setCode(fragment_shader_spirv);

		auto vertex_shader_module = mDevice.createShaderModule(vertex_shader_module_create_info);
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
			vk::DynamicState::eFrontFace,
			vk::DynamicState::eVertexInputBindingStride,
			vk::DynamicState::eVertexInputEXT
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

		pipeline_created = true;
	}

	struct Vertex
	{
		glm::vec3 pos;
		glm::vec2 uv;
		glm::vec4 col;
	};

	std::vector<Vertex> vertices = {
		{ { 0.0f, -0.5f, 0.0f }, { 0.0f, 0.0f }, { 0.0f, 1.0f, 0.0f, 1.0f } },
		{ { -0.5f, 0.5f, 0.0f }, { 0.0f, 0.0f }, { 1.0f, 0.0f, 0.0f, 1.0f } },
		{ { 0.5f, 0.5f, 0.0f }, { 0.0f, 0.0f }, { 0.0f, 0.0f, 1.0f, 1.0f } },
	};

	std::vector<uint32_t> indices = { 0, 1, 2 };
	
	PushConstants push_constants;
	
	auto vertex_input_binding_description = vk::VertexInputBindingDescription2EXT()
		.setInputRate(vk::VertexInputRate::eVertex)
		.setDivisor(1)
		.setBinding(0);

	std::vector<vk::VertexInputAttributeDescription2EXT> vertex_input_attribute_descriptions = {
		{ 0, 0, vk::Format::eR32G32B32Sfloat, offsetof(Vertex, pos) },
		{ 1, 0, vk::Format::eR32G32Sfloat, offsetof(Vertex, uv) },
		{ 2, 0, vk::Format::eR32G32B32A32Sfloat, offsetof(Vertex, col) },
	};

	mCommandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, *mPipeline);
	mCommandBuffer.setVertexInputEXT({ vertex_input_binding_description }, { vertex_input_attribute_descriptions });
	mCommandBuffer.pushConstants<PushConstants>(*mPipelineLayout, vk::ShaderStageFlagBits::eVertex, 0, { push_constants });

	uint32_t white_pixel = 0xFFFFFFFF;
	static auto texture = std::make_shared<Texture>(1, 1, 4, &white_pixel);

	setTexture(texture);

	mCommandBuffer.setFrontFace(vk::FrontFace::eCounterClockwise);
	mCommandBuffer.setLineWidth(1.0f);
	setVertexBuffer(vertices);
	setIndexBuffer(indices);
	setTopology(Renderer::Topology::TriangleList);
	setCullMode(Renderer::CullMode::None);
	setViewport(Renderer::Viewport());
	setScissor(nullptr);
	mCommandBuffer.drawIndexed(3, 1, 0, 0, 0);

	std::vector<Vertex> vertices2 = {
		{ { -0.5f, 0.0f, 0.0f }, { 0.0f, 0.0f }, { 1.0f, 1.0f, 1.0f, 0.5f } },
		{ { 0.5f, -0.5f, 0.0f }, { 0.0f, 0.0f }, { 1.0f, 1.0f, 1.0f, 0.5f } },
		{ { 0.5f, 0.5f, 0.0f }, { 0.0f, 0.0f }, { 1.0f, 1.0f, 1.0f, 0.5f } },
	};

	uint32_t red_pixel = 0xFF0000FF;
	static auto red_texture = std::make_shared<Texture>(1, 1, 4, &red_pixel);

	setTexture(red_texture);
	setVertexBuffer(vertices2);

	mCommandBuffer.drawIndexed(3, 1, 0, 0, 0);
}

#endif