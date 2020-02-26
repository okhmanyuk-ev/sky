#pragma once

#include <Renderer/system.h>
#include <Renderer/shader.h>
#include <Renderer/vertex.h>
#include <Renderer/low_level_api.h>
#include <map>

namespace Renderer
{
	class ShaderSDF : public Shader, public ShaderMatrices
	{
	private:
		const std::set<Vertex::Attribute::Type> RequiredAttribs = {
			Vertex::Attribute::Type::Position,
			Vertex::Attribute::Type::TexCoord,
		};

	private:
		struct alignas(16) ConstantBuffer
		{
			glm::mat4 view = glm::mat4(1.0f);
			glm::mat4 projection = glm::mat4(1.0f);
			glm::mat4 model = glm::mat4(1.0f);
			glm::vec4 color = { 1.0f, 1.0f, 1.0f, 1.0f };
			float minValue = 0.0f;
			float maxValue = 0.0f;
			float smoothFactor = 0.0f;
		};

	public:
		ShaderSDF(const Vertex::Layout& laayout);
		~ShaderSDF();

	protected:
		void apply() override;
		void update() override;

	public:
		glm::mat4 getProjectionMatrix() const override { return mConstantBuffer.projection; }
		void setProjectionMatrix(const glm::mat4& value) override { mConstantBuffer.projection = value; mConstantBufferDirty = true; }

		glm::mat4 getViewMatrix() const override { return mConstantBuffer.view; }
		void setViewMatrix(const glm::mat4& value) override { mConstantBuffer.view = value; mConstantBufferDirty = true; }

		glm::mat4 getModelMatrix() const override { return mConstantBuffer.model; }
		void setModelMatrix(const glm::mat4& value) override { mConstantBuffer.model = value; mConstantBufferDirty = true; }
		
		auto getMinValue() const { return mConstantBuffer.minValue; }
		void setMinValue(float value) { mConstantBuffer.minValue = value; mConstantBufferDirty = true; }

		auto getMaxValue() const { return mConstantBuffer.maxValue; }
		void setMaxValue(float value) { mConstantBuffer.maxValue = value; mConstantBufferDirty = true; }

		auto getSmoothFactor() const { return mConstantBuffer.smoothFactor; }
		void setSmoothFactor(float value) { mConstantBuffer.smoothFactor = value; mConstantBufferDirty = true; }

		auto getColor() const { return mConstantBuffer.color;  }
		void setColor(const glm::vec4& value) { mConstantBuffer.color = value; mConstantBufferDirty = true; }

	private:
		ConstantBuffer mConstantBuffer;
		bool mConstantBufferDirty = false;

	private:
		struct Impl;
		std::unique_ptr<Impl> mImpl;
	};
}