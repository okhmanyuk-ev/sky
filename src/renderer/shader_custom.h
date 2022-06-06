#pragma once

#include <renderer/system.h>
#include <renderer/shader.h>
#include <renderer/vertex.h>
#include <renderer/low_level_api.h>
#include <map>

namespace Renderer
{
	class ShaderCustom : public Shader, public ShaderMatrices
	{
	private:
		struct alignas(16) ConstantBuffer
		{
			glm::mat4 view = glm::mat4(1.0f);
			glm::mat4 projection = glm::mat4(1.0f);
			glm::mat4 model = glm::mat4(1.0f);
		};

	public:
		ShaderCustom(const Vertex::Layout& layout, const std::set<Vertex::Attribute::Type>& requiredAttribs,
			size_t customConstantBufferSize, const std::string& source);
		virtual ~ShaderCustom();

	protected:
		void apply() override;
		void update() override;

	public:
		void setCustomConstantBuffer(void* value) { mCustomConstantBuffer = value; }

	public:
		glm::mat4 getProjectionMatrix() const override { return mConstantBuffer.projection; }
		void setProjectionMatrix(const glm::mat4& value) override { mConstantBuffer.projection = value; }

		glm::mat4 getViewMatrix() const override { return mConstantBuffer.view; }
		void setViewMatrix(const glm::mat4& value) override { mConstantBuffer.view = value; }

		glm::mat4 getModelMatrix() const override { return mConstantBuffer.model; }
		void setModelMatrix(const glm::mat4& value) override { mConstantBuffer.model = value; }

	private:
		ConstantBuffer mConstantBuffer;
		void* mCustomConstantBuffer = nullptr;

	private:
		struct Impl;
		std::unique_ptr<Impl> mImpl;
	};

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