#pragma once

#include <renderer/shader_custom.h>
#include <optional>

namespace Renderer::Shaders
{
	class Default : public ShaderCustom // TODO: remove this class and rewrite every shader to Generic
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

	class Generic : public ShaderCross, public ShaderMatrices
	{
	private:
		struct alignas(16) Matrices
		{
			glm::mat4 projection = glm::mat4(1.0f);
			glm::mat4 view = glm::mat4(1.0f);
			glm::mat4 model = glm::mat4(1.0f);
			float lod_bias = 0.0f;
		};

	public:
		enum class Flag
		{
			Textured,
			Colored
		};

	public:
		Generic(const Vertex::Layout& layout, const std::set<Flag>& flags,
			std::optional<std::string> additional_ubo = std::nullopt,
			std::optional<std::string> additional_fragment_func = std::nullopt);
		Generic(const Vertex::Layout& layout,
			std::optional<std::string> additional_ubo = std::nullopt,
			std::optional<std::string> additional_fragment_func = std::nullopt);

		glm::mat4 getProjectionMatrix() const override { return mConstantBuffer.projection; }
		void setProjectionMatrix(const glm::mat4& value) override { mConstantBuffer.projection = value; }

		glm::mat4 getViewMatrix() const override { return mConstantBuffer.view; }
		void setViewMatrix(const glm::mat4& value) override { mConstantBuffer.view = value; }

		glm::mat4 getModelMatrix() const override { return mConstantBuffer.model; }
		void setModelMatrix(const glm::mat4& value) override { mConstantBuffer.model = value; }

		auto getLodBias() const { return mConstantBuffer.lod_bias; }
		void setLodBias(float value) { mConstantBuffer.lod_bias = value; }

	protected:
		void update() override;

	protected:
		Matrices mConstantBuffer;

	private:
		static std::string GenerateVertexCode(const Vertex::Layout& layout, const std::set<Flag>& flags);
		
		static std::string GenerateFragmentCode(const Vertex::Layout& layout, const std::set<Flag>& flags,
			std::optional<std::string> additional_ubo = std::nullopt, 
			std::optional<std::string> additional_fragment_func = std::nullopt);

		static std::string MakeShaderHeader(const Vertex::Layout& layout, const std::set<Flag>& flags);

		static std::set<Flag> MakeFlagsFromLayout(const Vertex::Layout& layout);
	};
}