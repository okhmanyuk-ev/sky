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
#endif
