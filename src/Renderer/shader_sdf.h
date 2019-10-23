#pragma once

#include <Renderer/system.h>
#include <Renderer/shader.h>
#include <Renderer/vertex.h>
#include <Renderer/low_level_api.h>
#include <map>

namespace Renderer
{
	class ShaderSDF : public Shader
	{
	private:
		const std::set<Vertex::Attribute::Type> requiredAttribs = {
			Vertex::Attribute::Type::Position,
			Vertex::Attribute::Type::Color,
			Vertex::Attribute::Type::TexCoord,
		};

	private:
		struct alignas(16) ConstantBuffer
		{
			glm::mat4 view = glm::mat4(1.0f);
			glm::mat4 projection = glm::mat4(1.0f);
			glm::mat4 model = glm::mat4(1.0f);
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
		void setProjectionMatrix(const glm::mat4& value) { mConstantBufferData.projection = value; mNeedUpdate = true; }
		void setViewMatrix(const glm::mat4& value) { mConstantBufferData.view = value; mNeedUpdate = true; }
		void setModelMatrix(const glm::mat4& value) { mConstantBufferData.model = value; mNeedUpdate = true; }
		void setMinValue(float value) { mConstantBufferData.minValue = value; mNeedUpdate = true; }
		void setMaxValue(float value) { mConstantBufferData.maxValue = value; mNeedUpdate = true; }
		void setSmoothFactor(float value) { mConstantBufferData.smoothFactor = value; mNeedUpdate = true; }

	private:
		ConstantBuffer mConstantBufferData;
		bool mNeedUpdate = false;

	private:
		struct Impl;
		std::unique_ptr<Impl> mImpl;
	};
}