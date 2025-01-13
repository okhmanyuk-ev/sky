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
	sky::AddCVar("r_resolution", sky::CommandProcessor::CVar("resolution of screen", { "int", "int" },
		CVAR_GETTER_INT2_FUNC(PLATFORM->getWidth, PLATFORM->getHeight), CVAR_SETTER_INT2_FUNC(PLATFORM->resize)));

	sky::AddCVar("r_scale", sky::CommandProcessor::CVar("logical scaling on retina displays", { "float" },
		CVAR_GETTER_FLOAT_FUNC(PLATFORM->getScale), CVAR_SETTER_FLOAT_FUNC(PLATFORM->setScale)));

	sky::AddCVar("r_batching", sky::CommandProcessor::CVar(std::nullopt, { "bool" },
		CVAR_GETTER_BOOL_FUNC(GRAPHICS->isBatching), CVAR_SETTER_BOOL_FUNC(GRAPHICS->setBatching)));

	sky::AddCVar("r_vsync", sky::CommandProcessor::CVar(std::nullopt, { "bool" }, CVAR_GETTER_BOOL_FUNC(skygfx::IsVsyncEnabled),
		CVAR_SETTER_BOOL_FUNC(skygfx::SetVsync)));

	sky::AddCommand("rescale", sky::CommandProcessor::Command("smart scaling", { "float" }, [](CON_ARGS) {
		PLATFORM->rescale(CON_ARG_FLOAT(0));
	}));

	sky::AddCVar("r_showtargets", sky::CommandProcessor::CVar(std::nullopt, { "bool" }, CVAR_GETTER_BOOL(mShowTargets), CVAR_SETTER_BOOL(mShowTargets)));
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
