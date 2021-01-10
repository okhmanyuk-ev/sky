#pragma once

#include <core/engine.h>

#include <console/system.h>
#include <common/actions.h>

#define CVAR_GETTER(V) [this] { return std::vector<std::string>({ V }); }
#define CVAR_GETTER2(V1, V2) [this] { return std::vector<std::string>({ V1, V2 }); }
#define CVAR_GETTER3(V1, V2, V3) [this] { return std::vector<std::string>({ V1, V2, V3 }); }

#define CVAR_GETTER_INT(V) CVAR_GETTER(std::to_string(V))
#define CVAR_GETTER_INT2(V1, V2) CVAR_GETTER2(std::to_string(V1), std::to_string(V2))
#define CVAR_GETTER_INT3(V1, V2, V3) CVAR_GETTER3(std::to_string(V1), std::to_string(V2), std::to_string(V3))
#define CVAR_GETTER_INT_FUNC(V) CVAR_GETTER_INT(V())
#define CVAR_GETTER_INT2_FUNC(V1, V2) CVAR_GETTER_INT2(V1(), V2())

#define CVAR_GETTER_BOOL(V) CVAR_GETTER(std::to_string(V))
#define CVAR_GETTER_BOOL2(V1, V2) CVAR_GETTER2(std::to_string(V1), std::to_string(V2))
#define CVAR_GETTER_BOOL3(V1, V2, V3) CVAR_GETTER3(std::to_string(V1), std::to_string(V2), std::to_string(V3))
#define CVAR_GETTER_BOOL_FUNC(V) CVAR_GETTER_BOOL(V())

#define CVAR_GETTER_FLOAT(V) CVAR_GETTER(std::to_string(V))
#define CVAR_GETTER_FLOAT2(V1, V2) CVAR_GETTER2(std::to_string(V1), std::to_string(V2))
#define CVAR_GETTER_FLOAT3(V1, V2, V3) CVAR_GETTER3(std::to_string(V1), std::to_string(V2), std::to_string(V3))
#define CVAR_GETTER_FLOAT_FUNC(V) CVAR_GETTER_FLOAT(V())

#define CVAR_GETTER_DOUBLE(V) CVAR_GETTER(std::to_string(V))
#define CVAR_GETTER_DOUBLE2(V1, V2) CVAR_GETTER2(std::to_string(V1), std::to_string(V2))
#define CVAR_GETTER_DOUBLE3(V1, V2, V3) CVAR_GETTER3(std::to_string(V1), std::to_string(V2), std::to_string(V3))
#define CVAR_GETTER_DOUBLE_FUNC(V) CVAR_GETTER_DOUBLE(V())

#define CON_ARGS_NAME args
#define CON_ARGS const std::vector<std::string>& CON_ARGS_NAME
#define CON_ARGS_COUNT CON_ARGS_NAME.size()
#define CON_HAS_ARGS !CON_ARGS_NAME.empty()
#define CON_ARGS_ACCUMULATED_STRING CON_HAS_ARGS ? std::accumulate(std::next(CON_ARGS_NAME.begin()), CON_ARGS_NAME.end(), *CON_ARGS_NAME.begin(), [](const auto& a, const auto& b) { return a + " " + b; }) : ""

#define CON_ARG(N) CON_ARGS_NAME[N]
#define CON_ARG_INT(N) stoi(CON_ARG(N))
#define CON_ARG_BOOL(N) stoi(CON_ARG(N))
#define CON_ARG_FLOAT(N) stof(CON_ARG(N))
#define CON_ARG_DOUBLE(N) stod(CON_ARG(N))

#define CVAR_SETTER(V) [this](CON_ARGS) { try { V; } catch (const std::exception& e) { CONSOLE_DEVICE->writeLine(e.what()); } }

#define CVAR_SETTER_FLOAT(V) CVAR_SETTER(V = CON_ARG_FLOAT(0))
#define CVAR_SETTER_FLOAT_FUNC(V) CVAR_SETTER(V(CON_ARG_FLOAT(0)))

#define CVAR_SETTER_DOUBLE(V) CVAR_SETTER(V = CON_ARG_DOUBLE(0))
#define CVAR_SETTER_DOUBLE_FUNC(V) CVAR_SETTER(V(CON_ARG_DOUBLE(0)))

#define CVAR_SETTER_BOOL(V) CVAR_SETTER(V = CON_ARG_BOOL(0))
#define CVAR_SETTER_BOOL_FUNC(V) CVAR_SETTER(V(CON_ARG_BOOL(0)))

#define CVAR_SETTER_INT(V) CVAR_SETTER(V = CON_ARG_INT(0))
#define CVAR_SETTER_INT_FUNC(V) CVAR_SETTER(V(CON_ARG_INT(0)))
#define CVAR_SETTER_INT2_FUNC(V) CVAR_SETTER(V(CON_ARG_INT(0), CON_ARG_INT(1)))

#define CMD_METHOD(V) [this](CON_ARGS) { V(CON_ARGS_NAME); }
#define CMD_METHOD_NO_ARGS(V) [this](CON_ARGS) { V(); }

namespace Common
{
	class ConsoleCommands
	{
	public:
		ConsoleCommands();
		~ConsoleCommands();
		
	private:
		void onCmdList(CON_ARGS);
		void onCVarList(CON_ARGS);
		void onEcho(CON_ARGS);
		void onDelay(CON_ARGS);
		void onExec(CON_ARGS);
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