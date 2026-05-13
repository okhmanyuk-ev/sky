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
		[this] {
			return std::vector{
				std::to_string(PLATFORM->getWidth()),
				std::to_string(PLATFORM->getHeight())
			};
		},
		[this](const auto& args) {
			PLATFORM->resize(stoi(args.at(0)), stoi(args.at(1)));
		})
	);

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
