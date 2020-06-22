#pragma once

#include <core/engine.h>

#include <common/task_system.h>
#include <common/frame_system.h>
#include <common/event_system.h>

#include <shared/console_device.h>
#include <console/system.h>

#include <common/console_commands.h>
#include <shared/graphical_console_commands.h>
#include <shared/performance_console_commands.h>
#include <shared/console_helper_commands.h>
#include <shared/cache_system.h>

#include <shared/imgui_system.h>

#include <platform/system.h>
#include <renderer/system.h>

#include <graphics/system.h>
#include <audio/system.h>
#include <vector>
#include <shared/renderer_debug.h>
#include <renderer/system_d3d11.h>
#include <renderer/system_gl.h>

#include <shared/graphical_application.h>

namespace Shared
{
	class RichApplication : public GraphicalApplication
	{
	public:
		RichApplication(const std::string& appname);
	};
}