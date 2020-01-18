#include "console_helper_commands.h"
#include <imgui.h>
#include <Shared/imgui_user.h>

#include <Common/console_commands.h>

using namespace Shared;

ConsoleHelperCommands::ConsoleHelperCommands()
{
	CONSOLE->registerCVar("hud_show_cvars", "show cvars menu on screen", { "bool" },
		CVAR_GETTER_BOOL(mWantShowCVars),
		CVAR_SETTER_BOOL(mWantShowCVars));


	CONSOLE->registerCVar("con_color", { "r", "g", "b" },
		[this] {
			auto col = ImGui::User::GetColorFromStyle();
			return std::vector<std::string>({ std::to_string(col.x), std::to_string(col.y), std::to_string(col.z) });
		},
		CVAR_SETTER(
			ImGui::User::SetupStyleFromColor(CON_ARG_FLOAT(0), CON_ARG_FLOAT(1), CON_ARG_FLOAT(2))
		)
	);

	CONSOLE->registerCVar("imgui_show_demo", { "bool" },
		CVAR_GETTER_BOOL(mWantImguiDemo),
		CVAR_SETTER_BOOL(mWantImguiDemo));
}

ConsoleHelperCommands::~ConsoleHelperCommands()
{
	//
}

void ConsoleHelperCommands::frame()
{
	if (mWantImguiDemo)
	{
		ImGui::ShowDemoWindow();
	}

	if (mWantShowCVars)
	{
		ImGui::Begin("CVars", nullptr, ImGui::User::ImGuiWindowFlags_ControlPanel);
		ImGui::SetWindowPos(ImGui::User::TopRightCorner());

		for (auto& [name, cvar] : CONSOLE->getCVars())
		{
			ImGui::Text(name.c_str());

			if (ImGui::IsItemHovered())
			{
				ImGui::BeginTooltip();
				ImGui::Text(cvar.getDescription().c_str());
				ImGui::EndTooltip();
			}

			for (auto arg : cvar.getArguments())
			{
				ImGui::SameLine();

				if (arg == "bool")
				{
					auto tokens = cvar.getGetter()();
					bool b = stoi(tokens.front());

					if (ImGui::Checkbox(("##" + name).c_str(), &b))
						cvar.getSetter()({ std::to_string(static_cast<int>(b)) });
				}
				else if (arg == "int")
				{
					ImGui::Button("awdawd");
				}
			}
		}

		ImGui::End();
	}
}