#pragma once

#include <scene3d/node.h>
#include <scene3d/scene.h>

namespace Scene3D
{
	class Model : public Node
	{
	public:
		using Indices = std::vector<uint32_t>;
		using Vertices = std::vector<Scene::Vertex>;

	public:
		void draw(Driver& driver) override;

	public:
		auto getTopology() const { return mTopology; }
		void setTopology(Renderer::Topology value) { mTopology = value; }

		const auto& getIndices() const { return mIndices; }
		void setIndices(const Indices& value) { mIndices = value; }

		const auto& getVertices() const { return mVertices; }
		void setVertices(const Vertices& value) { mVertices = value; }

		const auto& getMaterial() const { return mMaterial; }
		void setMaterial(const Renderer::Shaders::Light::Material& value) { mMaterial = value; }

	private:
		Renderer::Topology mTopology = Renderer::Topology::TriangleList;
		Indices mIndices;
		Vertices mVertices;
		Renderer::Shaders::Light::Material mMaterial;
	};

	class Cube : public Model
	{
	public:
		Cube();
	};
}