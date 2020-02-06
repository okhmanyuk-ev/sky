#pragma once

#include <Renderer/vertex.h>
#include <set>
#include <assert.h>

namespace Renderer
{
	class Shader
	{
		friend class SystemD3D11;
		friend class SystemGL;

	protected:
		virtual void apply() = 0;
		virtual void update() = 0;

	protected:
		static void checkRequiredAttribs(const std::set<Vertex::Attribute::Type>& requiredAttribs, const Vertex::Layout& layout);
	};

	class ShaderMatrices
	{
	public:
		virtual glm::mat4 getProjectionMatrix() const = 0;
		virtual void setProjectionMatrix(const glm::mat4& value) = 0;
		
		virtual glm::mat4 getViewMatrix() const = 0;
		virtual void setViewMatrix(const glm::mat4& value) = 0;
		
		virtual glm::mat4 getModelMatrix() const = 0;
		virtual void setModelMatrix(const glm::mat4& value) = 0;
	};
}