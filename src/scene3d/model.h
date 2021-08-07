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
		
		using PositionAttrib = glm::vec3;
		using ColorAttrib = glm::vec4;
		using NormalAttrib = glm::vec3;
		using TexCoordAttrib = glm::vec2;

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
		void setPositionAttribs(const std::vector<PositionAttrib>& value);

		const auto& getColorAttribs() const { return mColorAttribs; }
		void setColorAttribs(const std::vector<ColorAttrib>& value);

		const auto& getNormalAttribs() const { return mNormalAttribs; }
		void setNormalAttribs(const std::vector<NormalAttrib>& value);

		const auto& getTexCoordAttribs() const { return mTexCoordAttribs; }
		void setTexCoordAttribs(const std::vector<TexCoordAttrib>& value);

		auto getTexture() const { return mTexture; }
		void setTexture(std::shared_ptr<Renderer::Texture> value) { mTexture = value; }

	private:
		Renderer::Topology mTopology = Renderer::Topology::TriangleList;
		Indices mIndices;
		std::optional<std::vector<Vertex>> mVertices;
		std::set<Renderer::Shaders::Light::Flag> mShaderFlags;
		std::vector<PositionAttrib> mPositionAttribs;
		std::vector<ColorAttrib> mColorAttribs;
		std::vector<NormalAttrib> mNormalAttribs;
		std::vector<TexCoordAttrib> mTexCoordAttribs;
		Renderer::Shaders::Light::Material mMaterial;
		std::shared_ptr<Renderer::Texture> mTexture = nullptr;
	};
}