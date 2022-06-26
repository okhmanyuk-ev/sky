#pragma once

#include <glm/glm.hpp>
#include <renderer/buffer.h>
#include <renderer/vertex.h>
#include <renderer/render_target.h>
#include <unordered_map>

namespace Renderer::Techniques
{
	namespace ForwardLighting
	{
		struct Matrices
		{
			glm::mat4 view = glm::mat4(1.0f);
			glm::mat4 projection = glm::mat4(1.0f);
			glm::mat4 model = glm::mat4(1.0f);
		};

		struct DirectionalLight
		{
			glm::vec3 direction = { 0.75f, 0.75f, 0.75f };
			glm::vec3 ambient = { 0.5f, 0.5f, 0.5f };
			glm::vec3 diffuse = { 1.0f, 1.0f, 1.0f };
			glm::vec3 specular = { 1.0f, 1.0f, 1.0f };
		};

		struct Material
		{
			glm::vec3 ambient = { 0.5f, 0.5f, 0.5f };
			glm::vec3 diffuse = { 0.5f, 0.5f, 0.5f };
			glm::vec3 specular = { 0.5f, 0.5f, 0.5f };
			float shininess = 32.0f;
		};

		struct Settings
		{
			Matrices matrices;
			glm::vec3 eye_position = { 0.0f, 0.0f, 0.0f };
			DirectionalLight directional_light;
			Material material;
		};

		struct SubIndices
		{
			int index_offset;
			int index_count;
		};

		using TexturesMap = std::unordered_map<std::shared_ptr<Renderer::Texture>, SubIndices>;

		void Draw(const Buffer& vertex_buffer, const Buffer& index_buffer, const Vertex::Layout& layout,
			const Settings& settings, const TexturesMap& textures_map, std::shared_ptr<RenderTarget> target = nullptr);
	}
}