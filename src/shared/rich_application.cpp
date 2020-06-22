#include "rich_application.h"

#include <imgui.h>
#include <shared/imgui_user.h>
#include <common/easing.h>
#include <shared/action_helpers.h>

using namespace Shared;

RichApplication::RichApplication(const std::string& appname) : GraphicalApplication(appname)
{
	ENGINE->addSystem<Audio::System>(std::make_shared<Audio::System>());
}