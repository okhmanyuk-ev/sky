#pragma once

#include <common/event_system.h>
#include <console/device.h>
#include <sky/locator.h>
#include <string>
#include <vector>
#include <list>
#include <map>
#include <optional>

#define CONSOLE sky::Locator<Console::System>::GetService()

namespace Console
{
	class CVar
	{
	public:
		friend class System;

	public:
		using Getter = std::function<std::vector<std::string>()>;
		using Setter = std::function <void(const std::vector<std::string>&)>;

	public:
		std::string getValueAsString() const;
		std::string getArgsAsString() const;

	public:
		const auto& getDescription() const { return mDescription; }
		const auto& getArguments() const { return mArguments; }
		const auto& getOptionalArguments() const { return mOptionalArguments; }
		auto getGetter() const { return mGetter; }
		auto getSetter() const { return mSetter; }

	private:
		std::optional<std::string> mDescription;
		std::vector<std::string> mArguments;
		std::vector<std::string> mOptionalArguments;
		Getter mGetter;
		Setter mSetter;
	};

	class Command
	{
	public:
		friend class System;

	public:
		using Callback = std::function<void(const std::vector<std::string>&)>;

	public:
		std::string getArgsAsString() const;

	public:
		auto getDescription() const { return mDescription; }
		const auto& getArguments() const { return mArguments; }
		const auto& getOptionalArguments() const { return mOptionalArguments; }
		auto getCallback() const { return mCallback; }

	private:
		std::optional<std::string> mDescription;
		std::vector<std::string> mArguments;
		std::vector<std::string> mOptionalArguments;
		Callback mCallback;
	};

	class System : public Common::Event::Listenable<Device::ReadEvent>
	{
	public:
		void execute(const std::string& cmd);

	public:
		void registerCommand(const std::string& name, std::optional<std::string> description,
			const std::vector<std::string>& args, const std::vector<std::string>& optional_args,
			Command::Callback callback);

		void registerCommand(const std::string& name, std::optional<std::string> description,
			const std::vector<std::string>& args, Command::Callback callback);

		void registerCommand(const std::string& name, std::optional<std::string> description,
			Command::Callback callback);

		void registerCommand(const std::string& name, const std::vector<std::string>& args,
			Command::Callback callback);

		void registerCommand(const std::string& name, Command::Callback callback);

		void removeCommand(const std::string& name);

	public:
		void registerCVar(const std::string& name, std::optional<std::string> description,
			const std::vector<std::string>& args, const std::vector<std::string>& optional_args,
			CVar::Getter getter, CVar::Setter setter = nullptr);

		void registerCVar(const std::string& name, const std::vector<std::string>& args,
			const std::vector<std::string>& optional_args, CVar::Getter getter, CVar::Setter setter = nullptr);

		void registerCVar(const std::string& name, std::optional<std::string> description,
			const std::vector<std::string>& args, CVar::Getter getter, CVar::Setter setter = nullptr);

		void registerCVar(const std::string& name, const std::vector<std::string>& args,
			CVar::Getter getter, CVar::Setter setter = nullptr);

		void removeCVar(const std::string& name);

	public:
		void addAlias(const std::string& name, const std::vector<std::string>& value);
		void removeAlias(const std::string& name);

	public:
		const auto& getCVars() const { return mCVars; }
		const auto& getCommands() const { return mCommands; }
		const auto& getAliases() const { return mAliases; }

	public:
		void onEvent(const Device::ReadEvent& e) override;

	public:
		static std::vector<std::string> ParseCommandLine(const std::string& cmds);
		static std::vector<std::string> MakeTokensFromString(const std::string& cmd);
		static std::string MakeStringFromTokens(const std::vector<std::string>& value);

	private:
		void processConsoleCommand(std::vector<std::string> args);
		std::vector<std::string> dereferenceTokens(std::vector<std::string> tokens);

	private:
		std::map<std::string, CVar> mCVars;
		std::map<std::string, Command> mCommands;
		std::map<std::string, std::vector<std::string>> mAliases;
	};
}