#pragma once

#include <fmt/format.h>
#include <fmt/chrono.h>

#include <glm/glm.hpp>
#include <glm/ext.hpp>

#include <imgui.h>
#include <imgui_stdlib.h>

#include <nlohmann/json.hpp>

#include <tiny_obj_loader.h>
#include <magic_enum.hpp>

#include <common/easing.h>
#include <common/hash.h>
#include <common/actions.h>
#include <common/helpers.h>
#include <common/bitbuffer.h>
#include <common/buffer_helpers.h>

#include <platform/all.h>
#include <scene/all.h>
#include <graphics/all.h>

#include <shared/stats_system.h>
#include <shared/first_person_camera_controller.h>
#include <shared/imgui_user.h>
#include <shared/imscene.h>
#include <shared/touch_emulator.h>
#include <shared/gesture_detector.h>
#include <shared/scene_helpers.h>
#include <shared/scene_manager.h>
#include <shared/scene_editor.h>
#include <shared/scene_stylebook.h>
#include <shared/profile.h>
#include <shared/networking_ws.h>
#include <shared/networking_udp.h>
#include <shared/phys_helpers.h>

#include <network/system.h>
