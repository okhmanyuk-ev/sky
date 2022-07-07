#pragma once

#include <renderer/topology.h>
#include <renderer/vertex.h>
#include <renderer/system.h>
#include <renderer/shaders/generic.h>
#include <platform/system.h>
#include <glm/glm.hpp>

namespace Graphics
{
	class MeshBuilder
	{
	public:
		void begin();
		void vertex(const glm::vec2& value);
		void vertex(const glm::vec3& value);
		void color(const glm::vec3& value);
		void color(const glm::vec4& value);
		std::tuple<const std::vector<Renderer::Vertex::PositionColor>&, size_t> end();

	private:
		bool mWorking = false;
		size_t mVertexCount = 0;
		std::vector<Renderer::Vertex::PositionColor> mVertices;
		glm::vec4 mColor = { 0.0f, 0.0f, 0.0f, 0.0f };
	};
}