#include <sky/sky.h>

static const std::vector<skygfx::utils::Mesh::Vertex> Vertices = {
	/* front */
	/* 0  */ { .pos = { -1.0f,  1.0f,  1.0f }, .texcoord = { 0.0f, 0.0f }, .normal = { 0.0f, 0.0f, 1.0f } },
	/* 1  */ { .pos = {  1.0f,  1.0f,  1.0f }, .texcoord = { 1.0f, 0.0f }, .normal = { 0.0f, 0.0f, 1.0f } },
	/* 2  */ { .pos = { -1.0f, -1.0f,  1.0f }, .texcoord = { 0.0f, 1.0f }, .normal = { 0.0f, 0.0f, 1.0f } },
	/* 3  */ { .pos = {  1.0f, -1.0f,  1.0f }, .texcoord = { 1.0f, 1.0f }, .normal = { 0.0f, 0.0f, 1.0f } },

	/* top */
	/* 4  */ { .pos = { -1.0f,  1.0f,  1.0f }, .texcoord = { 0.0f, 0.0f }, .normal = { 0.0f, 1.0f, 0.0f } },
	/* 5  */ { .pos = { -1.0f,  1.0f, -1.0f }, .texcoord = { 0.0f, 1.0f }, .normal = { 0.0f, 1.0f, 0.0f } },
	/* 6  */ { .pos = {  1.0f,  1.0f,  1.0f }, .texcoord = { 1.0f, 0.0f }, .normal = { 0.0f, 1.0f, 0.0f } },
	/* 7  */ { .pos = {  1.0f,  1.0f, -1.0f }, .texcoord = { 1.0f, 1.0f }, .normal = { 0.0f, 1.0f, 0.0f } },

	/* left */
	/* 8  */ { .pos = { -1.0f,  1.0f, -1.0f }, .texcoord = { 0.0f, 0.0f }, .normal = { -1.0f, 0.0f, 0.0f } },
	/* 9  */ { .pos = { -1.0f,  1.0f,  1.0f }, .texcoord = { 1.0f, 0.0f }, .normal = { -1.0f, 0.0f, 0.0f } },
	/* 10 */ { .pos = { -1.0f, -1.0f, -1.0f }, .texcoord = { 0.0f, 1.0f }, .normal = { -1.0f, 0.0f, 0.0f } },
	/* 11 */ { .pos = { -1.0f, -1.0f,  1.0f }, .texcoord = { 1.0f, 1.0f }, .normal = { -1.0f, 0.0f, 0.0f } },

	/* back */
	/* 12 */ { .pos = { -1.0f,  1.0f, -1.0f }, .texcoord = { 1.0f, 0.0f }, .normal = { 0.0f, 0.0f, -1.0f } },
	/* 13 */ { .pos = { -1.0f, -1.0f, -1.0f }, .texcoord = { 1.0f, 1.0f }, .normal = { 0.0f, 0.0f, -1.0f } },
	/* 14 */ { .pos = {  1.0f,  1.0f, -1.0f }, .texcoord = { 0.0f, 0.0f }, .normal = { 0.0f, 0.0f, -1.0f } },
	/* 15 */ { .pos = {  1.0f, -1.0f, -1.0f }, .texcoord = { 0.0f, 1.0f }, .normal = { 0.0f, 0.0f, -1.0f } },

	/* bottom */
	/* 16 */ { .pos = { -1.0f, -1.0f,  1.0f }, .texcoord = { 0.0f, 0.0f }, .normal = { 0.0f, -1.0f, 0.0f } },
	/* 17 */ { .pos = {  1.0f, -1.0f,  1.0f }, .texcoord = { 0.0f, 1.0f }, .normal = { 0.0f, -1.0f, 0.0f } },
	/* 18 */ { .pos = { -1.0f, -1.0f, -1.0f }, .texcoord = { 1.0f, 0.0f }, .normal = { 0.0f, -1.0f, 0.0f } },
	/* 19 */ { .pos = {  1.0f, -1.0f, -1.0f }, .texcoord = { 1.0f, 1.0f }, .normal = { 0.0f, -1.0f, 0.0f } },

	/* right */
	/* 20 */ { .pos = { 1.0f, -1.0f, -1.0f }, .texcoord = { 1.0f, 1.0f }, .normal = { 1.0f, 0.0f, 0.0f } },
	/* 21 */ { .pos = { 1.0f, -1.0f,  1.0f }, .texcoord = { 0.0f, 1.0f }, .normal = { 1.0f, 0.0f, 0.0f } },
	/* 22 */ { .pos = { 1.0f,  1.0f, -1.0f }, .texcoord = { 1.0f, 0.0f }, .normal = { 1.0f, 0.0f, 0.0f } },
	/* 23 */ { .pos = { 1.0f,  1.0f,  1.0f }, .texcoord = { 0.0f, 0.0f }, .normal = { 1.0f, 0.0f, 0.0f } },
};

static const std::vector<uint32_t> Indices = {
	0, 1, 2, 1, 3, 2, // front
	4, 5, 6, 5, 7, 6, // top
	8, 9, 10, 9, 11, 10, // left
	12, 13, 14, 13, 15, 14, // back
	16, 17, 18, 17, 19, 18, // bottom
	20, 21, 22, 21, 23, 22, // right
};

void sky_main()
{
	sky::Application app("Cube", { sky::Application::Flag::Scene });

	auto viewport = std::make_shared<Scene::Viewport3D>();
	sky::GetService<Scene::Scene>()->getRoot()->attach(viewport);

	auto entity = std::make_shared<Scene::SingleMeshEntity>();
	entity->setVertices(Vertices);
	entity->setIndices(Indices);
	viewport->addEntity(entity);

	auto camera = viewport->getCamera();
	camera->pitch = glm::radians(-25.0f);
	camera->position = glm::vec3{ -5.0f, 2.0f, 0.0f };

	viewport->runAction(Actions::RepeatInfinite([entity] {
		return Actions::Interpolate(0.0f, 360.0f, 5.0f, Easing::Linear, [entity](float value) {
			entity->setRotation({ 0.0f, glm::radians(value), 0.0f });
		});
	}));

	app.run();
}