#pragma once

#include <renderer/vertex.h>
#include <set>
#include <vector>
#include <string>
#include <cassert>
#include <unordered_map>
#include <memory>

namespace Renderer
{
	class Shader
	{
		friend class SystemD3D11;
		friend class SystemGL;
		friend class SystemVK;
		friend class SystemSkygfx;
	
	public:
		Shader(const Vertex::Layout& layout, const std::string& vertex_code,
			const std::string& fragment_code);
		virtual ~Shader();

	protected:
		void apply();
		virtual void update() {};

	private:
		struct Impl;
		std::unique_ptr<Impl> mImpl;

	protected:
		static void checkRequiredAttribs(const std::set<Vertex::Attribute::Type>& requiredAttribs, const Vertex::Layout& layout)
		{
			for (auto& attrib : requiredAttribs)
			{
				assert(layout.hasAttribute(attrib));
			}
		}

	private:
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

	class ShaderMatrices
	{
	public:
		virtual glm::mat4 getProjectionMatrix() const = 0;
		virtual void setProjectionMatrix(const glm::mat4& value) = 0;
		
		virtual glm::mat4 getViewMatrix() const = 0;
		virtual void setViewMatrix(const glm::mat4& value) = 0;
		
		virtual glm::mat4 getModelMatrix() const = 0;
		virtual void setModelMatrix(const glm::mat4& value) = 0;
	};
}