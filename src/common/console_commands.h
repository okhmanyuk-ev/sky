#pragma once
#include <sky/utils.h>
#include <vector>
#include <string>

#define CVAR_GETTER(V) [this] { return std::vector<std::string>({ V }); }
#define CVAR_GETTER2(V1, V2) [this] { return std::vector<std::string>({ V1, V2 }); }

#define CVAR_GETTER_INT(V) CVAR_GETTER(std::to_string(V))
#define CVAR_GETTER_INT2(V1, V2) CVAR_GETTER2(std::to_string(V1), std::to_string(V2))
#define CVAR_GETTER_INT2_FUNC(V1, V2) CVAR_GETTER_INT2(V1(), V2())

#define CVAR_GETTER_BOOL(V) CVAR_GETTER(std::to_string(V))

#define CON_ARGS_NAME args
#define CON_ARGS const std::vector<std::string>& CON_ARGS_NAME

#define CON_ARG(N) CON_ARGS_NAME.at(N)
#define CON_ARG_INT(N) stoi(CON_ARG(N))
#define CON_ARG_BOOL(N) static_cast<bool>(stoi(CON_ARG(N)))
#define CON_ARG_FLOAT(N) stof(CON_ARG(N))

#define CVAR_SETTER(V) [this](CON_ARGS) { V; }

#define CVAR_SETTER_BOOL(V) CVAR_SETTER(V = CON_ARG_BOOL(0))

#define CVAR_SETTER_INT(V) CVAR_SETTER(V = CON_ARG_INT(0))
#define CVAR_SETTER_INT2_FUNC(V) CVAR_SETTER(V(CON_ARG_INT(0), CON_ARG_INT(1)))

#define CMD_METHOD(V) [this](CON_ARGS) { V(CON_ARGS_NAME); }

namespace Common
{
	class ConsoleCommands
	{
	public:
		ConsoleCommands();

	private:
		void onCmdList(CON_ARGS);
		void onCVarList(CON_ARGS);
		void onEcho(CON_ARGS);
		void onLater(CON_ARGS);
		void onClear(CON_ARGS);
		void onAlias(CON_ARGS);
		void onIf(CON_ARGS);
		void onQuit(CON_ARGS);

	public:
		void setQuitCallback(std::function<void()> value) { mQuitCallback = value; }

	private:
		std::function<void()> mQuitCallback = nullptr;
	};
}
