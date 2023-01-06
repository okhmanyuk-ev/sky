#include "console_commands.h"
#include <numeric>
#include <fstream>
#include <algorithm>

using namespace Common;

ConsoleCommands::ConsoleCommands()
{
	CONSOLE->registerCVar("sys_framerate", "limit of fps", { "int" }, 
		CVAR_GETTER_INT_FUNC(FRAME->getFramerateLimit),
		CVAR_SETTER_INT_FUNC(FRAME->setFramerateLimit));

	CONSOLE->registerCVar("sys_sleep", "cpu saving between frames", { "bool" },
		CVAR_GETTER_BOOL_FUNC(FRAME->isSleepAllowed),
		CVAR_SETTER_BOOL_FUNC(FRAME->setSleepAllowed));

	CONSOLE->registerCVar("sys_timescale", "time delta multiplier", { "float" },
		CVAR_GETTER_DOUBLE_FUNC(FRAME->getTimeScale),
		CVAR_SETTER_DOUBLE_FUNC(FRAME->setTimeScale));
		
	auto getter = []() -> std::vector<std::string> {
		auto delta_limit = FRAME->getTimeDeltaLimit();
		if (!delta_limit.has_value())
			return { "null" };
		
		auto fps = 1.0f / Clock::ToSeconds(delta_limit.value());
		return { std::to_string(fps) };
	};

	auto setter = [](CON_ARGS) {
		if (CON_ARG(0) == "null")
		{
			FRAME->setTimeDeltaLimit(std::nullopt);
			return;
		}

		auto sec = std::stof(CON_ARG(0));
		FRAME->setTimeDeltaLimit(Clock::FromSeconds(1.0f / sec));
	};

	CONSOLE->registerCVar("sys_time_delta_limit_fps", { "null/float" }, getter, setter);

	CONSOLE->registerCommand("cmdlist", "show list of commands", {}, { "filter" },
		CMD_METHOD(onCmdList));

	CONSOLE->registerCommand("cvarlist", "show list of cvars", {}, { "filter" },
		CMD_METHOD(onCVarList));

	CONSOLE->registerCommand("echo", "print to console", { "text" }, { "text.." },
		CMD_METHOD(onEcho));

	CONSOLE->registerCommand("later", "delayed execution", { "time", "command" },
		CMD_METHOD(onLater));

	CONSOLE->registerCommand("exec", "execute console commands from file", { "path" }, { "path.." },
		CMD_METHOD(onExec));

	CONSOLE->registerCommand("clear", "clear console field",
		CMD_METHOD(onClear));

	CONSOLE->registerCommand("alias", "manage aliases",
		CMD_METHOD(onAlias));
		
	CONSOLE->registerCommand("if", "condition checking and execution", { "var", "value", "then" }, { "else" },
		CMD_METHOD(onIf));

	CONSOLE->registerCommand("quit", "shutdown the app",
		CMD_METHOD(onQuit));
		
	CONSOLE_DEVICE->writeLine("type \"cmdlist\" to see available commands");
}

ConsoleCommands::~ConsoleCommands()
{
	//
}

void ConsoleCommands::onCmdList(CON_ARGS)
{
	CONSOLE_DEVICE->writeLine("Commands:");

	for (auto& [name, command] : CONSOLE->getCommands())
	{
		if (CON_ARG_EXIST(0) && (name.find(CON_ARG(0)) == std::string::npos))
			continue;

		auto s = " - " + name;

		auto args = command.getArgsAsString();

		if (!args.empty())
			s += " " + args;

		auto description = command.getDescription();
		
		if (description.has_value())
			s += " - " + description.value();

		CONSOLE_DEVICE->writeLine(s);
	}
}

void ConsoleCommands::onCVarList(CON_ARGS)
{
	CONSOLE_DEVICE->writeLine("CVars:");

	for (auto& [name, cvar] : CONSOLE->getCVars())
	{
		if (CON_HAS_ARGS && (name.find(CON_ARG(0)) == std::string::npos))
			continue;

		auto value = cvar.getValueAsString();
		auto description = cvar.getDescription();
		bool hasSetter = cvar.getSetter() != nullptr;

		auto s = " - " + name + " = " + value;

		if (description.has_value())
			s += " - " + description.value();

		if (!hasSetter)
			s += " (readonly)";

		CONSOLE_DEVICE->writeLine(s);
	}
}

void ConsoleCommands::onEcho(CON_ARGS)
{
	CONSOLE_DEVICE->writeLine(CON_ARGS_ACCUMULATED_STRING);
}

void ConsoleCommands::onLater(CON_ARGS)
{
	float seconds = 0.0f;

	try
	{
		seconds = std::stof(CON_ARG(0));
	}
	catch (const std::exception& e)
	{
		CONSOLE_DEVICE->writeLine(e.what());
		return;
	}

	Actions::Run(Actions::Collection::Delayed(seconds,
		Actions::Collection::Execute([this, command = CON_ARG(1)] {
			CONSOLE->execute(command);
		})
	));
}
	
void ConsoleCommands::onExec(CON_ARGS)
{
	/*for (auto path : args) // TODO: find solution
	{
		if (!std::experimental::filesystem::exists(path))
		{
			CONSOLE_DEVICE.writeLine(path + " does not exist");
			return;
		}

		std::ifstream file(path, std::fstream::in);
		std::string line;

		while (std::getline(file, line))
		{
			CONSOLE.execute(line);
		}
	}*/
	CONSOLE_DEVICE->writeLine("TODO"); // TODO: make
}

void ConsoleCommands::onClear(CON_ARGS)
{
	CONSOLE_DEVICE->clear();
}

void ConsoleCommands::onAlias(CON_ARGS)
{
	const auto& aliases = CONSOLE->getAliases();
		
	if (CON_ARGS_COUNT < 2)
	{
		if (aliases.empty())
		{
			CONSOLE_DEVICE->writeLine("alias list is empty");
		}
		else
		{
			for (auto& [name, value] : aliases)
			{
				if (CON_HAS_ARGS && (name.find(CON_ARG(0)) == std::string::npos))
					continue;

				CONSOLE_DEVICE->writeLine(" - " + name + " = " + Console::System::MakeStringFromTokens(value));
			}
		}
		CONSOLE_DEVICE->writeLine("aliases can be invoked by adding 'sharp' prefix, example: #alias_name");

		return;
	}

	auto name = CON_ARG(0);

	if (aliases.count(name) > 0)
	{
		if (CON_ARG_EXIST(1))
		{
			auto value = CON_ARG(1);

			if (!value.empty())
			{
				CONSOLE->addAlias(name, Console::System::MakeTokensFromString(value));
			}
			else
			{
				CONSOLE->removeAlias(name);
				CONSOLE_DEVICE->writeLine("alias \"" + name + "\" removed");
			}
		}
		else
		{
			CONSOLE_DEVICE->writeLine("alias \"" + name + "\" already exist");
		}

		return;
	}

	CONSOLE->addAlias(name, Console::System::MakeTokensFromString(CON_ARG(1)));
}

void ConsoleCommands::onIf(CON_ARGS)
{
	auto var = CON_ARG(0);
	auto condition_value = Console::System::MakeTokensFromString(CON_ARG(1));
	auto then_cmd = CON_ARG(2);
	auto else_cmd = std::string();
	auto has_else_cmd = CON_ARG_EXIST(3);
		
	if (has_else_cmd)
		else_cmd = CON_ARG(3);

	auto& cvars = CONSOLE->getCVars();
	auto& aliases = CONSOLE->getAliases();

	auto var_value = std::vector<std::string>();

	if (cvars.count(var) > 0)
	{
		var_value = cvars.at(var).getGetter()();
	}
	else if (aliases.count(var) > 0)
	{
		var_value = aliases.at(var);
	}
	else
	{
		CONSOLE_DEVICE->writeLine("variable \"" + var + "\" not found");
		return;
	}

	bool equals = condition_value == var_value;

	if (equals)
		CONSOLE->execute(then_cmd);
	else if (has_else_cmd)
		CONSOLE->execute(else_cmd);
}

void ConsoleCommands::onQuit(CON_ARGS)
{
	if (mQuitCallback != nullptr)
		mQuitCallback();
}
