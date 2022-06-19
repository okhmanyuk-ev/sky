#pragma once

#include <renderer/shader.h>
#include <optional>

namespace Renderer::Shaders
{
	class Generic : public Shader, public ShaderMatrices
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
			std::optional<std::string> additional_fragment_func = std::nullopt,
			const std::vector<std::string>& defines = {});
		Generic(const Vertex::Layout& layout, 
			std::optional<std::string> additional_fragment_func = std::nullopt,
			const std::vector<std::string>& defines = {});

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
			std::optional<std::string> additional_fragment_func = std::nullopt);

		static std::vector<std::string> GenerateDefines(const std::vector<std::string>& defines, 
			const Vertex::Layout& layout, const std::set<Flag>& flags, bool custom_fragment_func);
		static std::set<Flag> MakeFlagsFromLayout(const Vertex::Layout& layout);
	};
}