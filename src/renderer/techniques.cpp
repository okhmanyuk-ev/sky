#include "techniques.h"
#include <renderer/system.h>
#include <renderer/shaders/light.h> // TODO: del

using namespace Renderer::Techniques;

void ForwardLighting::Draw(const Buffer& vertex_buffer, const Buffer& index_buffer, const Vertex::Layout& layout,
	const Settings& settings, const TexturesMap& textures_map, std::shared_ptr<RenderTarget> target)
{
	static auto Shader = std::make_shared<Renderer::Shaders::Light>(layout);

	Shader->setProjectionMatrix(settings.matrices.projection);
	Shader->setViewMatrix(settings.matrices.view);
	Shader->setModelMatrix(settings.matrices.model);

	Renderer::Shaders::Light::DirectionalLight directional_light;
	directional_light.ambient = settings.directional_light.ambient;
	directional_light.diffuse = settings.directional_light.diffuse;
	directional_light.direction = settings.directional_light.direction;
	directional_light.specular = settings.directional_light.specular;
	
	Shader->setDirectionalLight(directional_light);
	Shader->setEyePosition(settings.eye_position);

	Renderer::Shaders::Light::Material material;
	material.ambient = settings.material.ambient;
	material.diffuse = settings.material.diffuse;
	material.shininess = settings.material.shininess;
	material.specular = settings.material.specular;

	Shader->setMaterial(material);

	RENDERER->setRenderTarget(target);
	RENDERER->clear();
	RENDERER->setVertexBuffer(vertex_buffer);
	RENDERER->setIndexBuffer(index_buffer);
	RENDERER->setShader(Shader);
	
	for (const auto& [texture, sub_indices]: textures_map)
	{
		RENDERER->setTexture(texture);
		RENDERER->drawIndexed(sub_indices.index_count, sub_indices.index_offset);
	}
}
