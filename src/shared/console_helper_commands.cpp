#include "console_helper_commands.h"
#include <imgui.h>
#include <shared/imgui_user.h>
#include <common/profiler_system.h>
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

	if (mShowProfiler)
	{
		ImGui::Begin("Profiler");

		std::function<void(Common::ProfilerSystem::Node*)> show = [&show](Common::ProfilerSystem::Node* node) {
			int flags = ImGuiTreeNodeFlags_DefaultOpen;

			if (node->getNodes().size() == 0)
				flags |= ImGuiTreeNodeFlags_Leaf;

			int percentage = int(node->getPercentage() * 100.0f); // TODO: make relative percentage ?

																  // TODO: percent must be right aligned

			bool opened = ImGui::TreeNodeEx(node->getName().c_str(), flags, "%s - %d%%", node->getName().c_str(),
				percentage > 100 ? 100 : percentage);

			if (ImGui::IsItemHovered())
			{
				ImGui::BeginTooltip();
				ImGui::Text("Name: %s", node->getName().c_str());
				ImGui::Text("Count: %d", node->getCount());
				ImGui::Text("Frame: %.3f msec", sky::ToSeconds(node->getDuration()) * 1000.0f);
				ImGui::Text("Total: %.3f msec", sky::ToSeconds(node->getDuration() * node->getCount()) * 1000.0f);
				ImGui::EndTooltip();
			}

			if (opened)
			{
				for (auto n : node->getNodes())
					show(n);

				ImGui::TreePop();
			}
		};

		for (auto node : PROFILER->getNodes())
			show(node);

		ImGui::End();
	}
}