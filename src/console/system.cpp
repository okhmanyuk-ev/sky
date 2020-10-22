#include "system.h"
#include <algorithm>
#include <numeric>
#include <cassert>

using namespace Console;

void System::execute(const std::string& cmd)
{
	for (auto s : ParseCommandLine(cmd))
	{
		processConsoleCommand(dereferenceTokens(MakeTokensFromString(s)));
	}
}

void System::registerCommand(const std::string& name, const std::string& description,
	const std::vector<std::string>& args, const std::vector<std::string>& optional_args,
	Command::Callback callback)
{
	assert(mCommands.count(name) == 0);
	auto cmd = Command();
	cmd.mDescription = description;
	cmd.mCallback = callback;
	cmd.mArguments = args;
	cmd.mOptionalArguments = optional_args;
	mCommands[name] = cmd;
}

void System::registerCommand(const std::string& name, const std::string& description, const std::vector<std::string>& args, Command::Callback callback)
{
	registerCommand(name, description, args, {}, callback);
}

void System::registerCommand(const std::string& name, const std::string& description, Command::Callback callback)
{
	registerCommand(name, description, {}, callback);
}

void System::registerCommand(const std::string& name, const std::vector<std::string>& args, Command::Callback callback)
{
	registerCommand(name, {}, args, callback);
}

void System::registerCommand(const std::string& name, Command::Callback callback)
{
	registerCommand(name, {}, {}, callback);
}

void System::removeCommand(const std::string& name)
{
	mCommands.erase(name);
}

void System::registerCVar(const std::string& name, const std::string& description,
	const std::vector<std::string>& args, const std::vector<std::string>& optional_args,
	CVar::Getter getter, CVar::Setter setter)
{
	assert(mCVars.count(name) == 0);
	auto cvar = CVar();
	cvar.mDescription = description;
	cvar.mArguments = args;
	cvar.mOptionalArguments = optional_args;
	cvar.mGetter = getter;
	cvar.mSetter = setter;
	mCVars[name] = cvar;
}

void System::registerCVar(const std::string& name, const std::vector<std::string>& args,
	const std::vector<std::string>& optional_args, CVar::Getter getter, CVar::Setter setter)
{
	registerCVar(name, "", args, optional_args, getter, setter);
}

void System::registerCVar(const std::string& name, const std::string& description,
	const std::vector<std::string>& args, CVar::Getter getter, CVar::Setter setter)
{
	registerCVar(name, description, args, {}, getter, setter);
}

void System::registerCVar(const std::string& name, const std::vector<std::string>& args, 
	CVar::Getter getter, CVar::Setter setter)
{
	registerCVar(name, "", args, getter, setter);
}

void System::removeCVar(const std::string& name)
{
	mCVars.erase(name);
}

void System::addAlias(const std::string& name, const std::vector<std::string>& value)
{
	mAliases[name] = value;
}

void System::removeAlias(const std::string& name)
{
	mAliases.erase(name);
}

void System::onEvent(const Device::ReadEvent& e)
{
	execute(e.text);
}

std::vector<std::string> System::ParseCommandLine(const std::string& cmds)
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

std::vector<std::string> System::MakeTokensFromString(const std::string& cmd)
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

std::string System::MakeStringFromTokens(const std::vector<std::string>& value)
{
	if (value.empty())
		return "\"\"";

	return '"' + std::accumulate(std::next(value.begin()), value.end(), *value.begin(),
		[](const auto& a, const auto& b) { return a + "\" \"" + b; }) + '"';
}

void System::processConsoleCommand(std::vector<std::string> args)
{
	if (args.size() == 0)
		return;

	auto name = args[0];

	std::transform(name.begin(), name.end(), name.begin(), tolower);

	if (name.empty())
		return;

	args.erase(args.begin());

	bool known = false;

	if (mCVars.count(name) > 0)
	{
		known = true;

		auto cvar = mCVars.at(name);
		if (args.size() == 0 || cvar.getSetter() == nullptr)
			CONSOLE_DEVICE->writeLine(name + " = " + cvar.getValueAsString());
		else if (args.size() >= cvar.getArguments().size() && cvar.getSetter() != nullptr)
			cvar.getSetter()(args);
		else
			CONSOLE_DEVICE->writeLine("Syntax: " + name + " " + cvar.getArgsAsString());
	}

	if (mCommands.count(name) > 0)
	{
		known = true;

		auto command = mCommands.at(name);
		if (command.getArguments().size() > args.size())
			CONSOLE_DEVICE->writeLine("Syntax: " + name + " " + command.getArgsAsString());
		else
			command.getCallback()(args);
	}

	if (mAliases.count(name) > 0)
	{
		known = true;
		
		if (args.size() == 0)
			CONSOLE_DEVICE->writeLine(name + " = " + MakeStringFromTokens(mAliases.at(name)));
		else
			mAliases[name] = MakeTokensFromString(args[0]);
	}

	if (!known)
	{
		CONSOLE_DEVICE->writeLine("Unknown command: \"" + name + "\"");
	}
}

std::vector<std::string> System::dereferenceTokens(std::vector<std::string> tokens)
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

			if (mCVars.count(name) > 0)
			{
				args = mCVars.at(name).getGetter()();
			}
			else if (mAliases.count(name) > 0)
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
	return Console::System::MakeStringFromTokens(mGetter());
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