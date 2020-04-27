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

#include <scene/scene.h>
#include <scene/node.h>
#include <scene/label.h>
#include <scene/sprite.h>
#include <scene/sliced_sprite.h>
#include <scene/render_layer.h>
#include <scene/bloom_layer.h>
#include <scene/rectangle.h>
#include <scene/circle.h>
#include <scene/clickable.h>
#include <scene/clippable.h>
#include <scene/debuggable.h>
#include <scene/actionable.h>
#include <scene/ani_sprite.h>
#include <scene/scrollbox.h>
#include <scene/transform.h>
#include <scene/color.h>
#include <scene/trail.h>
#include <scene/cullable.h>

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
#include <shared/format_helpers.h>
#include <shared/scene_editor.h>

#include <graphics/font.h>
#include <graphics/legacy_renderer.h>

#include <renderer/shaders/default.h>
#include <renderer/shaders/light.h>
#include <renderer/shaders/blur.h>
#include <renderer/shaders/sdf.h>
#include <renderer/shaders/circle.h>

#include <network/system.h>