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

	Shader->setEyePosition(settings.eye_position);

	Renderer::Shaders::Light::DirectionalLight directional_light;
	directional_light.ambient = settings.directional_light.ambient;
	directional_light.diffuse = settings.directional_light.diffuse;
	directional_light.direction = settings.directional_light.direction;
	directional_light.specular = settings.directional_light.specular;
	Shader->setDirectionalLight(directional_light);

	Renderer::Shaders::Light::PointLight point_light;
	point_light.position = settings.point_light.position;
	point_light.constantAttenuation = settings.point_light.constantAttenuation;
	point_light.linearAttenuation = settings.point_light.linearAttenuation;
	point_light.quadraticAttenuation = settings.point_light.quadraticAttenuation;
	point_light.ambient = settings.point_light.ambient;
	point_light.diffuse = settings.point_light.diffuse;
	point_light.specular = settings.point_light.specular;
	Shader->setPointLight(point_light);

	Renderer::Shaders::Light::Material material;
	material.ambient = settings.material.ambient;
	material.diffuse = settings.material.diffuse;
	material.shininess = settings.material.shininess;
	material.specular = settings.material.specular;
	Shader->setMaterial(material);

	RENDERER->setRenderTarget(target);
	RENDERER->setVertexBuffer(vertex_buffer);
	RENDERER->setIndexBuffer(index_buffer);
	RENDERER->setShader(Shader);
	
	for (const auto& [texture, index_batch]: textures_map)
	{
		RENDERER->setTexture(texture);
		RENDERER->drawIndexed(index_batch.index_count, index_batch.index_offset);
	}
}
