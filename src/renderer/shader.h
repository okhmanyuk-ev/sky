#pragma once

#include <renderer/vertex.h>
#include <set>
#include <vector>
#include <string>
#include <cassert>
#include <unordered_map>
#include <memory>

namespace Renderer
{
	class Shader
	{
		friend class System;
	
	public:
		Shader(const Vertex::Layout& layout, const std::string& vertex_code,
			const std::string& fragment_code, const std::vector<std::string>& defines = {});
		virtual ~Shader();

	protected:
		void apply();
		virtual void update() {};

	private:
		struct Impl;
		std::unique_ptr<Impl> mImpl;

	protected:
		static void checkRequiredAttribs(const std::set<Vertex::Attribute::Type>& requiredAttribs, const Vertex::Layout& layout)
		{
			for (auto& attrib : requiredAttribs)
			{
				assert(layout.hasAttribute(attrib));
			}
		}
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