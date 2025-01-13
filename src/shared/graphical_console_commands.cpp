#include "graphical_console_commands.h"
#include <shared/imgui_system.h>
#include <shared/scene_editor.h>
#include <sky/console.h>
#include <sky/utils.h>
#include <platform/system.h>
#include <graphics/system.h>
#include <common/console_commands.h>

using namespace Shared;

GraphicalConsoleCommands::GraphicalConsoleCommands()
{
	sky::GetService<sky::CommandProcessor>()->registerCVar("r_resolution", "resolution of screen", { "int", "int" },
		CVAR_GETTER_INT2_FUNC(PLATFORM->getWidth, PLATFORM->getHeight),
		CVAR_SETTER_INT2_FUNC(PLATFORM->resize));

	sky::GetService<sky::CommandProcessor>()->registerCVar("r_scale", "logical scaling on retina displays", { "float" },
		CVAR_GETTER_FLOAT_FUNC(PLATFORM->getScale),
		CVAR_SETTER_FLOAT_FUNC(PLATFORM->setScale));

	sky::GetService<sky::CommandProcessor>()->registerCVar("r_batching", { "bool" },
		CVAR_GETTER_BOOL_FUNC(GRAPHICS->isBatching),
		CVAR_SETTER_BOOL_FUNC(GRAPHICS->setBatching));

	sky::GetService<sky::CommandProcessor>()->registerCVar("r_vsync", { "bool" },
		CVAR_GETTER_BOOL_FUNC(skygfx::IsVsyncEnabled),
		CVAR_SETTER_BOOL_FUNC(skygfx::SetVsync));

	sky::GetService<sky::CommandProcessor>()->registerCommand("rescale", "smart scaling", { "float" }, [](CON_ARGS) {
		PLATFORM->rescale(CON_ARG_FLOAT(0));
	});

	sky::GetService<sky::CommandProcessor>()->registerCVar("r_showtargets", { "bool" },
		CVAR_GETTER_BOOL(mShowTargets), CVAR_SETTER_BOOL(mShowTargets));
}

GraphicalConsoleCommands::~GraphicalConsoleCommands()
{
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
