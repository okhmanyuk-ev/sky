#include "console.h"
#ifdef PLATFORM_WINDOWS
#include <Windows.h>
#endif
#include <iostream>
#include <string>
#include <sky/scheduler.h>
#include <sky/utils.h>
#include <ranges>

using namespace sky;

void EmbeddedConsole::write(const std::string& s, Console::Color color)
{
	if (mWriteCallback)
	{
		mWriteCallback(s, color);
	}
}

void EmbeddedConsole::writeLine(const std::string& s, Console::Color color)
{
	if (mWriteLineCallback)
	{
		mWriteLineCallback(s, color);
	}
}

void EmbeddedConsole::clear()
{
	if (mClearCallback)
	{
		mClearCallback();
	}
}

#ifdef PLATFORM_WINDOWS

NativeConsole::NativeConsole()
{
	mReadThread = std::thread([this] {
		while (true)
		{
			std::string s;
			std::getline(std::cin, s);
			SCHEDULER->addOne([this, s] {
				sky::Emit(ReadEvent({ s }));
			});
		}
	});

	mReadThread.detach();

	mConsoleHandle = GetStdHandle(STD_OUTPUT_HANDLE);
	SetConsoleOutputCP(1251);
}

NativeConsole::~NativeConsole()
{
	//mReadThread.~mReadThread();
}

void NativeConsole::write(const std::string& s, Console::Color color)
{
	SetConsoleTextAttribute(mConsoleHandle, color == Console::Color::Default ? (WORD)Console::Color::Gray : (WORD)color);
	std::cout << s;
	SetConsoleTextAttribute(mConsoleHandle, (WORD)Console::Color::Gray);
}

void NativeConsole::writeLine(const std::string& s, Console::Color color)
{
	write(s, color);
	std::cout << std::endl;
}

void NativeConsole::clear()
{
	system("cls");
}

void NativeConsole::setTitle(const std::string& s)
{
	SetConsoleTitle(std::string(s).c_str());
}

#endif

CommandProcessor::CVar::CVar(std::optional<std::string> _description, std::vector<std::string> _arguments, Getter _getter, Setter _setter) :
	description(_description),
	arguments(_arguments),
	getter(_getter),
	setter(_setter)
{
}

CommandProcessor::Command::DefaultArgument::DefaultArgument(std::string _name, std::string _default_value) :
	name(_name), default_value(_default_value)
{
}

CommandProcessor::Command::Command(std::optional<std::string> _description, std::vector<std::string> _arguments, std::vector<DefaultArgument> _default_arguments,
	std::vector<std::string> _optional_arguments, Callback _callback) :
	description(_description),
	arguments(_arguments),
	default_arguments(_default_arguments),
	optional_arguments(_optional_arguments),
	callback(_callback)
{
}

CommandProcessor::Command::Command(std::optional<std::string> description, Callback callback) :
	Command(description, {}, {}, {}, callback)
{
}

CommandProcessor::Alias::Alias(std::vector<std::string> _value) :
	value(_value)
{
}

void CommandProcessor::execute(const std::string& cmd)
{
	for (auto s : ParseCommandLine(cmd))
	{
		try
		{
			processConsoleCommand(dereferenceTokens(MakeTokensFromString(s)));
		}
		catch (const std::exception& e)
		{
			sky::Log("raised exception in command \"{}\", reason: \"{}\"", s, e.what());
		}
	}
}

void CommandProcessor::addItem(const std::string& name, Item item)
{
	assert(!name.empty());
	assert(!mItems.contains(name));
	mItems.insert({ name, item });
}

void CommandProcessor::removeItem(const std::string& name)
{
	assert(!name.empty());
	assert(mItems.contains(name));
	mItems.erase(name);
}

void CommandProcessor::onEvent(const sky::Console::ReadEvent& e)
{
	execute(e.text);
}

std::vector<std::string> CommandProcessor::ParseCommandLine(const std::string& cmds)
{
	int quotes = 0;
	char quote = '\0';

	bool wasSlash = false;

	std::vector<std::string> result;
	std::string s;

	for (auto& c : cmds)
	{
		if (c == '/' && quotes == 0)
		{
			if (wasSlash)
			{
				s.pop_back();
				break;
			}
			else
			{
				wasSlash = true;
			}
		}
		else
		{
			wasSlash = false;
		}

		if (c == '\'' || c == '"')
		{
			if (c == quote)
			{
				quotes--;
				quote = (quote == '"') ? '\'' : '"';
			}
			else
			{
				quotes++;
				quote = c;
			}
		}

		if (c == ';' && quotes == 0)
		{
			result.push_back(s);
			s.clear();
			continue;
		}

		s += c;
	}

	result.push_back(s);

	return result;
}

std::vector<std::string> CommandProcessor::MakeTokensFromString(const std::string& cmd)
{
	int quotes = 0;
	char quote = '\0';

	std::vector<std::string> tokens;
	std::string token;

	for (auto& c : cmd)
	{
		switch (c)
		{
		case ' ':
		case '\t':
		case '\n':
		case '\r':
		case '\f':
		case '\v':
			if (quotes == 0)
			{
				if (token.length() > 0)
				{
					tokens.push_back(token);
					token.clear();
				}
			}
			else
			{
				token += c;
			}

			break;

		case '"':
		case '\'':

			if (quote == c) // quote was closed
			{
				quotes--;

				if (quotes == 0) // last quote was closed
				{
					tokens.push_back(token); // add empty token
					token.clear();
					quote = '\0';
				}
				else
				{
					token += c;
					quote = (quote == '"') ? '\'' : '"';
				}
			}
			else // quote entered new level
			{
				quote = c;

				if (quotes == 0)
				{
					if (token.length() > 0)
					{
						tokens.push_back(token);
						token.clear();
					}
				}
				else
				{
					token += c;
				}

				quotes++;
			}

			break;

		default:
			token += c;
			break;
		}
	}

	if (token.length() > 0)
		tokens.push_back(token);

	return tokens;
}

std::string CommandProcessor::MakeStringFromTokens(const std::vector<std::string>& value)
{
	if (value.empty())
		return "\"\"";

	return '"' + std::accumulate(std::next(value.begin()), value.end(), *value.begin(),
		[](const auto& a, const auto& b) { return a + "\" \"" + b; }) + '"';
}

void CommandProcessor::processConsoleCommand(std::vector<std::string> args)
{
	if (args.size() == 0)
		return;

	auto name = args[0];

	std::transform(name.begin(), name.end(), name.begin(), tolower);

	if (name.empty())
		return;

	args.erase(args.begin());

	if (!mItems.contains(name))
	{
		sky::Log("Unknown command: {}", name);
		return;
	}

	auto& item = mItems.at(name);

	std::visit(cases{
		[&](const CVar& cvar) {
			if (args.size() == 0 || cvar.setter == nullptr)
				sky::Log("{} = {}", name, cvar.getValueAsString());
			else if (args.size() >= cvar.arguments.size() && cvar.setter != nullptr)
				cvar.setter(args);
			else
				sky::Log("Syntax: {} {}", name, cvar.getArgsAsString());
		},
		[&](const Command& command) {
			if (command.arguments.size() > args.size())
			{
				sky::Log("Syntax: {} {}", name, command.getArgsAsString());
				return;
			}
			for (size_t i = 0; i < command.default_arguments.size(); i++)
			{
				if (command.arguments.size() + i + 1 > args.size())
					args.push_back(command.default_arguments.at(i).default_value);
			}
			command.callback(args);
		},
		[&](Alias& alias) {
			if (args.size() == 0)
				sky::Log("{} = {}", name, MakeStringFromTokens(alias.value));
			else
				alias.value = MakeTokensFromString(args[0]);
		}
	}, item);
}

std::vector<std::string> CommandProcessor::dereferenceTokens(std::vector<std::string> tokens)
{
	bool dereferenced = true;

	while (dereferenced)
	{
		dereferenced = false;

		for (int i = static_cast<int>(tokens.size()) - 1; i >= 0; i--)
		{
			if (tokens[i].empty())
				continue;

			if (tokens[i][0] != '*')
				continue;

			auto name = tokens[i].substr(1);
			std::transform(name.begin(), name.end(), name.begin(), tolower);
			tokens[i].clear();

			if (!mItems.contains(name))
				continue;

			const auto& item = mItems.at(name);
			auto args = std::vector<std::string>();

			std::visit(cases{
				[&](const CVar& cvar) {
					args = cvar.getter();
				},
				[&](const Command& command) { },
				[&](const Alias& alias) {
					args = alias.value;
				}
			}, item);

			tokens.erase(std::next(tokens.begin(), i));
			tokens.insert(std::next(tokens.begin(), i), args.begin(), args.end());
			dereferenced = true;
		}
	}

	return tokens;
}

std::string CommandProcessor::CVar::getValueAsString() const
{
	return CommandProcessor::MakeStringFromTokens(getter());
}

std::string CommandProcessor::CVar::getArgsAsString() const
{
	auto args = arguments
		| sky::ranges::wrap("<", ">")
		| std::ranges::to<std::vector>()
		| std::views::filter(std::not_fn(std::ranges::empty))
		| std::ranges::to<std::vector>();

	return sky::join(args, " ");
}

std::string CommandProcessor::Command::getArgsAsString() const
{
	auto to_string = std::views::transform([](const DefaultArgument& arg) { return arg.name + "=" + arg.default_value; });
	auto args_groups = {
		arguments | sky::ranges::wrap("<", ">") | std::ranges::to<std::vector>(),
		default_arguments | to_string | sky::ranges::wrap("<", ">") | std::ranges::to<std::vector>(),
		optional_arguments | sky::ranges::wrap("(<", ">)") | std::ranges::to<std::vector>()
	};
	auto args = std::views::join(args_groups) | std::views::filter(std::not_fn(std::ranges::empty)) | std::ranges::to<std::vector>();
	return sky::join(args, " ");
}

const std::vector<std::string> CVarTraits<bool>::Args = { "bool" };

std::vector<std::string> CVarTraits<bool>::ValueToArgs(bool value)
{
	return { std::to_string(value) };
}

bool CVarTraits<bool>::ArgsToValue(const std::vector<std::string>& args)
{
	return stoi(args.at(0)) > 0;
}

const std::vector<std::string> CVarTraits<int>::Args = { "int" };

std::vector<std::string> CVarTraits<int>::ValueToArgs(int value)
{
	return { std::to_string(value) };
}

int CVarTraits<int>::ArgsToValue(const std::vector<std::string>& args)
{
	return stoi(args.at(0));
}

const std::vector<std::string> CVarTraits<float>::Args = { "float" };

std::vector<std::string> CVarTraits<float>::ValueToArgs(float value)
{
	return { std::to_string(value) };
}

float CVarTraits<float>::ArgsToValue(const std::vector<std::string>& args)
{
	return stof(args.at(0));
}
