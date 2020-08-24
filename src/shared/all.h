#pragma once

#include <tinyutf8.hpp>

#include <fmt/format.h>
#include <fmt/chrono.h>

#include <glm/glm.hpp>
#include <glm/ext.hpp>

#include <nlohmann/json.hpp>

#include <common/timer.h>
#include <common/easing.h>
#include <common/hash.h>
#include <common/actions.h>
#include <common/frame_system.h>

#include <audio/sound.h>

#include <platform/defines.h>
#include <platform/mouse.h>

#if defined(PLATFORM_WINDOWS)
#include <platform/system_windows.h>
#endif

#include <renderer/all.h>
#include <scene/all.h>
#include <imscene/all.h>

#include <shared/stats_system.h>
#include <shared/cache_system.h>
#include <shared/rich_application.h>
#include <shared/graphical_application.h>
#include <shared/localization_system.h>
#include <shared/graphics_helpers.h>
#include <shared/action_helpers.h>
#include <shared/first_person_camera_controller.h>
#include <shared/imgui_user.h>
#include <shared/renderer_debug.h>
#include <shared/touch_emulator.h>
#include <shared/gesture_detector.h>
#include <shared/scene_helpers.h>
#include <shared/scene_manager.h>
#include <shared/scene_editor.h>
#include <shared/scene_stylebook.h>
#include <shared/format_helpers.h>
#include <shared/profile.h>

#include <graphics/font.h>
#include <graphics/legacy_renderer.h>

#include <network/system.h>
