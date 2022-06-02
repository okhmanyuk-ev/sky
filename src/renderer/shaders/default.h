#pragma once

#include <renderer/shader_custom.h>
#include <optional>

namespace Renderer::Shaders
{
	class Default : public ShaderCustom
	{
	public:
		enum class Flag
		{
			Textured,
			Colored
		};

	public:
		struct CustomCode
		{
			std::string constant_buffer_fields;
			std::string fragment_func;
		};

	private:
		static std::set<Flag> MakeFlagsFromLayout(const Vertex::Layout& layout);
		static std::string MakeDefinesFromFlags(const Vertex::Layout& layout, const std::string& source, 
			const std::set<Flag>& flags, std::optional<CustomCode> custom_code);

	public:
		Default(const Vertex::Layout& layout, const std::set<Flag>& flags, size_t customConstantBufferSize = 0, std::optional<CustomCode> custom_code = std::nullopt);
		Default(const Vertex::Layout& layout, size_t customConstantBufferSize = 0, std::optional<CustomCode> custom_code = std::nullopt);
	};

	class Generic : public ShaderCrossWithMatrices
	{
	public:
		enum class Flag
		{
			Textured,
			Colored
		};

	public:
		Generic(const Vertex::Layout& layout, const std::set<Flag>& flags);
		Generic(const Vertex::Layout& layout);

		static std::string FinalizeShaderCode(const Vertex::Layout& layout, const std::string& source,
			const std::set<Flag>& flags);

		static std::set<Flag> MakeFlagsFromLayout(const Vertex::Layout& layout);
	};
}