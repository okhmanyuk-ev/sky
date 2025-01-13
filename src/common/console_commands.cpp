#include "console_commands.h"
#include <numeric>
#include <fstream>
#include <algorithm>

using namespace Common;

ConsoleCommands::ConsoleCommands()
{
	auto getter = []() -> std::vector<std::string> {
		auto delta_limit = FRAME->getTimeDeltaLimit();
		if (!delta_limit.has_value())
			return { "null" };

		auto fps = 1.0f / sky::ToSeconds(delta_limit.value());
		return { std::to_string(fps) };
	};

	auto setter = [](CON_ARGS) {
		if (CON_ARG(0) == "null")
		{
			FRAME->setTimeDeltaLimit(std::nullopt);
			return;
		}

		auto sec = std::stof(CON_ARG(0));
		FRAME->setTimeDeltaLimit(sky::FromSeconds(1.0f / sec));
	};

	sky::AddCVar("sys_time_delta_limit_fps", sky::CommandProcessor::CVar(std::nullopt, { "null/float" }, getter, setter));
	sky::AddCommand("cmdlist", sky::CommandProcessor::Command("show list of commands", {}, { "filter" }, CMD_METHOD(onCmdList)));
	sky::AddCommand("cvarlist", sky::CommandProcessor::Command("show list of cvars", {}, { "filter" }, CMD_METHOD(onCVarList)));
	sky::AddCommand("echo", sky::CommandProcessor::Command("print to console", { "text" }, { "text.." }, CMD_METHOD(onEcho)));
	sky::AddCommand("later", sky::CommandProcessor::Command("delayed execution", { "time", "command" }, CMD_METHOD(onLater)));
	sky::AddCommand("exec", sky::CommandProcessor::Command("execute console commands from file", { "path" }, { "path.." }, CMD_METHOD(onExec)));
	sky::AddCommand("clear", sky::CommandProcessor::Command("clear console field", CMD_METHOD(onClear)));
	sky::AddCommand("alias", sky::CommandProcessor::Command("manage aliases", CMD_METHOD(onAlias)));
	sky::AddCommand("if", sky::CommandProcessor::Command("condition checking and execution", { "var", "value", "then" }, { "else" }, CMD_METHOD(onIf)));
	sky::AddCommand("quit", sky::CommandProcessor::Command("shutdown the app", CMD_METHOD(onQuit)));

	sky::Log("type \"cmdlist\" to see available commands");
}

ConsoleCommands::~ConsoleCommands()
{
}

void ConsoleCommands::onCmdList(CON_ARGS)
{
	sky::Log("Commands:");

	for (auto& [name, command] : sky::GetService<sky::CommandProcessor>()->getCommands())
	{
		if (CON_ARG_EXIST(0) && (name.find(CON_ARG(0)) == std::string::npos))
			continue;

		auto s = " - " + name;

		auto args = command.getArgsAsString();

		if (!args.empty())
			s += " " + args;

		auto description = command.description;

		if (description.has_value())
			s += " - " + description.value();

		sky::Log(s);
	}
}

void ConsoleCommands::onCVarList(CON_ARGS)
{
	sky::Log("CVars:");

	for (auto& [name, cvar] : sky::GetService<sky::CommandProcessor>()->getCVars())
	{
		if (CON_HAS_ARGS && (name.find(CON_ARG(0)) == std::string::npos))
			continue;

		auto value = cvar.getValueAsString();
		auto description = cvar.description;
		bool hasSetter = cvar.setter != nullptr;

		auto s = " - " + name + " = " + value;

		if (description.has_value())
			s += " - " + description.value();

		if (!hasSetter)
			s += " (readonly)";

		sky::Log(s);
	}
}

void ConsoleCommands::onEcho(CON_ARGS)
{
	sky::Log(CON_ARGS_ACCUMULATED_STRING);
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
		sky::Log(e.what());
		return;
	}

	Actions::Run(Actions::Collection::Delayed(seconds,
		Actions::Collection::Execute([this, command = CON_ARG(1)] {
			sky::GetService<sky::CommandProcessor>()->execute(command);
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
	sky::Log("TODO"); // TODO: make
}

void ConsoleCommands::onClear(CON_ARGS)
{
	sky::GetService<sky::Console>()->clear();
}

void ConsoleCommands::onAlias(CON_ARGS)
{
	const auto& aliases = sky::GetService<sky::CommandProcessor>()->getAliases();

	if (CON_ARGS_COUNT < 2)
	{
		if (aliases.empty())
		{
			sky::Log("alias list is empty");
		}
		else
		{
			for (auto& [name, value] : aliases)
			{
				if (CON_HAS_ARGS && (name.find(CON_ARG(0)) == std::string::npos))
					continue;

				sky::Log(" - " + name + " = " + sky::CommandProcessor::MakeStringFromTokens(value));
			}
		}
		sky::Log("aliases can be invoked by adding 'sharp' prefix, example: #alias_name");

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
				sky::GetService<sky::CommandProcessor>()->addAlias(name, sky::CommandProcessor::MakeTokensFromString(value));
			}
			else
			{
				sky::GetService<sky::CommandProcessor>()->removeAlias(name);
				sky::Log("alias \"" + name + "\" removed");
			}
		}
		else
		{
			sky::Log("alias \"" + name + "\" already exist");
		}

		return;
	}

	sky::GetService<sky::CommandProcessor>()->addAlias(name, sky::CommandProcessor::MakeTokensFromString(CON_ARG(1)));
}

void ConsoleCommands::onIf(CON_ARGS)
{
	auto var = CON_ARG(0);
	auto condition_value = sky::CommandProcessor::MakeTokensFromString(CON_ARG(1));
	auto then_cmd = CON_ARG(2);
	auto else_cmd = std::string();
	auto has_else_cmd = CON_ARG_EXIST(3);

	if (has_else_cmd)
		else_cmd = CON_ARG(3);

	auto& cvars = sky::GetService<sky::CommandProcessor>()->getCVars();
	auto& aliases = sky::GetService<sky::CommandProcessor>()->getAliases();

	auto var_value = std::vector<std::string>();

	if (cvars.count(var) > 0)
	{
		var_value = cvars.at(var).getter();
	}
	else if (aliases.count(var) > 0)
	{
		var_value = aliases.at(var);
	}
	else
	{
		sky::Log("variable \"" + var + "\" not found");
		return;
	}

	bool equals = condition_value == var_value;

	if (equals)
		sky::GetService<sky::CommandProcessor>()->execute(then_cmd);
	else if (has_else_cmd)
		sky::GetService<sky::CommandProcessor>()->execute(else_cmd);
}

void ConsoleCommands::onQuit(CON_ARGS)
{
	if (mQuitCallback != nullptr)
		mQuitCallback();
}
