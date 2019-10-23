#include "graphical_console_commands.h"
#include "Platform/system_windows.h"

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
}

GraphicalConsoleCommands::~GraphicalConsoleCommands()
{
	//
}