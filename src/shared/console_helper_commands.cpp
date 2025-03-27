#include "console_helper_commands.h"
#include <imgui.h>
#include <shared/imgui_user.h>
#include <common/console_commands.h>

using namespace Shared;

ConsoleHelperCommands::ConsoleHelperCommands()
{
	sky::AddCVar("con_color", sky::CommandProcessor::CVar(std::nullopt, { "r", "g", "b" },
		[this] {
			auto col = ImGui::User::GetColorFromStyle();
			return std::vector<std::string>({ std::to_string(col.x), std::to_string(col.y), std::to_string(col.z) });
		},
		CVAR_SETTER(ImGui::User::SetupStyleFromColor(CON_ARG_FLOAT(0), CON_ARG_FLOAT(1), CON_ARG_FLOAT(2)))
	));
}

void ConsoleHelperCommands::onFrame()
{
	if (mShowImguiDemo)
	{
		ImGui::ShowDemoWindow();
	}

	if (mShowCVars)
	{
		ImGui::Begin("CVars", nullptr, ImGui::User::ImGuiWindowFlags_ControlPanel);
		ImGui::SetWindowPos(ImGui::User::TopRightCorner());

		auto cvars = sky::GetService<sky::CommandProcessor>()->getItems() | std::views::filter([](const auto& pair) {
			return std::holds_alternative<sky::CommandProcessor::CVar>(pair.second);
		}) | std::views::transform([](const auto& pair) {
			return std::pair{ pair.first, std::get<sky::CommandProcessor::CVar>(pair.second) };
		});

		for (const auto& [name, cvar] : cvars)
		{
			ImGui::Text("%s", name.c_str());

			if (ImGui::IsItemHovered())
			{
				ImGui::BeginTooltip();
				ImGui::Text("%s", cvar.description.value_or("").c_str());
				ImGui::EndTooltip();
			}

			for (auto arg : cvar.arguments)
			{
				ImGui::SameLine();

				if (arg == "bool")
				{
					auto tokens = cvar.getter();
					bool b = stoi(tokens.front());

					if (ImGui::Checkbox(("##" + name).c_str(), &b))
						cvar.setter({ std::to_string(static_cast<int>(b)) });
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