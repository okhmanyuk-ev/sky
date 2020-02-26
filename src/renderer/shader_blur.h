#pragma once

#include <Renderer/system.h>
#include <Renderer/shader.h>
#include <Renderer/vertex.h>
#include <Renderer/low_level_api.h>
#include <map>

namespace Renderer
{
	class ShaderBlur : public Shader, public ShaderMatrices
	{
	public:
		enum class Direction
		{
			Vertical,
			Horizontal
		};

	private:
		const std::set<Vertex::Attribute::Type> requiredAttribs = {
			Vertex::Attribute::Type::Position,
		};

	private:
		struct alignas(16) ConstantBuffer
		{
			glm::mat4 view = glm::mat4(1.0f);
			glm::mat4 projection = glm::mat4(1.0f);
			glm::mat4 model = glm::mat4(1.0f);

			glm::vec2 direction;
			glm::vec2 resolution;
		};

	public:
		ShaderBlur(const Vertex::Layout& laayout);
		~ShaderBlur();

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

	public:
		void setDirection(Direction value) { mConstantBuffer.direction = (value == Direction::Horizontal ? glm::vec2(1.0f, 0.0f) : glm::vec2(0.0f, 1.0f)); mConstantBufferDirty = true; }
		void setResolution(const glm::vec2& value) { mConstantBuffer.resolution = value; mConstantBufferDirty = true; }

	private:
		ConstantBuffer mConstantBuffer;
		bool mConstantBufferDirty = false;

	private:
		struct Impl;
		std::unique_ptr<Impl> mImpl;
	};
}