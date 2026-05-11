#include "console_commands.h"
#include <numeric>
#include <fstream>
#include <algorithm>
#include <sky/scheduler.h>
#include <sky/utils.h>

using namespace Common;

static void OnCmdList(const std::vector<std::string>& args)
{
	sky::Log("Commands:");

	auto commands = sky::GetService<sky::CommandProcessor>()->getItems()
		| std::views::filter([](const auto& pair) { return std::holds_alternative<sky::CommandProcessor::Command>(pair.second); })
		| std::views::transform([](const auto& pair) { return std::pair{ pair.first, std::get<sky::CommandProcessor::Command>(pair.second) }; })
		| std::views::filter([&](const auto& pair) { return args.empty() || (pair.first.find(CON_ARG(0)) != std::string::npos); });

	for (const auto& [name, command] : commands)
	{
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

static void OnCVarList(const std::vector<std::string>& args)
{
	sky::Log("CVars:");

	auto cvars = sky::GetService<sky::CommandProcessor>()->getItems()
		| std::views::filter([](const auto& pair) { return std::holds_alternative<sky::CommandProcessor::CVar>(pair.second); })
		| std::views::transform([](const auto& pair) { return std::pair{ pair.first, std::get<sky::CommandProcessor::CVar>(pair.second) }; })
		| std::views::filter([&](const auto& pair) { return args.empty() || (pair.first.find(CON_ARG(0)) != std::string::npos); });

	for (const auto& [name, cvar] : cvars)
	{
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

static void OnEcho(const std::vector<std::string>& args)
{
	if (args.empty())
		return;

	auto str = std::accumulate(std::next(args.begin()), args.end(), *args.begin(),
		[](const auto& a, const auto& b) { return a + " " + b; });

	sky::Log(str);
}

static void OnLater(float seconds, std::string command)
{
	sky::RunAction(sky::Actions::Delayed(seconds, [command] {
		sky::GetService<sky::CommandProcessor>()->execute(command);
	}));
}

static void OnClear(const std::vector<std::string>& args)
{
	sky::GetService<sky::Console>()->clear();
}

static void OnAlias(const std::vector<std::string>& args)
{
	const auto& items = sky::GetService<sky::CommandProcessor>()->getItems();

	if (args.size() < 2)
	{
		auto aliases = items
			| std::views::filter([](const auto& pair) { return std::holds_alternative<sky::CommandProcessor::Alias>(pair.second); })
			| std::views::transform([](const auto& pair) { return std::pair{ pair.first, std::get<sky::CommandProcessor::Alias>(pair.second) }; })
			| std::views::filter([&](const auto& pair) { return args.empty() || (pair.first.find(CON_ARG(0)) != std::string::npos); });

		if (aliases.empty())
			sky::Log("alias list is empty");

		for (const auto& [name, alias] : aliases)
			sky::Log(" - {} = {}", name, sky::CommandProcessor::MakeStringFromTokens(alias.value));

		sky::Log("aliases can be invoked by adding 'dereference' prefix, example: *alias_name");
		return;
	}

	auto name = CON_ARG(0);

	if (name.empty())
	{
		sky::Log("alias name must not be empty");
		return;
	}

	if (items.contains(name))
	{
		const auto& item = items.at(name);
		auto is_alias = std::holds_alternative<sky::CommandProcessor::Alias>(item);

		if (!is_alias)
		{
			if (std::holds_alternative<sky::CommandProcessor::Command>(item))
				sky::Log("cannot add alias, command {} already exist", name);
			else if (std::holds_alternative<sky::CommandProcessor::CVar>(item))
				sky::Log("cannot add alias, cvar {} already exist", name);
			return;
		}

		sky::GetService<sky::CommandProcessor>()->removeItem(name);
		sky::Log("alias {} removed", name);
	}

	auto value = CON_ARG(1);

	if (value.empty())
		return;

	sky::GetService<sky::CommandProcessor>()->addItem(name, sky::CommandProcessor::Alias(sky::CommandProcessor::MakeTokensFromString(CON_ARG(1))));
	sky::Log("alias {} added", name);
}

static void OnIf(const std::vector<std::string>& args)
{
	auto var = CON_ARG(0);
	auto condition_value = sky::CommandProcessor::MakeTokensFromString(CON_ARG(1));
	auto then_cmd = CON_ARG(2);
	auto else_cmd = std::string();
	auto has_else_cmd = args.size() > 3;

	if (has_else_cmd)
		else_cmd = CON_ARG(3);

	const auto& items = sky::GetService<sky::CommandProcessor>()->getItems();

	auto var_value = std::vector<std::string>();

	if (!items.contains(var))
	{
		sky::Log("variable {} not found", var);
		return;
	}

	const auto& item = items.at(var);

	if (std::holds_alternative<sky::CommandProcessor::CVar>(item))
	{
		const auto& cvar = std::get<sky::CommandProcessor::CVar>(item);
		var_value = cvar.getter();
	}
	else if (std::holds_alternative<sky::CommandProcessor::Alias>(item))
	{
		const auto& alias = std::get<sky::CommandProcessor::Alias>(item);
		var_value = alias.value;
	}
	else
	{
		sky::Log("variable {} not found", var);
		return;
	}

	bool equals = condition_value == var_value;

	if (equals)
		sky::GetService<sky::CommandProcessor>()->execute(then_cmd);
	else if (has_else_cmd)
		sky::GetService<sky::CommandProcessor>()->execute(else_cmd);
}

ConsoleCommands::ConsoleCommands()
{
	auto getter = []() -> std::optional<float> {
		auto delta_limit = sky::Scheduler::Instance->getTimeDeltaLimit();
		if (!delta_limit.has_value())
			return std::nullopt;

		return 1.0f / sky::ToSeconds(delta_limit.value());
	};

	auto setter = [](std::optional<float> sec) {
		if (!sec.has_value())
		{
			sky::Scheduler::Instance->setTimeDeltaLimit(std::nullopt);
			return;
		}
		sky::Scheduler::Instance->setTimeDeltaLimit(sky::FromSeconds(1.0f / sec.value()));
	};

	sky::AddCVar("sys_time_delta_limit_fps", sky::CVar<std::optional<float>>::CreateDefinition(getter, setter));
	sky::AddCommand("cmdlist", sky::CommandProcessor::Command("show list of commands", {}, {}, { "filter" }, OnCmdList));
	sky::AddCommand("cvarlist", sky::CommandProcessor::Command("show list of cvars", {}, {}, { "filter" }, OnCVarList));
	sky::AddCommand("echo", sky::CommandProcessor::Command("print to console", { "text" }, {}, { "text.." }, OnEcho));
	sky::AddCommand("later", sky::CommandProcessor::Command("delayed execution", { "time", "command" }, {}, {}, sky::CreateCommandCallback<float, std::string>(OnLater)));
	sky::AddCommand("clear", sky::CommandProcessor::Command("clear console field", OnClear));
	sky::AddCommand("alias", sky::CommandProcessor::Command("manage aliases", OnAlias));
	sky::AddCommand("if", sky::CommandProcessor::Command("condition checking and execution", { "var", "value", "then" }, {}, { "else" }, OnIf));
	sky::AddCommand("quit", sky::CommandProcessor::Command("shutdown the app", [this](const auto& args) { onQuit(args); }));

	sky::Log("type \"cmdlist\" to see available commands");
}

void ConsoleCommands::onQuit(const std::vector<std::string>& args)
{
	if (mQuitCallback != nullptr)
		mQuitCallback();
}
