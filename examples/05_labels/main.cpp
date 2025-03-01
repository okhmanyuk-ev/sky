#include <sky/sky.h>

void sky_main()
{
	auto app = sky::Application("Labels", { sky::Application::Flag::Scene });

	Scene::Label::DefaultFont = sky::GetFont("fonts/sansation.ttf");

	auto label = std::make_shared<Scene::Label>();
	label->setAnchor(0.5f);
	label->setPivot(0.5f);
	label->setText(L"label");
	sky::GetService<Scene::Scene>()->getRoot()->attach(label);

	app.run();
}