#pragma once

#include <vector>
#include <string>
#include <renderer/vertex.h>

namespace Renderer
{
	enum class ShaderStage
	{
		Vertex,
		Fragment
	};

	std::vector<uint32_t> CompileGlslToSpirv(ShaderStage stage, const std::string& code);
	std::string CompileSpirvToHlsl(const std::vector<uint32_t>& spirv);
}