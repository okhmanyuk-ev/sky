#include <sky/sky.h>

void sky_main()
{
	auto app = sky::Application("Labels", { sky::Application::Flag::Scene });

	Scene::Label::DefaultFont = sky::GetFont("fonts/sansation.ttf");

	auto bg = std::make_shared<Scene::Rectangle>();
	bg->setStretch(1.0f);
	bg->setColor(glm::vec4{ 0.125f, 0.125f, 0.125f, 1.0f });
	sky::GetService<Scene::Scene>()->getRoot()->attach(bg);

	auto column = std::make_shared<Scene::AutoSized<Scene::Column>>();
	column->setAnchor(0.5f);
	column->setPivot(0.5f);
	bg->attach(column);

	auto createLabel = [&](const std::wstring& text) {
		auto label = std::make_shared<Scene::Label>();
		label->setAnchor({ 0.5f, 0.0f });
		label->setPivot({ 0.5f, 0.0f });
		label->setParseColorTagsEnabled(true);
		label->setText(text);
		column->attach(label);
	};

	sky::GetService<sky::Scheduler>()->addInfinity([column] {
		auto scale = column->getScale().x;
		ImGui::Begin("Settings");
		ImGui::DragFloat("Scale", &scale, 0.01f);
		ImGui::End();
		column->setScale(scale);
	});

	createLabel(L"<b>Simple Label</b>");

	createLabel(L"<color=rgba(0,255,0,96)>Semi-transparent Green (RGBA)</color>");
	createLabel(L"<color=rgb(0,0,255)>Solid Blue (RGB)</color>");
	createLabel(L"<color=frgba(1,0,0,1)>Fully Opaque Red (FRGBA)</color>");
	createLabel(L"<color=frgb(1,0,1)>Magenta (FRGB)</color>");
	createLabel(L"<color=hex(00FF00FF)>Opaque Lime Green (HEX)</color>");

	createLabel(L"<color=red><b>Red Text</b> with <color=blue><i>Blue Nested</i></color> and back to red</color>");

	createLabel(L"<color=yellow><u>Yellow Text</u> with <color=green><b>Green Bold</b> and <color=gray>Gray Nested</color></color> ending here</color>");

	createLabel(L"<color=rgba(255,0,0,128)>Semi-Transparent Red</color> mixed with <color=hex(00FF00)>Solid Green (HEX)</color>");

	createLabel(L"<color=rgb(255,165,0)>Orange Text</color> combined with <color=frgba(0,1,0,0.5)><i>Translucent Green Italic</i></color>");

	createLabel(L"<color=red>Red <color=blue>Blue <color=green>Green</color> Back to Blue</color> Back to Red</color>");

	createLabel(L"<gradient=red_to_blue>Gradient from Red to Blue</gradient>");
	createLabel(L"<gradient=rainbow>Rainbow Gradient Text</gradient>");

	createLabel(L"<b>Bold Text</b>, <i>Italic Text</i>, and <u>Underlined Text</u>");

	createLabel(L"<color=rgba(255,0,0,192)><b><i>Semi-Bold Italic Red</i></b></color>");
	createLabel(L"<color=hex(0000FF)><u><b>Underlined Bold Blue</b></u></color>");

	createLabel(L"<color=yellow>Yellow <color=green>Green <color=blue>Blue <color=red>Red <color=purple>Purple</color> Back to Red</color> Back to Blue</color> Back to Green</color> Back to Yellow</color>");

	createLabel(L"<color=red>Red <color=blue>Blue <color=green>Green <color=yellow>Yellow <color=orange>Orange <color=purple>Purple <color=black>Black</color> Back to Purple</color> Back to Orange</color> Back to Yellow</color> Back to Green</color> Back to Blue</color> Back to Red</color>");

	app.run();
}