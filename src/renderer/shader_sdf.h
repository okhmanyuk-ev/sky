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
		glm::mat4 getProjectionMatrix() const override { return mConstantBufferData.projection; }
		void setProjectionMatrix(const glm::mat4& value) override { mConstantBufferData.projection = value; mNeedUpdate = true; }

		glm::mat4 getViewMatrix() const override { return mConstantBufferData.view; }
		void setViewMatrix(const glm::mat4& value) override { mConstantBufferData.view = value; mNeedUpdate = true; }

		glm::mat4 getModelMatrix() const override { return mConstantBufferData.model; }
		void setModelMatrix(const glm::mat4& value) override { mConstantBufferData.model = value; mNeedUpdate = true; }
		
		auto getMinValue() const { return mConstantBufferData.minValue; }
		void setMinValue(float value) { mConstantBufferData.minValue = value; mNeedUpdate = true; }

		auto getMaxValue() const { return mConstantBufferData.maxValue; }
		void setMaxValue(float value) { mConstantBufferData.maxValue = value; mNeedUpdate = true; }

		auto getSmoothFactor() const { return mConstantBufferData.smoothFactor; }
		void setSmoothFactor(float value) { mConstantBufferData.smoothFactor = value; mNeedUpdate = true; }

		auto getColor() const { return mConstantBufferData.color;  }
		void setColor(const glm::vec4& value) { mConstantBufferData.color = value; mNeedUpdate = true; }

	private:
		ConstantBuffer mConstantBufferData;
		bool mNeedUpdate = false;

	private:
		struct Impl;
		std::unique_ptr<Impl> mImpl;
	};
}