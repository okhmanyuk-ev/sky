#pragma once

#include <cstdint>
#include <Renderer/vertex.h>
#include <Renderer/topology.h>

namespace Graphics
{
	template <class Vertex> struct Mesh
	{
		Renderer::Topology topology;
		std::vector<Vertex> vertices;
		std::vector<uint32_t> indices;
	};
}