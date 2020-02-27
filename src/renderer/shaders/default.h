#pragma once

#include <Renderer/shader_custom.h>

namespace Renderer
{
	class ShaderDefault : public ShaderCustom
	{
	public:
		enum class Flag
		{
			Textured,
			Colored
		};

	private:
		static std::set<Flag> MakeFlagsFromLayout(const Vertex::Layout& layout);
		static std::string MakeDefinesFromFlags(const Vertex::Layout& layout, const std::string& source, const std::set<Flag>& flags);

	public:
		ShaderDefault(const Vertex::Layout& layout, const std::set<Flag>& flags);
		ShaderDefault(const Vertex::Layout& layout);
		~ShaderDefault();
	};
}