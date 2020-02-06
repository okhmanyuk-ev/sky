#pragma once

#include <Renderer/system.h>
#include <Renderer/shader.h>
#include <Renderer/vertex.h>
#include <Renderer/low_level_api.h>
#include <map>

namespace Renderer
{
	class ShaderBlur : public Shader
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
		void setDirection(Direction value) { mConstantBufferData.direction = (value == Direction::Horizontal ? glm::vec2(1.0f, 0.0f) : glm::vec2(0.0f, 1.0f)); mNeedUpdate = true; }
		void setResolution(const glm::vec2& value) { mConstantBufferData.resolution = value; mNeedUpdate = true; }

	private:
		ConstantBuffer mConstantBufferData;
		bool mNeedUpdate = false;

	private:
		struct Impl;
		std::unique_ptr<Impl> mImpl;
	};
}