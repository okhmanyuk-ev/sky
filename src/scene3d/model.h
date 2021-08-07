#pragma once

#include <scene3d/node.h>
#include <scene3d/scene.h>
#include <renderer/all.h>

namespace Scene3D
{
	class Model : public Node
	{
	public:
		using Indices = std::vector<uint32_t>;
		
		using PositionAttribs = std::vector<glm::vec3>;
		using ColorAttribs = std::vector<glm::vec4>;
		using NormalAttribs = std::vector<glm::vec3>;
		using TexCoordAttribs = std::vector<glm::vec2>;

	public:
		void draw(Driver& driver) override;

	private:
		using Vertex = Renderer::Vertex::PositionColorTextureNormal;

	private:
		std::tuple<std::vector<Vertex>, std::set<Renderer::Shaders::Light::Flag>> generateVertices();

	public:
		void setVertices(std::vector<Renderer::Vertex::PositionColorNormal> vertices);
		void setVertices(std::vector<Renderer::Vertex::PositionTextureNormal> vertices);

	public:
		auto getTopology() const { return mTopology; }
		void setTopology(Renderer::Topology value) { mTopology = value; }

		const auto& getIndices() const { return mIndices; }
		void setIndices(const Indices& value) { mIndices = value; }

		const auto& getMaterial() const { return mMaterial; }
		void setMaterial(const Renderer::Shaders::Light::Material& value) { mMaterial = value; }

		const auto& getPositionAttribs() const { return mPositionAttribs; }
		void setPositionAttribs(const PositionAttribs& value);

		const auto& getColorAttribs() const { return mColorAttribs; }
		void setColorAttribs(const ColorAttribs& value);

		const auto& getNormalAttribs() const { return mNormalAttribs; }
		void setNormalAttribs(const NormalAttribs& value);

		const auto& getTexCoordAttribs() const { return mTexCoordAttribs; }
		void setTexCoordAttribs(const TexCoordAttribs& value);

		auto getTexture() const { return mTexture; }
		void setTexture(std::shared_ptr<Renderer::Texture> value) { mTexture = value; }

	private:
		Renderer::Topology mTopology = Renderer::Topology::TriangleList;
		Indices mIndices;
		std::optional<std::vector<Vertex>> mVertices;
		std::set<Renderer::Shaders::Light::Flag> mShaderFlags;
		PositionAttribs mPositionAttribs;
		ColorAttribs mColorAttribs;
		NormalAttribs mNormalAttribs;
		TexCoordAttribs mTexCoordAttribs;
		Renderer::Shaders::Light::Material mMaterial;
		std::shared_ptr<Renderer::Texture> mTexture = nullptr;
	};
}