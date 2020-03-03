#include "stats_system.h"

#include <imgui.h>
#include <platform/system.h>
#include <glm/glm.hpp>

using namespace Shared;

StatsSystem::StatsSystem()
{
	// 
}

StatsSystem::~StatsSystem()
{
	//
}

void StatsSystem::frame()
{
	if (!mEnabled)
		return;

	if (mGroups.empty())
		return;

	ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowMinSize, ImVec2(0, 0));

	ImGui::Begin("Statistics", nullptr,
		ImGuiWindowFlags_NoInputs |
		ImGuiWindowFlags_NoDecoration |
		ImGuiWindowFlags_NoNav );

	ImGui::Columns(2, nullptr, false);

	float key_width = 0.0f;
	float value_width = 0.0f;

	auto& style = ImGui::GetStyle();
	ImVec4 col = style.Colors[ImGuiCol_Text];

	auto now = FRAME->getUptime();

	const float MaxLifetime = 5.0f;

	bool needSeparator = false;

	for (const auto& [group, indicators] : mGroups)
	{
		if (needSeparator)
			ImGui::Separator();
		else
			needSeparator = true;

		for (const auto& [key, value] : indicators)
		{
			auto lifetime = Clock::ToSeconds(now - value.time);

			if (lifetime >= MaxLifetime)
				continue;

			auto alpha = 1.0f - glm::smoothstep(0.0f, MaxLifetime, lifetime);
			auto c = col;

			c.w *= alpha;

			key_width = glm::max(key_width, ImGui::CalcTextSize(key.c_str()).x);
			value_width = glm::max(value_width, ImGui::CalcTextSize(value.text.c_str()).x);
			ImGui::TextColored(c, key.c_str());
			ImGui::NextColumn();
			ImGui::TextColored(c, value.text.c_str());
			ImGui::NextColumn();
		}
	}

	float key_column_width = key_width + (style.WindowPadding.x * 2.0f);
	float value_column_width = value_width + (style.WindowPadding.x * 2.0f);

	ImGui::SetColumnWidth(0, key_column_width);
	ImGui::SetColumnWidth(1, value_column_width);
	
	ImGui::SetWindowSize(ImVec2(key_column_width + value_column_width, 0.0f));

	if (mAlignment == Align::TopLeft)
		ImGui::SetWindowPos(ImVec2(0, 0));
	else if (mAlignment == Align::TopRight)
		ImGui::SetWindowPos(ImVec2(PLATFORM->getLogicalWidth() - ImGui::GetWindowWidth(), 0.0f));
	else if (mAlignment == Align::BottomLeft)
		ImGui::SetWindowPos(ImVec2(0.0f, PLATFORM->getLogicalHeight() - ImGui::GetWindowHeight()));
	else if (mAlignment == Align::BottomRight)
		ImGui::SetWindowPos(ImVec2(PLATFORM->getLogicalWidth() - ImGui::GetWindowWidth(), PLATFORM->getLogicalHeight() - ImGui::GetWindowHeight()));


	ImGui::End();
	ImGui::PopStyleVar(2);

	for (auto group_it = mGroups.begin(); group_it != mGroups.end();)
	{
		for (auto ind_it = group_it->second.begin(); ind_it != group_it->second.end();)
		{
			auto lifetime = Clock::ToSeconds(now - ind_it->second.time);

			if (lifetime >= MaxLifetime)
				ind_it = group_it->second.erase(ind_it);
			else
				ind_it++;
		}

		if (group_it->second.empty())
			group_it = mGroups.erase(group_it);
		else
			group_it++;
	}
}

void StatsSystem::indicate(const std::string& key, const std::string& value, const std::string& group)
{
	mGroups[group][key] = { value, FRAME->getUptime() };
}

void StatsSystem::indicate(const std::string& key, int value, const std::string& group)
{
	indicate(key, std::to_string(value), group);
}

void StatsSystem::indicate(const std::string& key, float value, const std::string& group)
{
	indicate(key, std::to_string(value), group);
}

void StatsSystem::indicate(const std::string& key, size_t value, const std::string& group)
{
	indicate(key, std::to_string(value), group);
}