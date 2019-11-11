#pragma once

#include <glm/glm.hpp>
#include <glm/ext.hpp>

#include <nlohmann/json.hpp>

#include <Common/timer.h>
#include <Common/easing.h>
#include <Common/hash.h>
#include <Common/actions.h>
#include <Common/frame_system.h>

#include <Audio/sound.h>

#include <Platform/defines.h>
#include <Platform/mouse.h>

#if defined(PLATFORM_WINDOWS)
#include <Platform/system_windows.h>
#endif

#include <Scene/scene.h>
#include <Scene/node.h>
#include <Scene/label.h>
#include <Scene/render_layer.h>
#include <Scene/bloom_layer.h>
#include <Scene/sprite.h>
#include <Scene/rectangle.h>
#include <Scene/circle.h>
#include <Scene/pickable.h>
#include <Scene/clickable.h>
#include <Scene/clippable.h>
#include <Scene/debuggable.h>
#include <Scene/scrollable.h>
#include <Scene/actionable.h>
#include <Scene/ani_sprite.h>
#include <Scene/scrollbox.h>
#include <Scene/transform.h>
#include <Scene/color.h>
#include <Scene/trail.h>

#include <Shared/stats_system.h>
#include <Shared/cache_system.h>
#include <Shared/rich_application.h>
#include <Shared/graphical_application.h>
#include <Shared/localization_system.h>
#include <Shared/graphics_helpers.h>
#include <Shared/common_actions.h>
#include <Shared/first_person_camera_controller.h>
#include <Shared/imgui_user.h>
#include <Shared/renderer_debug.h>
#include <Shared/touch_emulator.h>
#include <Shared/gesture_detector.h>
#include <Shared/scene_helpers.h>
#include <Shared/scene_manager.h>

#include <Graphics/font.h>
#include <Graphics/legacy_renderer.h>
#include <Graphics/embedded_font.h>
#include <Graphics/embedded_texture.h>

#include <Renderer/shader_default.h>
#include <Renderer/shader_light.h>
#include <Renderer/shader_blur.h>

#include <Network/system.h>