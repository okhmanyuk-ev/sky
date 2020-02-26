#pragma once

#include <vector>
#include <Renderer/vertex.h>
#include <Renderer/topology.h>
#include <Renderer/texture.h>
#include <memory>

namespace Graphics
{
	struct Mesh
	{
		Renderer::Topology topology;
		std::vector<Renderer::Vertex::PositionColorTexture> vertices;
		std::vector<uint32_t> indices;
		std::shared_ptr<Renderer::Texture> texture;
	};
}