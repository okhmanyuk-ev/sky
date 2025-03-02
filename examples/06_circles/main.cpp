#include <sky/sky.h>

void sky_main()
{
	auto app = sky::Application("Circles", { sky::Application::Flag::Scene });

	auto bg = std::make_shared<Scene::Rectangle>();
	bg->setStretch(1.0f);
	bg->setColor(glm::vec4{ 0.125f, 0.125f, 0.125f, 1.0f });
	sky::GetService<Scene::Scene>()->getRoot()->attach(bg);

	auto circle = std::make_shared<Scene::Circle>();
	circle->setSize(100.0f);
	circle->setAnchor(0.5f);
	circle->setPivot(0.5f);
	bg->attach(circle);

	app.run();
}