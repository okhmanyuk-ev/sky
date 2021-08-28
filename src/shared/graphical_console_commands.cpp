#include "graphical_console_commands.h"
#include "platform/system_windows.h"
#include <shared/imgui_system.h>
#include <shared/scene_editor.h>

using namespace Shared;

GraphicalConsoleCommands::GraphicalConsoleCommands()
{
	CONSOLE->registerCVar("r_resolution", "resolution of screen", { "int", "int" },
		CVAR_GETTER_INT2_FUNC(PLATFORM->getWidth, PLATFORM->getHeight),
		CVAR_SETTER_INT2_FUNC(PLATFORM->resize));

	CONSOLE->registerCVar("r_scale", "logical scaling on retina displays", { "float" },
		CVAR_GETTER_FLOAT_FUNC(PLATFORM->getScale),
		CVAR_SETTER_FLOAT_FUNC(PLATFORM->setScale));

	CONSOLE->registerCVar("r_batching", { "bool" },
		CVAR_GETTER_BOOL_FUNC(GRAPHICS->isBatching),
		CVAR_SETTER_BOOL_FUNC(GRAPHICS->setBatching));

	CONSOLE->registerCVar("r_vsync", { "bool" },
		CVAR_GETTER_BOOL_FUNC(RENDERER->isVsync),
		CVAR_SETTER_BOOL_FUNC(RENDERER->setVsync));

	CONSOLE->registerCommand("rescale", "smart scaling", { "float" }, [](CON_ARGS) {
		PLATFORM->rescale(CON_ARG_FLOAT(0));
	});

	CONSOLE->registerCVar("r_showtargets", { "bool" },
		CVAR_GETTER_BOOL(mShowTargets), CVAR_SETTER_BOOL(mShowTargets));
}

GraphicalConsoleCommands::~GraphicalConsoleCommands()
{
	//
}

void GraphicalConsoleCommands::onFrame()
{
	if (mShowTargets)
	{
		ImGui::Begin("Render Targets");
		for (const auto& [name, target] : GRAPHICS->getRenderTargets())
		{
			ImGui::Text(name.c_str());
			ImGui::Text("%dx%d", target->getWidth(), target->getHeight());
			auto width = ImGui::GetContentRegionAvailWidth();
			SceneEditor::drawImage(target, std::nullopt, width);
			ImGui::Separator();
		}
		ImGui::End();
	}
}