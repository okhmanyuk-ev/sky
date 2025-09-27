#include <sky/sky.h>

void sky_main()
{
	sky::Application app("Physics", { sky::Application::Flag::Scene });

	auto world = std::make_shared<Shared::PhysHelpers::World>();
	world->setAnchor(0.5f);
	world->setPivot(0.5f);
	world->setDebug(true);
	sky::GetService<Scene::Scene>()->getRoot()->attach(world);

	auto floor = std::make_shared<Shared::PhysHelpers::Entity>();
	floor->setSize({ 768.0f, 16.0f });
	floor->setPivot(0.5f);
	floor->setPosition({ 0.0f, 192.0f });
	world->attach(floor);

	auto floor_rect = std::make_shared<Scene::Rectangle>();
	floor_rect->setStretch(1.0f);
	floor_rect->setRounding(0.5f);
	floor->attach(floor_rect);

	auto enableAutoSuicide = [](std::shared_ptr<Scene::Node> node){
		node->runAction(sky::Actions::ExecuteInfinite([node]{
			if (node->getY() > 1000.0f)
				node->runAction(sky::Actions::Kill(node));
		}));
	};

	static glm::vec2 BoxSize = { 24.0f, 24.0f };
	static float BallSize = 24.0f;
	static float y = -96.0f;

	auto spawnBox = [world, enableAutoSuicide]{
		auto box = std::make_shared<Shared::PhysHelpers::Entity>();
		box->setType(Shared::PhysHelpers::Entity::Type::Dynamic);
		box->setSize(BoxSize);
		box->setPivot(0.5f);
		box->setY(y);
		enableAutoSuicide(box);
		world->attach(box);

		auto rect = std::make_shared<Scene::Rectangle>();
		rect->setStretch(1.0f);
		rect->setRounding(0.5f);
		box->attach(rect);
	};

	auto spawnBall = [world, enableAutoSuicide]{
		auto ball = std::make_shared<Shared::PhysHelpers::Entity>();
		ball->setType(Shared::PhysHelpers::Entity::Type::Dynamic);
		ball->setShape(Shared::PhysHelpers::Entity::Shape::Circle);
		ball->setSize(BallSize);
		ball->setPivot(0.5f);
		ball->setY(y);
		enableAutoSuicide(ball);
		world->attach(ball);

		auto circle = std::make_shared<Scene::Circle>();
		circle->setStretch(1.0f);
		ball->attach(circle);
	};

	static bool auto_spawn = true;
	static float ChanceValue = 0.5f;

	world->runAction(sky::Actions::Pausable([]{ return auto_spawn; }, sky::Actions::RepeatInfinite([spawnBox, spawnBall]() -> std::optional<sky::Action> {
		const auto delay = 0.125f;

		return sky::Actions::Sequence(
			sky::Actions::Wait(delay),
			[spawnBox, spawnBall]{
				if (sky::chance(ChanceValue))
					spawnBox();
				else
					spawnBall();
			}
		);
	})));

	sky::RunAction([spawnBox, spawnBall] {
		ImGui::Begin("Options", nullptr, ImGui::User::ImGuiWindowFlags_ControlPanel);
		ImGui::SetWindowPos(ImGui::User::TopRightCorner());

		ImGui::Checkbox("Auto spawn", &auto_spawn);
		ImGui::Separator();
		ImGui::DragFloat2("Box size", (float*)&BoxSize, 1.0f, 1.0f, 256.0f);
		ImGui::DragFloat("Ball size", &BallSize, 1.0f, 1.0f, 256.0f);
		ImGui::SliderFloat("Ball <-> Box", &ChanceValue, 0.0f, 1.0f);
		ImGui::Separator();

		if (ImGui::Button("Spawn 1 box"))
		{
			spawnBox();
		}

		if (ImGui::Button("Spawn 25 boxes"))
		{
			for (int i = 0; i < 25; i++)
			{
				spawnBox();
			}
		}

		if (ImGui::Button("Spawn 100 boxes"))
		{
			for (int i = 0; i < 100; i++)
			{
				spawnBox();
			}
		}

		ImGui::Separator();

		if (ImGui::Button("Spawn 1 ball"))
		{
			spawnBall();
		}

		if (ImGui::Button("Spawn 25 ball"))
		{
			for (int i = 0; i < 25; i++)
			{
				spawnBall();
			}
		}

		if (ImGui::Button("Spawn 100 ball"))
		{
			for (int i = 0; i < 100; i++)
			{
				spawnBall();
			}
		}

		ImGui::End();

		return sky::Action::Result::Continue;
	});

	app.run();
}