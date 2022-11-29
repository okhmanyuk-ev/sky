#pragma once

#include <set>
#include <vector>
#include <string>
#include <cassert>
#include <unordered_map>
#include <memory>
#include <skygfx/skygfx.h>

namespace Renderer
{
	class Shader : public skygfx::Shader
	{
	public:
		using skygfx::Shader::Shader;

	public:
		virtual void update() {};
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
