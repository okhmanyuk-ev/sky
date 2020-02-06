#pragma once

#include <Renderer/system.h>
#include <Renderer/shader.h>
#include <Renderer/vertex.h>
#include <Renderer/low_level_api.h>
#include <map>

namespace Renderer
{
	class ShaderDefault : public Shader, public ShaderMatrices
	{
	public:
		enum class Flag
		{
			Textured,
			Colored
		};

	private:
		static inline std::set<Flag> MakeFlagsFromLayout(const Vertex::Layout& layout);
	
	private:
		const std::set< Vertex::Attribute::Type> RequiredAttribs = {
			Vertex::Attribute::Type::Position
		};

	private:
		struct alignas(16) ConstantBuffer
		{
			glm::mat4 view = glm::mat4(1.0f);
			glm::mat4 projection = glm::mat4(1.0f);
			glm::mat4 model = glm::mat4(1.0f);
		};

	public:
		ShaderDefault(const Vertex::Layout& layout, const std::set<Flag>& flags);
		ShaderDefault(const Vertex::Layout& layout) : ShaderDefault(layout, MakeFlagsFromLayout(layout)) { };
		~ShaderDefault();

	protected:
		void apply() override;
		void update() override;

	public:
		glm::mat4 getProjectionMatrix() const override { return mConstantBufferData.projection; }
		void setProjectionMatrix(const glm::mat4& value) override { mConstantBufferData.projection = value; mNeedUpdate = true; }

		glm::mat4 getViewMatrix() const override { return mConstantBufferData.view; }
		void setViewMatrix(const glm::mat4& value) override { mConstantBufferData.view = value; mNeedUpdate = true; }

		glm::mat4 getModelMatrix() const override { return mConstantBufferData.model; }
		void setModelMatrix(const glm::mat4& value) override { mConstantBufferData.model = value; mNeedUpdate = true; }

	private:
		ConstantBuffer mConstantBufferData;
		bool mNeedUpdate = false;
		
	private:
		struct Impl;
        std::unique_ptr<Impl> mImpl;
	};

	std::set<ShaderDefault::Flag> ShaderDefault::MakeFlagsFromLayout(const Vertex::Layout& layout)
	{
		std::set<Flag> result = { };

		if (layout.hasAttribute(Vertex::Attribute::Type::Color))
			result.insert(Flag::Colored);

		if (layout.hasAttribute(Vertex::Attribute::Type::TexCoord))
			result.insert(Flag::Textured);

		return result;
	}
}