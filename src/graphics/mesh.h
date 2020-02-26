#pragma once

#include <vector>
#include <memory>
#include <optional>
#include <Renderer/vertex.h>
#include <Renderer/topology.h>
#include <Renderer/texture.h>

namespace Graphics
{
	struct Mesh
	{
		Renderer::Topology topology;
		std::vector<Renderer::Vertex::PositionColorTexture> vertices;
		std::optional<std::vector<uint32_t>> indices;
		std::shared_ptr<Renderer::Texture> texture = nullptr;
	};
}