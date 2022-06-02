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

	std::vector<uint32_t> CompileGlslToSpirv(ShaderStage stage, const std::string& code, const std::vector<std::string>& defines = {});
	std::string CompileSpirvToHlsl(const std::vector<uint32_t>& spirv);
	std::string CompileSpirvToGlsl(const std::vector<uint32_t>& spirv);
}