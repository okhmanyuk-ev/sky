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
	};
}