#include "shader.h"

using namespace Renderer;

Shader::Shader(const skygfx::Vertex::Layout& layout, const std::string& vertex_code, const std::string& fragment_code,
	const std::vector<std::string>& defines)
{
	mShader = std::make_shared<skygfx::Shader>(layout, vertex_code, fragment_code, defines);
}

Shader::~Shader()
{
}
