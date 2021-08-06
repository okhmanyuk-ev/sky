#pragma once

#include <scene3d/node.h>
#include <scene3d/scene.h>

namespace Scene3D
{
	class Model : public Node
	{
	public:
		using Indices = std::vector<uint16_t>;
		using Vertices = std::vector<Scene::Vertex>;

	public:
		void draw() override;

	public:
		auto getTopology() const { return mTopology; }
		void setTopology(Renderer::Topology value) { mTopology = value; }

		const auto& getIndices() const { return mIndices; }
		void setIndices(const Indices& value) { mIndices = value; }

		const auto& getVertices() const { return mVertices; }
		void setVertices(const Vertices& value) { mVertices = value; }

	private:
		Renderer::Topology mTopology = Renderer::Topology::TriangleList;
		Indices mIndices;
		Vertices mVertices;
	};

	class Cube : public Model
	{
	public:
		Cube();
	};
}