#include "console_commands.h"
#include <numeric>
#include <fstream>
#include <algorithm>
#include <cstdlib>
#include <sky/scheduler.h>
#include <sky/utils.h>
#include <common/helpers.h>

using namespace Common;

static void OnCmdList(std::optional<std::string> filter)
{
	sky::Log("Commands:");

	auto commands = sky::GetService<sky::CommandProcessor>()->getItems()
		| std::views::filter([](const auto& pair) { return std::holds_alternative<sky::CommandProcessor::Command>(pair.second); })
		| std::views::transform([](const auto& pair) { return std::pair{ pair.first, std::get<sky::CommandProcessor::Command>(pair.second) }; })
		| std::views::filter([&](const auto& pair) { return !filter.has_value() || (pair.first.find(filter.value()) != std::string::npos); });

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

static void OnCVarList(std::optional<std::string> filter)
{
	sky::Log("CVars:");

	auto cvars = sky::GetService<sky::CommandProcessor>()->getItems()
		| std::views::filter([](const auto& pair) { return std::holds_alternative<sky::CommandProcessor::CVar>(pair.second); })
		| std::views::transform([](const auto& pair) { return std::pair{ pair.first, std::get<sky::CommandProcessor::CVar>(pair.second) }; })
		| std::views::filter([&](const auto& pair) { return !filter.has_value() || (pair.first.find(filter.value()) != std::string::npos); });

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

static void OnEcho(std::string text)
{
	sky::Log(text);
}

static void OnLater(float seconds, std::string command)
{
	sky::RunAction(sky::Actions::Delayed(seconds, [command] {
		sky::GetService<sky::CommandProcessor>()->execute(command);
	}));
}

static void OnClear()
{
	sky::GetService<sky::Console>()->clear();
}

static void OnAlias(std::optional<std::string> _name, std::optional<std::string> _value)
{
	const auto& items = sky::GetService<sky::CommandProcessor>()->getItems();

	if (!_value.has_value())
	{
		auto aliases = items
			| std::views::filter([](const auto& pair) { return std::holds_alternative<sky::CommandProcessor::Alias>(pair.second); })
			| std::views::transform([](const auto& pair) { return std::pair{ pair.first, std::get<sky::CommandProcessor::Alias>(pair.second) }; })
			| std::views::filter([&](const auto& pair) { return !_name.has_value() || (pair.first.find(_name.value()) != std::string::npos); });

		if (aliases.empty())
			sky::Log("alias list is empty");

		for (const auto& [name, alias] : aliases)
			sky::Log(" - {} = {}", name, sky::CommandProcessor::MakeStringFromTokens(alias.value));

		sky::Log("aliases can be invoked by adding 'dereference' prefix, example: *alias_name");
		return;
	}

	const auto& name = _name.value();

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

	const auto& value = _value.value();

	if (value.empty())
		return;

	sky::GetService<sky::CommandProcessor>()->addItem(name, sky::CommandProcessor::Alias(sky::CommandProcessor::MakeTokensFromString(value)));
	sky::Log("alias {} added", name);
}

static void OnIf(std::string var, std::string _condition_value, std::string then_cmd, std::optional<std::string> else_cmd)
{
	auto condition_value = sky::CommandProcessor::MakeTokensFromString(_condition_value);
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

	if (condition_value == var_value)
		sky::GetService<sky::CommandProcessor>()->execute(then_cmd);
	else if (else_cmd.has_value())
		sky::GetService<sky::CommandProcessor>()->execute(else_cmd.value());
}

static void OnChance(float probability, std::string then_cmd, std::optional<std::string> else_cmd)
{
	if (sky::chance(probability))
		sky::GetService<sky::CommandProcessor>()->execute(then_cmd);
	else if (else_cmd.has_value())
		sky::GetService<sky::CommandProcessor>()->execute(else_cmd.value());
}

static std::optional<float> TryParseFloat(const std::string& str)
{
	char* end = nullptr;
	auto value = std::strtof(str.c_str(), &end);

	if (str.empty() || end != str.c_str() + str.size())
		return std::nullopt;

	return value;
}

static bool IsSameValue(const std::vector<std::string>& a, const std::vector<std::string>& b)
{
	return std::ranges::equal(a, b, [](const auto& x, const auto& y) {
		auto fx = TryParseFloat(x);
		auto fy = TryParseFloat(y);

		if (fx.has_value() && fy.has_value())
			return fx.value() == fy.value();

		return x == y;
	});
}

static std::vector<std::string> GetNextValue(const std::vector<std::vector<std::string>>& values, const std::vector<std::string>& current)
{
	auto it = std::ranges::find_if(values, [&](const auto& value) { return IsSameValue(value, current); });

	if (it == values.end() || std::next(it) == values.end())
		return values.front();

	return *std::next(it);
}

static void OnToggle(std::string var, std::vector<std::string> _values)
{
	if (_values.size() == 1)
	{
		sky::Log("toggle needs at least two values");
		return;
	}

	if (_values.empty())
		_values = { "0", "1" };

	auto values = _values
		| std::views::transform(sky::CommandProcessor::MakeTokensFromString)
		| std::ranges::to<std::vector>();

	auto command_processor = sky::GetService<sky::CommandProcessor>();
	const auto& items = command_processor->getItems();
	auto item = items.find(var);

	if (item == items.end())
	{
		sky::Log("variable {} not found", var);
		return;
	}

	if (auto cvar = std::get_if<sky::CommandProcessor::CVar>(&item->second))
	{
		if (cvar->setter == nullptr)
		{
			sky::Log("cvar {} is readonly", var);
			return;
		}

		cvar->setter(GetNextValue(values, cvar->getter()));
		sky::Log("{} = {}", var, cvar->getValueAsString());
	}
	else if (auto alias = std::get_if<sky::CommandProcessor::Alias>(&item->second))
	{
		auto value = GetNextValue(values, alias->value);
		sky::Log("{} = {}", var, sky::CommandProcessor::MakeStringFromTokens(value));
		command_processor->removeItem(var);
		command_processor->addItem(var, sky::CommandProcessor::Alias(std::move(value)));
	}
	else
	{
		sky::Log("{} is a command, not a variable", var);
	}
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
	sky::AddCommand("cmdlist", "show list of commands", {}, {}, { "filter" }, OnCmdList);
	sky::AddCommand("cvarlist", "show list of cvars", {}, {}, { "filter" }, OnCVarList);
	sky::AddCommand("echo", "print to console", { "text" }, {}, {}, OnEcho);
	sky::AddCommand("later", "delayed execution", { "time", "command" }, {}, {}, OnLater);
	sky::AddCommand("clear", "clear console field", {}, {}, {}, OnClear);
	sky::AddCommand("alias", "manage aliases", {}, {}, { "name", "value" }, OnAlias);
	sky::AddCommand("if", "condition checking and execution", { "var", "value", "then" }, {}, { "else" }, OnIf);
	sky::AddCommand("chance", "random execution with probability 0.0..1.0", { "probability", "then" }, {}, { "else" }, OnChance);
	sky::AddCommand("toggle", "cycle cvar or alias through values (0 and 1 by default)", { "var" }, {}, { "values..." }, OnToggle);
	sky::AddCommand("quit", "shutdown the app", {}, {}, {}, [this] { onQuit(); });

	sky::Log("type \"cmdlist\" to see available commands");
}

void ConsoleCommands::onQuit()
{
	if (mQuitCallback != nullptr)
		mQuitCallback();
}
