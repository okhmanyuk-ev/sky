#pragma once

#include <string>
#include <functional>
#include <thread>
#include <optional>
#include <platform/defines.h>
#include <sky/dispatcher.h>
#include <map>

namespace sky
{
	class Console
	{
	public:
		enum class Color
		{
			Black = 0,
			DarkBlue = 1,
			DarkGreen = 2,
			DarkCyan = 3,
			DarkRed = 4,
			DarkMagenta = 5,
			DarkYellow = 6,
			Gray = 7,
			DarkGray = 8,
			Blue = 9,
			Green = 10,
			Cyan = 11,
			Red = 12,
			Magenta = 13,
			Yellow = 14,
			White = 15,
			Default
		};

	public:
		struct ReadEvent
		{
			std::string text;
		};

	public:
		virtual void write(const std::string& s, Color color = Color::Default) = 0;
		virtual void writeLine(const std::string& s, Color color = Color::Default) = 0;
		virtual void clear() = 0;

		virtual bool isOpened() const = 0;

		virtual bool isEnabled() const = 0;
		virtual void setEnabled(bool value) = 0;
	};

	class EmbeddedConsole : public sky::Console
	{
	public:
		typedef std::function<void(const std::string&, Console::Color)> WriteCallback;
		typedef std::function<void()> ClearCallback;

	public:
		void write(const std::string& s, Console::Color color = Console::Color::Default) override;
		void writeLine(const std::string& s, Console::Color color = Console::Color::Default) override;
		void clear() override;

		bool isOpened() const  override { return true; }

		bool isEnabled() const override { return true; }
		void setEnabled(bool value)  override {}

	public:
		void setWriteCallback(WriteCallback value) { mWriteCallback = value; }
		void setWriteLineCallback(WriteCallback value) { mWriteLineCallback = value; }
		void setClearCallback(ClearCallback value) { mClearCallback = value; }

	private:
		WriteCallback mWriteCallback = nullptr;
		WriteCallback mWriteLineCallback = nullptr;
		ClearCallback mClearCallback = nullptr;
	};

#ifdef PLATFORM_WINDOWS
	class NativeConsole : public sky::Console
	{
	public:
		NativeConsole();
		~NativeConsole();

	public:
		void write(const std::string& s, Console::Color color = Console::Color::Default) override;
		void writeLine(const std::string& s, Console::Color color = Console::Color::Default) override;
		void clear() override;
		bool isOpened() const override { return true; }

		bool isEnabled() const override { return true; }
		void setEnabled(bool value) override {};

	public:
		void setTitle(const std::string& s);

	private:
		std::thread mReadThread;
		void* mConsoleHandle;
	};
#endif

	class CVar
	{
	public:
		using Getter = std::function<std::vector<std::string>()>;
		using Setter = std::function<void(const std::vector<std::string>&)>;

	public:
		CVar(std::optional<std::string> description, std::vector<std::string> arguments, std::vector<std::string> optional_arguments,
			Getter getter, Setter setter);

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
		using Callback = std::function<void(const std::vector<std::string>&)>;

	public:
		Command(std::optional<std::string> description, std::vector<std::string> arguments, std::vector<std::string> optional_arguments,
			Callback callback);

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

	class CommandProcessor : public sky::Listenable<sky::Console::ReadEvent>
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
		void onEvent(const sky::Console::ReadEvent& e) override;

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