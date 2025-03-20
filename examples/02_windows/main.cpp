#include <sky/sky.h>

static std::shared_ptr<Shared::SceneManager::Window> CreateWindow()
{
	auto window = std::make_shared<Shared::SceneHelpers::StandardWindow>();

	auto rect = std::make_shared<Scene::Rectangle>();
	rect->setSize(256.0f);
	rect->setAnchor(0.5f);
	rect->setPivot(0.5f);
	rect->setAlpha(0.5f);
	rect->setAbsoluteRounding(true);
	rect->setRounding(16.0f);
	rect->setTouchable(true);
	window->getContent()->attach(rect);

	using Button = Shared::SceneHelpers::BouncingButtonBehavior<Shared::SceneHelpers::Button<Scene::Rectangle>>;

	auto close = std::make_shared<Button>();
	close->setSize(32.0f);
	close->setRounding(0.5f);
	close->setAnchor({ 1.0f, 0.0f });
	close->setPivot({ 1.0f, 0.0f });
	close->setPosition({ -8.0f, 8.0f });
	close->setColor({ sky::GetColor(sky::Color::Red), 0.5f });
	close->setClickCallback([] {
		SCENE_MANAGER->popWindow();
	});
	rect->attach(close);

	auto push = std::make_shared<Button>();
	push->setSize({ 96.0f, 32.0f });
	push->setAnchor({ 0.5f, 1.0f });
	push->setPivot({ 0.5f, 1.0f });
	push->setY(-8.0f);
	push->setAlpha(0.5f);
	push->setRounding(0.5f);
	push->setClickCallback([] {
		auto window = CreateWindow();
		SCENE_MANAGER->pushWindow(window);
	});
	rect->attach(push);

	return window;
}

void sky_main()
{
	sky::Application app("Windows", { sky::Application::Flag::Scene });

	auto window = CreateWindow();
	SCENE_MANAGER->pushWindow(window);

	app.run();
}