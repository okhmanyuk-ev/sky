#pragma once

#include <glm/glm.hpp>
#include <vector>
#include <algorithm>

namespace Renderer::Vertex
{
	struct Attribute
	{
		enum class Type
		{
			Position,
			Color,
			TexCoord,
			Normal
		};

		enum class Format
		{
			R32F,
			R32G32F,
			R32G32B32F,
			R32G32B32A32F,
			R8UN,
			R8G8UN,
			R8G8B8UN,
			R8G8B8A8UN
		};

		Vertex::Attribute::Type type;
		Vertex::Attribute::Format format;
		size_t offset;
	};

	struct Layout // TODO: rename to VertexFormat or smth
	{
		size_t stride;
		std::vector<Attribute> attributes;

		bool hasAttribute(Vertex::Attribute::Type type) const
		{
			return attributes.cend() != std::find_if(attributes.cbegin(), attributes.cend(), [type](const Vertex::Attribute& attrib) {
				return attrib.type == type;
			});
		}
	};

	// predefined vertex types:

	struct Position
	{
		glm::vec3 pos;

		static const Layout Layout;
	};

	struct PositionColor
	{
		glm::vec3 pos;
		glm::vec4 col;

		static const Layout Layout;
	};

	struct PositionTexture 
	{
		glm::vec3 pos;
		glm::vec2 tex;
	
		static const Layout Layout;
	};

	struct PositionNormal
	{
		glm::vec3 pos;
		glm::vec3 normal;

		static const Layout Layout;
	};

	struct PositionColorNormal
	{
		glm::vec3 pos;
		glm::vec4 col;
		glm::vec3 normal;

		static const Layout Layout;
	};

	struct PositionColorTexture
	{
		glm::vec3 pos;
		glm::vec4 col;
		glm::vec2 tex;

		static const Layout Layout;
	};

	struct PositionTextureNormal
	{
		glm::vec3 pos;
		glm::vec2 tex;
		glm::vec3 normal;

		static const Layout Layout;
	};
}