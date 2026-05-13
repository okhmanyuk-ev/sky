#include "graphical_console_commands.h"
#include <sky/imgui_system.h>
#include <shared/scene_editor.h>
#include <sky/console.h>
#include <sky/utils.h>
#include <common/console_commands.h>

using namespace Shared;

GraphicalConsoleCommands::GraphicalConsoleCommands()
{
	sky::AddCVar("r_resolution", sky::CVar<std::tuple<int, int>>::CreateDefinition(
		[] { return std::tuple{ PLATFORM->getWidth(), PLATFORM->getHeight() }; },
		[](std::tuple<int, int> value) { PLATFORM->resize(std::get<0>(value), std::get<1>(value)); }, "resolution of screen"));

	sky::AddCommand("rescale", "smart scaling", { "float" }, {}, {}, [](float value) {
		PLATFORM->rescale(value);
	});
}

void GraphicalConsoleCommands::onFrame()
{
	if (mShowTargets)
	{
		ImGui::Begin("Render Targets");
		for (const auto& [name, target] : GRAPHICS->getRenderTargets())
		{
			ImGui::Text("%s", name.c_str());
			ImGui::Text("%dx%d", target->getWidth(), target->getHeight());
			auto width = ImGui::GetContentRegionAvail().x;
			SceneEditor::drawImage(target, std::nullopt, width);
			ImGui::Separator();
		}
		ImGui::End();
	}
}
