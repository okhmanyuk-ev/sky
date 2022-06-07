#include "system_skygfx.h"

#if defined(RENDERER_SKYGFX)
#include <platform/system_windows.h>

using namespace Renderer;

struct Shader::Impl
{
	std::shared_ptr<skygfx::Shader> shader;
};

Shader::Shader(const Vertex::Layout& layout, const std::string& vertex_code, const std::string& fragment_code)
{
	mImpl = std::make_unique<Impl>();
	mImpl->shader = std::make_shared<skygfx::Shader>(*(skygfx::Vertex::Layout*)&layout, vertex_code, fragment_code);
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

Texture::Texture(int width, int height, bool mipmap) :
	mWidth(width),
	mHeight(height),
	mMipmap(mipmap)
{
	mTextureImpl = std::make_unique<TextureImpl>();
	//uint32_t pixel = 0xFFFFFFFF;
	//mTextureImpl->texture = std::make_shared<skygfx::Texture>(1, 1, 4, &pixel);
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
	mTextureImpl->texture = std::make_shared<skygfx::Texture>(width, height, channels, data, mMipmap);
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

SystemSkygfx::SystemSkygfx()
{
	auto width = PLATFORM->getWidth();
	auto height = PLATFORM->getHeight();
	mDevice = std::make_shared<skygfx::Device>(skygfx::BackendType::D3D11, Platform::SystemWindows::Window, width, height);
}

SystemSkygfx::~SystemSkygfx()
{
}

void SystemSkygfx::onEvent(const Platform::System::ResizeEvent& e)
{
}

void SystemSkygfx::setTopology(const Renderer::Topology& value)
{
	std::map<Renderer::Topology, skygfx::Topology> Topologies = {
		{ Renderer::Topology::PointList, skygfx::Topology::PointList },
		{ Renderer::Topology::LineList, skygfx::Topology::LineList },
		{ Renderer::Topology::LineStrip, skygfx::Topology::LineStrip },
		{ Renderer::Topology::TriangleList, skygfx::Topology::TriangleList },
		{ Renderer::Topology::TriangleStrip, skygfx::Topology::TriangleStrip },
	};
	mDevice->setTopology(Topologies.at(value));
}

void SystemSkygfx::setViewport(const Viewport& value)
{
	skygfx::Viewport viewport;
	viewport.position = value.position;
	viewport.size = value.size;
	viewport.min_depth = value.minDepth;
	viewport.max_depth = value.maxDepth;
	mDevice->setViewport(viewport);
}

void SystemSkygfx::setScissor(const Scissor& value)
{
}

void SystemSkygfx::setScissor(std::nullptr_t value)
{
}

void SystemSkygfx::setVertexBuffer(const Buffer& value)
{
	skygfx::Buffer buf;
	buf.data = value.data;
	buf.size = value.size;
	buf.stride = value.stride;
	mDevice->setVertexBuffer(buf);
}

void SystemSkygfx::setIndexBuffer(const Buffer& value)
{
	skygfx::Buffer buf;
	buf.data = value.data;
	buf.size = value.size;
	buf.stride = value.stride;
	mDevice->setIndexBuffer(buf);
}

void SystemSkygfx::setUniformBuffer(int slot, void* memory, size_t size)
{
	mDevice->setUniformBuffer(slot, memory, size);
}

void SystemSkygfx::setTexture(int binding, std::shared_ptr<Texture> value)
{
	if (!value)
		return;

	if (!value->mTextureImpl->texture)
		return;

	mDevice->setTexture(*value->mTextureImpl->texture);
}

void SystemSkygfx::setTexture(std::shared_ptr<Texture> value)
{
	setTexture(0, value);
}

void SystemSkygfx::setRenderTarget(std::shared_ptr<RenderTarget> value)
{
}

void SystemSkygfx::setShader(std::shared_ptr<Shader> value)
{
	mDevice->setShader(*value->mImpl->shader);
	value->apply();
}

void SystemSkygfx::setSampler(const Sampler& value)
{
}

void SystemSkygfx::setDepthMode(const DepthMode& value)
{
}

void SystemSkygfx::setStencilMode(const StencilMode& value)
{
}

void SystemSkygfx::setCullMode(const CullMode& value)
{
}

void SystemSkygfx::setBlendMode(const BlendMode& value)
{
}

void SystemSkygfx::setTextureAddressMode(const TextureAddress& value)
{
}

void SystemSkygfx::clear(std::optional<glm::vec4> color, std::optional<float> depth, std::optional<uint8_t> stencil)
{
	mDevice->clear(color, depth, stencil);
}

void SystemSkygfx::draw(size_t vertexCount, size_t vertexOffset)
{
}

void SystemSkygfx::drawIndexed(size_t indexCount, size_t indexOffset, size_t vertexOffset)
{
	mDevice->drawIndexed(indexCount, indexOffset);
}

void SystemSkygfx::readPixels(const glm::ivec2& pos, const glm::ivec2& size, void* memory)
{
}

void SystemSkygfx::readPixels(const glm::ivec2& pos, const glm::ivec2& size, std::shared_ptr<Renderer::Texture> dst_texture)
{
}

void SystemSkygfx::present()
{
	static std::string vertex_shader_code = R"(
	#version 450 core

	layout(location = POSITION_LOCATION) in vec3 aPosition;
	layout(location = COLOR_LOCATION) in vec4 aColor;

	layout(location = 0) out struct { vec4 Color; } Out;
	out gl_PerVertex { vec4 gl_Position; };

	void main()
	{
		Out.Color = aColor;
		gl_Position = vec4(aPosition, 1.0);
	})";

	static std::string fragment_shader_code = R"(
	#version 450 core

	layout(location = 0) out vec4 result;
	layout(location = 0) in struct { vec4 Color; } In;

	void main() 
	{ 
		result = In.Color;
	})";

	using Vertex = skygfx::Vertex::PositionColor;

	static std::vector<Vertex> vertices = {
		{ {  0.5f, -0.5f, 0.0f }, { 0.0f, 0.0f, 1.0f, 1.0f } },
		{ { -0.5f, -0.5f, 0.0f }, { 1.0f, 0.0f, 0.0f, 1.0f } },
		{ {  0.0f,  0.5f, 0.0f }, { 0.0f, 1.0f, 0.0f, 1.0f } },
	};

	static std::vector<uint32_t> indices = { 0, 1, 2 };

	auto viewport = skygfx::Viewport();
	viewport.size = { static_cast<float>(800), static_cast<float>(600) };

	static auto shader = skygfx::Shader(Vertex::Layout, vertex_shader_code, fragment_shader_code);

	mDevice->setTopology(skygfx::Topology::TriangleList);
	mDevice->setViewport(viewport);
	mDevice->setShader(shader);
	mDevice->setVertexBuffer(vertices);
	mDevice->setIndexBuffer(indices);
	mDevice->drawIndexed(static_cast<uint32_t>(indices.size()));




	mDevice->present();
}

#endif