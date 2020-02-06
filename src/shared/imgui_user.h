#pragma once

#define IMGUI_DISABLE_OBSOLETE_FUNCTIONS

#include <imgui.h>

namespace ImGui::User
{
	const int ImGuiWindowFlags_Overlay = ImGuiWindowFlags_NoTitleBar |
		ImGuiWindowFlags_NoResize |
		ImGuiWindowFlags_NoMove |
		ImGuiWindowFlags_NoScrollbar |
		ImGuiWindowFlags_NoScrollWithMouse |
		ImGuiWindowFlags_NoCollapse |
		ImGuiWindowFlags_AlwaysAutoResize |
		ImGuiWindowFlags_NoSavedSettings |
		ImGuiWindowFlags_NoInputs |
		ImGuiWindowFlags_NoFocusOnAppearing |
		ImGuiWindowFlags_NoBringToFrontOnFocus;

	const int ImGuiWindowFlags_ControlPanel = ImGuiWindowFlags_NoTitleBar |
		ImGuiWindowFlags_NoResize |
		ImGuiWindowFlags_NoMove |
		ImGuiWindowFlags_NoFocusOnAppearing |
		ImGuiWindowFlags_NoBringToFrontOnFocus |
		ImGuiWindowFlags_AlwaysAutoResize;

	ImVec4 GetColorFromStyle();
	void SetupStyleFromColor(float r, float g, float b);

	ImVec2 TopLeftCorner(float margin = 10.0f);
	ImVec2 TopRightCorner(float margin = 10.0f);

	ImVec2 BottomLeftCorner(float margin = 10.0f);
	ImVec2 BottomRightCorner(float margin = 10.0f);
}