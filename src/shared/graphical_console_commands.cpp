#include "graphical_console_commands.h"
#include <sky/imgui.h>
#include <shared/scene_editor.h>
#include <sky/console.h>
#include <sky/utils.h>
#include <common/console_commands.h>
#include <sky/renderer.h>
#include <magic_enum/magic_enum.hpp>

using namespace Shared;

GraphicalConsoleCommands::GraphicalConsoleCommands()
{
	sky::AddCVar("r_resolution", sky::CVar<std::tuple<int, int>>::CreateDefinition(
		[] { return std::tuple{ PLATFORM->getWidth(), PLATFORM->getHeight() }; },
		[](std::tuple<int, int> value) { PLATFORM->resize(std::get<0>(value), std::get<1>(value)); }, "resolution of screen"));

	std::string available_apis;
	for (auto type : magic_enum::enum_values<skygfx::BackendType>())
	{
		if (!skygfx::GetAvailableBackends().contains(type))
			continue;

		if (!available_apis.empty())
			available_apis += ", ";

		available_apis += magic_enum::enum_name(type);
	}

	sky::AddCVar("r_api", sky::CVar<std::string>::CreateDefinition(
		[] { return std::string(magic_enum::enum_name(skygfx::GetBackendType())); },
		[available_apis](std::string value) {
			auto type = magic_enum::enum_cast<skygfx::BackendType>(value, magic_enum::case_insensitive);

			if (!type.has_value() || !skygfx::GetAvailableBackends().contains(type.value()))
			{
				sky::Log(sky::Console::Color::Red, "unknown render api '{}', available: {}", value, available_apis);
				return;
			}

			sky::Renderer::Instance->setBackendType(type.value());
		}, "render api (" + available_apis + "), switches at the end of frame"));

	sky::AddCommand("rescale", "smart scaling", { "float" }, {}, {}, [](float value) {
		PLATFORM->rescale(value);
	});
}

void GraphicalConsoleCommands::onFrame()
{
	if (mShowTargets.getValue())
	{
		ImGui::Begin("Render Targets");
		for (const auto& [name, target] : GRAPHICS->getRenderTargets())
		{
			ImGui::Text("%s", name.c_str());
			ImGui::Text("%dx%d", target->getWidth(), target->getHeight());
			auto width = ImGui::GetContentRegionAvail().x;
			SceneEditor::drawImage(target, std::nullopt, width);
			ImGui::Separator();
		}
		ImGui::End();
	}
}
