#include "imgui_user.h"
#include <core/engine.h>
#include <platform/system.h>
#include <shared/imgui_system.h>

namespace ImGui::User
{
	ImVec4 GetColorFromStyle()
	{
		return ImGui::GetStyle().Colors[ImGuiCol_Text];
	}

	void SetupStyleFromColor(float r, float g, float b)
	{
		auto color = [r, g ,b](float a) {
			return ImVec4(r, g, b, a);
		};

		auto black = [](float a) {
			return ImVec4(0.0f, 0.0f, 0.0f, a);
		};

		const float Full = 1.0f;
		const float Half = Full / 2.0f;
		const float Quad = Half / 2.0f;
		const float Bit = Quad / 2.0f;
		const float VeryBit = Bit / 2.0f;
		const float Zero = 0.0f;

		ImVec4* colors = ImGui::GetStyle().Colors;
		
		colors[ImGuiCol_Text] = color(Full);
		colors[ImGuiCol_TextDisabled] = color(Half);
		
		colors[ImGuiCol_WindowBg] = black(Half + Quad);
		colors[ImGuiCol_ChildBg] = color(Zero);
		colors[ImGuiCol_PopupBg] = black(Half + Quad);
		
		colors[ImGuiCol_Border] = color(Half + Bit);
		colors[ImGuiCol_BorderShadow] = black(Zero);
		
		colors[ImGuiCol_FrameBg] = color(Quad);
		colors[ImGuiCol_FrameBgHovered] = color(Quad + Bit);
		colors[ImGuiCol_FrameBgActive] = color(Half + Bit);
		
		colors[ImGuiCol_TitleBg] = color(Quad);
		colors[ImGuiCol_TitleBgActive] = color(Quad + VeryBit);
		colors[ImGuiCol_TitleBgCollapsed] = color(Bit);

		colors[ImGuiCol_MenuBarBg] = color(Bit);
		colors[ImGuiCol_ScrollbarBg] = color(Bit);
		
		colors[ImGuiCol_ScrollbarGrab] = color(Quad + Bit);
		colors[ImGuiCol_ScrollbarGrabHovered] = color(Half);
		colors[ImGuiCol_ScrollbarGrabActive] = color(Half + Bit);
		
		colors[ImGuiCol_CheckMark] = color(Half + Quad);
		
		colors[ImGuiCol_SliderGrab] = color(Quad + Bit);
		colors[ImGuiCol_SliderGrabActive] = color(Half);

		colors[ImGuiCol_Button] = color(Quad);
		colors[ImGuiCol_ButtonHovered] = color(Half);
		colors[ImGuiCol_ButtonActive] = color(Half + Bit);

		colors[ImGuiCol_Header] = color(Quad + Bit);
		colors[ImGuiCol_HeaderHovered] = color(Half);
		colors[ImGuiCol_HeaderActive] = color(Half + Bit);

		colors[ImGuiCol_Separator] = ImVec4(0.43f, 0.43f, 0.50f, 0.50f);
		colors[ImGuiCol_SeparatorHovered] = ImVec4(0.10f, 0.40f, 0.75f, 0.78f);
		colors[ImGuiCol_SeparatorActive] = ImVec4(0.10f, 0.40f, 0.75f, 1.00f);

		colors[ImGuiCol_ResizeGrip] = color(Quad + Bit);
		colors[ImGuiCol_ResizeGripHovered] = color(Half);
		colors[ImGuiCol_ResizeGripActive] = color(Half + Quad);

		colors[ImGuiCol_Tab] = color(Quad);
		colors[ImGuiCol_TabHovered] = color(Half + Bit);
		colors[ImGuiCol_TabActive] = color(Half);
		colors[ImGuiCol_TabUnfocused] = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
		colors[ImGuiCol_TabUnfocusedActive] = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);

		colors[ImGuiCol_PlotLines] = color(Half + Bit);
		colors[ImGuiCol_PlotLinesHovered] = color(Half + Quad + Bit);
		colors[ImGuiCol_PlotHistogram] = color(Half);
		colors[ImGuiCol_PlotHistogramHovered] = color(Half + Quad + Bit);
		colors[ImGuiCol_TextSelectedBg] = black(Full);

		colors[ImGuiCol_DragDropTarget] = ImVec4(1.00f, 1.00f, 0.00f, 0.90f);
		colors[ImGuiCol_NavHighlight] = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
		colors[ImGuiCol_NavWindowingHighlight] = ImVec4(1.00f, 1.00f, 1.00f, 0.70f);
		colors[ImGuiCol_NavWindowingDimBg] = ImVec4(0.80f, 0.80f, 0.80f, 0.20f);
		colors[ImGuiCol_ModalWindowDimBg] = ImVec4(0.80f, 0.80f, 0.80f, 0.35f);
	}

	ImVec2 TopLeftCorner(float margin)
	{ 
		return ImVec2(margin + (PLATFORM->getSafeAreaLeftMargin() / PLATFORM->getScale() / IMGUI_SYSTEM->getScale()),
            margin + (PLATFORM->getSafeAreaTopMargin() / PLATFORM->getScale() / IMGUI_SYSTEM->getScale()));
	}

	ImVec2 TopRightCorner(float margin)
	{ 
		return ImVec2(IMGUI_SYSTEM->getLogicalWidth() - GetWindowWidth() - margin - (PLATFORM->getSafeAreaRightMargin() / PLATFORM->getScale() / IMGUI_SYSTEM->getScale()),
            margin + (PLATFORM->getSafeAreaTopMargin() / PLATFORM->getScale() / IMGUI_SYSTEM->getScale()));
	}

	ImVec2 BottomLeftCorner(float margin)
	{
		return ImVec2(margin + (PLATFORM->getSafeAreaLeftMargin() / PLATFORM->getScale() / IMGUI_SYSTEM->getScale()),
            IMGUI_SYSTEM->getLogicalHeight() - GetWindowHeight() - margin - (PLATFORM->getSafeAreaBottomMargin() / PLATFORM->getScale() / IMGUI_SYSTEM->getScale()));
	}

	ImVec2 BottomRightCorner(float margin)
	{
		return ImVec2(IMGUI_SYSTEM->getLogicalWidth() - GetWindowWidth() - margin - (PLATFORM->getSafeAreaRightMargin() / PLATFORM->getScale() / IMGUI_SYSTEM->getScale()),
            IMGUI_SYSTEM->getLogicalHeight() - GetWindowHeight() - margin - (PLATFORM->getSafeAreaBottomMargin() / PLATFORM->getScale() / IMGUI_SYSTEM->getScale()));
	}
}
