#pragma once

#include <renderer/topology.h>
#include <renderer/vertex.h>
#include <renderer/system.h>
#include <renderer/shaders/default.h>
#include <platform/system.h>
#include <glm/glm.hpp>

namespace Graphics
{
	class LegacyRenderer
	{
	public:
		LegacyRenderer();
		~LegacyRenderer();
	
	public:
		void begin(Renderer::Topology topology);
		void vertex(const glm::vec2& value);
		void vertex(const glm::vec3& value);
		void color(const glm::vec3& value);
		void color(const glm::vec4& value);
		void end();

		void setProjectionMatrix(const glm::mat4& value) { mProjectionMatrix = value; }
		void setViewMatrix(const glm::mat4& value) { mViewMatrix = value; }
		void setModelMatrix(const glm::mat4& value) { mModelMatrix = value; }

	private:		
		std::shared_ptr<Renderer::Shaders::Default> mShader = std::make_shared<Renderer::Shaders::Default>(Renderer::Vertex::PositionColor::Layout);
		
	private:
		bool mWorking = false;
		Renderer::Topology mTopology;
		glm::vec4 mColor = { 0.0f, 0.0f, 0.0f, 0.0f };
		std::vector<Renderer::Vertex::PositionColor> mVertices;
		size_t mVertexCount = 0;

		glm::mat4 mProjectionMatrix = glm::mat4(1.0f);
		glm::mat4 mViewMatrix = glm::mat4(1.0f);
		glm::mat4 mModelMatrix = glm::mat4(1.0f);
	};
}