#pragma once

#include <string>
#include <functional>
#include <thread>
#include <optional>
#include <variant>
#include <platform/defines.h>
#include <sky/dispatcher.h>
#include <sky/locator.h>
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

	class CommandProcessor : public sky::Listenable<sky::Console::ReadEvent>
	{
	public:
		struct CVar
		{
			using Getter = std::function<std::vector<std::string>()>;
			using Setter = std::function<void(const std::vector<std::string>&)>;

			CVar(std::optional<std::string> description, std::vector<std::string> arguments, Getter getter, Setter setter);

			std::string getValueAsString() const;
			std::string getArgsAsString() const;

			std::optional<std::string> description;
			std::vector<std::string> arguments;
			Getter getter;
			Setter setter;
		};

		struct Command
		{
			using Callback = std::function<void(const std::vector<std::string>&)>;

			struct DefaultArgument
			{
				DefaultArgument(std::string name, std::string default_value);
				std::string name;
				std::string default_value;
			};

			Command(std::optional<std::string> description, std::vector<std::string> arguments, std::vector<DefaultArgument> default_arguments,
				std::vector<std::string> optional_arguments, Callback callback);
			Command(std::optional<std::string> description, Callback callback);

			std::string getArgsAsString() const;

			std::optional<std::string> description;
			std::vector<std::string> arguments;
			std::vector<DefaultArgument> default_arguments;
			std::vector<std::string> optional_arguments;
			Callback callback;
		};

		struct Alias
		{
			Alias(std::vector<std::string> value);
			std::vector<std::string> value;
		};

		using Item = std::variant<
			CVar,
			Command,
			Alias
		>;

	public:
		void execute(const std::string& cmd);
		void addItem(const std::string& name, Item item);
		void removeItem(const std::string& name);

	public:
		const auto& getItems() const { return mItems; }

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
		std::map<std::string, Item> mItems;
	};

	template<typename T>
	struct CVarTraits;

	template<>
	struct CVarTraits<bool>
	{
		static const std::vector<std::string> Args;
		static std::vector<std::string> ValueToArgs(bool value);
		static bool ArgsToValue(const std::vector<std::string>& args);
	};

	template<>
	struct CVarTraits<int>
	{
		static const std::vector<std::string> Args;
		static std::vector<std::string> ValueToArgs(int value);
		static int ArgsToValue(const std::vector<std::string>& args);
	};

	template<>
	struct CVarTraits<float>
	{
		static const std::vector<std::string> Args;
		static std::vector<std::string> ValueToArgs(float value);
		static float ArgsToValue(const std::vector<std::string>& args);
	};

	template<class T>
	class CVar
	{
	public:
		using Getter = std::function<T()>;
		using Setter = std::function<void(T)>;

		CVar(const std::string& name, Getter getter, Setter setter, std::optional<std::string> description = std::nullopt) :
			mName(name),
			mGetter(getter),
			mSetter(setter)
		{
			sky::Locator<CommandProcessor>::Get()->addItem(mName, sky::CommandProcessor::CVar(description, CVarTraits<T>::Args,
				[this] { return CVarTraits<T>::ValueToArgs(mGetter()); }, [this](const auto& args) { mSetter(CVarTraits<T>::ArgsToValue(args)); }));
		}

		CVar(const std::string& name, std::shared_ptr<T> value, std::optional<std::string> description = std::nullopt) :
			CVar(name, [value] { return *value; }, [value](T _value) { *value = _value; }, description)
		{
		}

		CVar(const std::string& name, T value, std::optional<std::string> description = std::nullopt) :
			CVar(name, std::make_shared<T>(value), description)
		{
		}

		~CVar()
		{
			sky::Locator<CommandProcessor>::Get()->removeItem(mName);
		}

		CVar(const CVar&) = delete;
		CVar& operator=(const CVar&) = delete;

		operator T() const { return mGetter(); }
		CVar& operator=(T value) { mSetter(value); return *this; }

	private:
		std::string mName;
		Getter mGetter;
		Setter mSetter;
	};
}
