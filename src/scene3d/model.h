#pragma once

#include <scene3d/node.h>
#include <scene3d/scene.h>
#include <renderer/all.h>
#include <variant>

namespace Scene3D
{
	/*class Model : public Node
	{
	public:
		using Indices = std::vector<uint32_t>;
		
		using PositionAttribs = std::vector<glm::vec3>;
		using ColorAttribs = std::vector<glm::vec4>;
		using NormalAttribs = std::vector<glm::vec3>;
		using TexCoordAttribs = std::vector<glm::vec2>;

	public:
		void draw(Renderer::Technique& technique) override;

	private:
		using Vertex = skygfx::vertex::PositionColorTextureNormal;

	private:
		std::tuple<std::vector<Vertex>, std::set<Renderer::Shaders::Light::Flag>> generateVertices();

	public:
		void setVertices(const std::vector<skygfx::vertex::PositionColorNormal>& vertices);
		void setVertices(const std::vector<skygfx::vertex::PositionTextureNormal>& vertices);
		void setVertices(const std::vector<skygfx::vertex::PositionColorTextureNormal>& vertices);

	public:
		auto getTopology() const { return mTopology; }
		void setTopology(skygfx::Topology value) { mTopology = value; }

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

		const auto& getTexturesMap() const { return mTexturesMap; }
		void setTexturesMap(const Renderer::Technique::TexturesMap& value) { mTexturesMap = value; }

		void setTexture(std::shared_ptr<skygfx::Texture> value) { mTexturesMap = value; }

	private:
		skygfx::Topology mTopology = skygfx::Topology::TriangleList;
		Indices mIndices;
		std::optional<std::vector<Vertex>> mVertices;
		std::set<Renderer::Shaders::Light::Flag> mShaderFlags;
		PositionAttribs mPositionAttribs;
		ColorAttribs mColorAttribs;
		NormalAttribs mNormalAttribs;
		TexCoordAttribs mTexCoordAttribs;
		Renderer::Shaders::Light::Material mMaterial;
		std::variant<Renderer::Technique::TexturesMap, std::shared_ptr<skygfx::Texture>> mTexturesMap;
	};*/
}