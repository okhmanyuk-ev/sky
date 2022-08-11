#include "system.h"

#include <platform/system_windows.h>
#include <platform/system_ios.h>

using namespace Renderer;

struct Shader::Impl
{
	std::shared_ptr<skygfx::Shader> shader;
};

Shader::Shader(const Vertex::Layout& layout, const std::string& vertex_code, const std::string& fragment_code,
	const std::vector<std::string>& defines)
{
	mImpl = std::make_unique<Impl>();
	mImpl->shader = std::make_shared<skygfx::Shader>(*(skygfx::Vertex::Layout*)&layout, vertex_code, fragment_code, defines);
}

Shader::~Shader()
{
}

void Shader::apply()
{
	this->update();
}

struct Texture::TextureImpl
{
	std::shared_ptr<skygfx::Texture> texture;
};

Texture::Texture(uint32_t width, uint32_t height, uint32_t channels, void* data, bool mipmap) :
	mWidth(width),
	mHeight(height),
	mMipmap(mipmap)
{
	mTextureImpl = std::make_unique<TextureImpl>();
	mTextureImpl->texture = std::make_shared<skygfx::Texture>(width, height, channels, data, mMipmap);
}

Texture::~Texture()
{
}

struct RenderTarget::RenderTargetImpl
{
	std::shared_ptr<skygfx::RenderTarget> render_target;
};

RenderTarget::RenderTarget(uint32_t width, uint32_t height) : Texture(width, height, 4, nullptr)
{
	mRenderTargetImpl = std::make_unique<RenderTargetImpl>();
	mRenderTargetImpl->render_target = std::make_shared<skygfx::RenderTarget>(width, height);
	mTextureImpl->texture = mRenderTargetImpl->render_target;
}

RenderTarget::~RenderTarget()
{
}

System::System()
{
	auto width = PLATFORM->getWidth();
	auto height = PLATFORM->getHeight();

	auto type = skygfx::BackendType::D3D11;

#if defined(PLATFORM_IOS)
	//type = skygfx::BackendType::Metal;
	type = skygfx::BackendType::OpenGL44;
#endif

	void* window = nullptr;

#if defined(PLATFORM_WINDOWS)
	window = Platform::SystemWindows::Window;
#elif defined(PLATFORM_IOS)
	window = Platform::SystemIos::Window; // TODO: maybe we should send rootView here
#endif

	mDevice = std::make_shared<skygfx::Device>(type, window, width, height);
}

System::~System()
{
}

void System::onEvent(const Platform::System::ResizeEvent& e)
{
	mDevice->resize(e.width, e.height);
}

void System::setTopology(skygfx::Topology value)
{
	mDevice->setTopology(value);
}

void System::setViewport(std::optional<skygfx::Viewport> value)
{
	mDevice->setViewport(value);
}

void System::setScissor(std::optional<skygfx::Scissor> value)
{
	mDevice->setScissor(value);
}

void System::setVertexBuffer(const Buffer& value)
{
	mDevice->setVertexBuffer(*(skygfx::Buffer*)&value);
}

void System::setIndexBuffer(const Buffer& value)
{
	mDevice->setIndexBuffer(*(skygfx::Buffer*)&value);
}

void System::setUniformBuffer(int slot, void* memory, size_t size)
{
	mDevice->setUniformBuffer(slot, memory, size);
}

void System::setTexture(int binding, std::shared_ptr<Texture> value)
{
	if (!value)
		return;

	if (!value->mTextureImpl->texture)
		return;

	mDevice->setTexture((uint32_t)binding, *value->mTextureImpl->texture);
}

void System::setTexture(std::shared_ptr<Texture> value)
{
	setTexture(0, value);
}

void System::setRenderTarget(std::shared_ptr<RenderTarget> value)
{
	if (value == nullptr)
		mDevice->setRenderTarget(nullptr);
	else
		mDevice->setRenderTarget(*value->mRenderTargetImpl->render_target);
}

void System::setShader(std::shared_ptr<Shader> value)
{
	mDevice->setShader(*value->mImpl->shader);
	value->apply();
}

void System::setSampler(const Sampler& value)
{
	mDevice->setSampler(*(skygfx::Sampler*)&value);
}

void System::setDepthMode(const DepthMode& value)
{
	if (!value.enabled)
	{
		mDevice->setDepthMode(std::nullopt);
	}
	else
	{
		auto depth_mode = skygfx::DepthMode();
		depth_mode.func = (skygfx::ComparisonFunc)value.func;
		mDevice->setDepthMode(depth_mode);
	}
}

void System::setStencilMode(const StencilMode& value)
{
	if (!value.enabled)
	{
		mDevice->setStencilMode(std::nullopt);
	}
	else
	{
		auto stencil_mode = skygfx::StencilMode();
		stencil_mode.read_mask = value.readMask;
		stencil_mode.write_mask = value.writeMask;
		stencil_mode.depth_fail_op = (skygfx::StencilOp)value.depthFailOp;
		stencil_mode.fail_op = (skygfx::StencilOp)value.failOp;
		stencil_mode.func = (skygfx::ComparisonFunc)value.func;
		stencil_mode.pass_op = (skygfx::StencilOp)value.passOp;
		stencil_mode.reference = value.reference;
		mDevice->setStencilMode(stencil_mode);
	}
}

void System::setCullMode(const CullMode& value)
{
	mDevice->setCullMode(*(skygfx::CullMode*)&value);
}

void System::setBlendMode(const BlendMode& value)
{
	mDevice->setBlendMode(*(skygfx::BlendMode*)&value);
}

void System::setTextureAddressMode(const TextureAddress& value)
{
	mDevice->setTextureAddress(*(skygfx::TextureAddress*)&value);
}

void System::clear(std::optional<glm::vec4> color, std::optional<float> depth, std::optional<uint8_t> stencil)
{
	mDevice->clear(color, depth, stencil);
}

void System::draw(size_t vertexCount, size_t vertexOffset)
{
	mDrawcalls += 1;
	mDevice->draw(vertexCount, vertexOffset);
}

void System::drawIndexed(size_t indexCount, size_t indexOffset, size_t vertexOffset)
{
	mDrawcalls += 1;
	mDevice->drawIndexed(indexCount, indexOffset);
}

void System::readPixels(const glm::ivec2& pos, const glm::ivec2& size, std::shared_ptr<Renderer::Texture> dst_texture)
{
	mDevice->readPixels(pos, size, *dst_texture->mTextureImpl->texture);
}

void System::present()
{
	mDrawcallsPublic = mDrawcalls;
	mDrawcalls = 0;
	mDevice->present();
}