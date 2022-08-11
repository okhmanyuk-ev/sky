#include "techniques.h"
#include <renderer/system.h>
#include <renderer/shaders/light.h> // TODO: del

using namespace Renderer;

void ForwardLightTechnique::draw(const Buffer& vertex_buffer, const Buffer& index_buffer, const skygfx::Vertex::Layout& layout,
	const TexturesMap& textures_map)
{
	static auto Shader = std::make_shared<Renderer::Shaders::Light>(layout, std::set{ Renderer::Shaders::Light::Flag::Textured });
	
	Shader->setProjectionMatrix(getProjectionMatrix());
	Shader->setViewMatrix(getViewMatrix());
	Shader->setModelMatrix(getModelMatrix());
	Shader->setEyePosition(mEyePosition);
	Shader->setDirectionalLight(getDirectionalLight());
	Shader->setPointLight(getPointLight());
	Shader->setMaterial(getMaterial());

	RENDERER->setTopology(skygfx::Topology::TriangleList);
	RENDERER->setVertexBuffer(vertex_buffer);
	RENDERER->setIndexBuffer(index_buffer);
	RENDERER->setShader(Shader);

	for (const auto& [texture, index_range] : textures_map)
	{
		RENDERER->setTexture(texture);
		RENDERER->drawIndexed(index_range.count, index_range.offset);
	}
}
