#pragma once

#include <renderer/system.h>
#include <renderer/shader.h>
#include <renderer/vertex.h>
#include <renderer/low_level_api.h>
#include <map>

namespace Renderer
{
	class ShaderCross : public Shader
	{
	public:
		ShaderCross(const Vertex::Layout& layout, const std::string& vertex_code,
			const std::string& fragment_code);
		virtual ~ShaderCross();

	protected:
		void apply() override;
		void update() override {}

	private:
		struct Impl;
		std::unique_ptr<Impl> mImpl;

	public:
		static void AddLocationDefines(const Vertex::Layout& layout, std::vector<std::string>& defines)
		{
			const std::unordered_map<Vertex::Attribute::Type, std::string> Names = {
				{ Vertex::Attribute::Type::Position, "POSITION_LOCATION" },
				{ Vertex::Attribute::Type::Color, "COLOR_LOCATION" },
				{ Vertex::Attribute::Type::TexCoord, "TEXCOORD_LOCATION" },
				{ Vertex::Attribute::Type::Normal, "NORMAL_LOCATION" },
			};

			for (int i = 0; i < layout.attributes.size(); i++)
			{
				const auto& attrib = layout.attributes.at(i);
				assert(Names.contains(attrib.type));
				auto name = Names.at(attrib.type);
				defines.push_back(name + " " + std::to_string(i));
			}
		}
	};
}