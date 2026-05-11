#include "graphical_console_commands.h"
#include <sky/imgui_system.h>
#include <shared/scene_editor.h>
#include <sky/console.h>
#include <sky/utils.h>
#include <common/console_commands.h>

using namespace Shared;

GraphicalConsoleCommands::GraphicalConsoleCommands()
{
	sky::AddCVar("r_resolution", sky::CommandProcessor::CVar("resolution of screen", { "int", "int" },
		CVAR_GETTER_INT2_FUNC(PLATFORM->getWidth, PLATFORM->getHeight), CVAR_SETTER_INT2_FUNC(PLATFORM->resize)));

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
