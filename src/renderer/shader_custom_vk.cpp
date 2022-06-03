#include <renderer/shader_custom.h>

#if defined(RENDERER_VK)

using namespace Renderer;

struct ShaderCustom::Impl
{

};

ShaderCustom::ShaderCustom(const Vertex::Layout& layout, const std::set<Vertex::Attribute::Type>& requiredAttribs, 
	size_t customConstantBufferSize, const std::string& source)
{
	//
}

ShaderCustom::~ShaderCustom()
{
	//
}

void ShaderCustom::apply()
{
	//
}

void ShaderCustom::update()
{
	//
}

struct ShaderCross::Impl
{

};

ShaderCross::ShaderCross(const Vertex::Layout& layout, const std::string& vertex_code,
	const std::string& fragment_code)
{
	//
}

ShaderCross::~ShaderCross()
{
	//
}

void ShaderCross::apply()
{
	//
}

void ShaderCross::update()
{
	//
}

void ShaderCross::pushConstants(int slot, void* memory, size_t size)
{
	//
}

#endif
