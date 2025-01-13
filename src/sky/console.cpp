#include "console.h"
#include <conio.h>
#include <Windows.h>
#include <iostream>
#include <string>
#include <common/frame_system.h>
#include <sky/utils.h>

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
			FRAME->addOne([this, s] {
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

CVar::CVar(std::optional<std::string> description, std::vector<std::string> arguments, std::vector<std::string> optional_arguments,
	Getter getter, Setter setter) :
	mDescription(description),
	mArguments(arguments),
	mOptionalArguments(optional_arguments),
	mGetter(getter),
	mSetter(setter)
{
}

Command::Command(std::optional<std::string> description, std::vector<std::string> arguments, std::vector<std::string> optional_arguments,
	Callback callback) :
	mDescription(description),
	mArguments(arguments),
	mOptionalArguments(optional_arguments),
	mCallback(callback)
{
}

void CommandProcessor::execute(const std::string& cmd)
{
	for (auto s : ParseCommandLine(cmd))
	{
		processConsoleCommand(dereferenceTokens(MakeTokensFromString(s)));
	}
}

void CommandProcessor::registerCommand(const std::string& name, std::optional<std::string> description,
	const std::vector<std::string>& args, const std::vector<std::string>& optional_args,
	Command::Callback callback)
{
	assert(!mCommands.contains(name));
	auto cmd = Command(description, args, optional_args, callback);
	mCommands.insert({ name, cmd });
}

void CommandProcessor::registerCommand(const std::string& name, std::optional<std::string> description, const std::vector<std::string>& args, Command::Callback callback)
{
	registerCommand(name, description, args, {}, callback);
}

void CommandProcessor::registerCommand(const std::string& name, std::optional<std::string> description, Command::Callback callback)
{
	registerCommand(name, description, {}, callback);
}

void CommandProcessor::registerCommand(const std::string& name, const std::vector<std::string>& args, Command::Callback callback)
{
	registerCommand(name, std::nullopt, args, callback);
}

void CommandProcessor::registerCommand(const std::string& name, Command::Callback callback)
{
	registerCommand(name, std::nullopt, {}, callback);
}

void CommandProcessor::removeCommand(const std::string& name)
{
	mCommands.erase(name);
}

void CommandProcessor::registerCVar(const std::string& name, std::optional<std::string> description,
	const std::vector<std::string>& args, const std::vector<std::string>& optional_args,
	CVar::Getter getter, CVar::Setter setter)
{
	assert(!mCVars.contains(name));
	auto cvar = CVar(description, args, optional_args, getter, setter);
	mCVars.insert({ name, cvar });
}

void CommandProcessor::registerCVar(const std::string& name, const std::vector<std::string>& args,
	const std::vector<std::string>& optional_args, CVar::Getter getter, CVar::Setter setter)
{
	registerCVar(name, std::nullopt, args, optional_args, getter, setter);
}

void CommandProcessor::registerCVar(const std::string& name, std::optional<std::string> description,
	const std::vector<std::string>& args, CVar::Getter getter, CVar::Setter setter)
{
	registerCVar(name, description, args, {}, getter, setter);
}

void CommandProcessor::registerCVar(const std::string& name, const std::vector<std::string>& args,
	CVar::Getter getter, CVar::Setter setter)
{
	registerCVar(name, std::nullopt, args, getter, setter);
}

void CommandProcessor::removeCVar(const std::string& name)
{
	mCVars.erase(name);
}

void CommandProcessor::addAlias(const std::string& name, const std::vector<std::string>& value)
{
	mAliases[name] = value;
}

void CommandProcessor::removeAlias(const std::string& name)
{
	mAliases.erase(name);
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

	bool known = false;

	if (mCVars.contains(name))
	{
		known = true;

		auto cvar = mCVars.at(name);
		if (args.size() == 0 || cvar.getSetter() == nullptr)
			sky::GetService<sky::Console>()->writeLine(name + " = " + cvar.getValueAsString());
		else if (args.size() >= cvar.getArguments().size() && cvar.getSetter() != nullptr)
			cvar.getSetter()(args);
		else
			sky::GetService<sky::Console>()->writeLine("Syntax: " + name + " " + cvar.getArgsAsString());
	}

	if (mCommands.contains(name))
	{
		known = true;

		auto command = mCommands.at(name);
		if (command.getArguments().size() > args.size())
			sky::GetService<sky::Console>()->writeLine("Syntax: " + name + " " + command.getArgsAsString());
		else
			command.getCallback()(args);
	}

	if (mAliases.contains(name))
	{
		known = true;

		if (args.size() == 0)
			sky::GetService<sky::Console>()->writeLine(name + " = " + MakeStringFromTokens(mAliases.at(name)));
		else
			mAliases[name] = MakeTokensFromString(args[0]);
	}

	if (!known)
	{
		sky::GetService<sky::Console>()->writeLine("Unknown command: \"" + name + "\"");
	}
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

			if (tokens[i][0] != '#')
				continue;

			auto name = tokens[i].substr(1);
			std::transform(name.begin(), name.end(), name.begin(), tolower);
			tokens[i].clear();

			auto args = std::vector<std::string>();

			if (mCVars.contains(name))
			{
				args = mCVars.at(name).getGetter()();
			}
			else if (mAliases.contains(name))
			{
				args = mAliases.at(name);
			}
			else
			{
				continue;
			}

			tokens.erase(std::next(tokens.begin(), i));
			tokens.insert(std::next(tokens.begin(), i), args.begin(), args.end());
			dereferenced = true;
		}
	}

	return tokens;
}

std::string CVar::getValueAsString() const
{
	return CommandProcessor::MakeStringFromTokens(mGetter());
}

std::string CVar::getArgsAsString() const
{
	std::string result = "";

	if (mArguments.size() > 0)
		result += "<" + std::accumulate(std::next(mArguments.begin()), mArguments.end(), *mArguments.begin(),
			[](const auto& a, const auto& b) { return a + "> <" + b; }) + ">";

	if (mArguments.size() > 0 && mOptionalArguments.size() > 0)
		result += " ";

	if (mOptionalArguments.size() > 0)
		result += "(<" + std::accumulate(std::next(mOptionalArguments.begin()), mOptionalArguments.end(), *mOptionalArguments.begin(),
			[](const auto& a, const auto& b) { return a + ">) (<" + b; }) + ">)";

	return result;
}

std::string Command::getArgsAsString() const
{
	std::string result = "";

	if (mArguments.size() > 0)
		result += "<" + std::accumulate(std::next(mArguments.begin()), mArguments.end(), *mArguments.begin(),
			[](const auto& a, const auto& b) { return a + "> <" + b; }) + ">";

	if (mArguments.size() > 0 && mOptionalArguments.size() > 0)
		result += " ";

	if (mOptionalArguments.size() > 0)
		result += "(<" + std::accumulate(std::next(mOptionalArguments.begin()), mOptionalArguments.end(), *mOptionalArguments.begin(),
			[](const auto& a, const auto& b) { return a + ">) (<" + b; }) + ">)";

	return result;
}
