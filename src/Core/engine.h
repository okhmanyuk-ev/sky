#pragma once

#include <map>
#include <string>

#define ENGINE Core::Engine::Context
#define PLATFORM ENGINE->getPlatform()
#define RENDERER ENGINE->getRenderer()
#define FRAME ENGINE->getFrame()
#define EVENT ENGINE->getEvent()
#define GRAPHICS ENGINE->getGraphics()
#define CONSOLE ENGINE->getConsole()
#define CONSOLE_DEVICE ENGINE->getConsoleDevice()
#define TASK ENGINE->getTask()
#define AUDIO ENGINE->getAudio()
#define NETWORK ENGINE->getNetwork()
#define LOCALIZATION ENGINE->getLocalization()
#define CACHE ENGINE->getCache()
#define STATS ENGINE->getStats()

namespace Common
{
	class FrameSystem;
	class EventSystem;
	class TaskSystem;
}

namespace Console
{
	class System;
	class Device;
}

namespace Platform
{
	class System;
}

namespace Renderer
{
	class System;
}

namespace Graphics
{
	class System;
}

namespace Audio
{
	class System;
}

namespace Network
{
	class System;
}

namespace Shared
{
	class LocalizationSystem;
	class CacheSystem;
	class StatsSystem;
}

namespace Core
{
	class System;
	
	class Engine final
	{
	public:
		static inline Engine* Context;
	
	public:
		Engine()
		{
			Context = this;
		}

	public:
		auto getPlatform() { return mPlatform; }
		void setPlatform(Platform::System* value) { mPlatform = value; }

		auto getRenderer() { return mRenderer; }
		void setRenderer(Renderer::System* value) { mRenderer = value; }

		auto getFrame() { return mFrame; }
		void setFrame(Common::FrameSystem* value) { mFrame = value; }

		auto getEvent() { return mEvent; }
		void setEvent(Common::EventSystem* value) { mEvent = value; }

		auto getGraphics() { return mGraphics; }
		void setGraphics(Graphics::System* value) { mGraphics = value; }

		auto getConsole() { return mConsole; }
		void setConsole(Console::System* value) { mConsole = value; }

		auto getConsoleDevice() { return mConsoleDevice; }
		void setConsoleDevice(Console::Device* value) { mConsoleDevice = value; }

		auto getTask() { return mTask; }
		void setTask(Common::TaskSystem* value) { mTask = value; }

		auto getAudio() { return mAudio; }
		void setAudio(Audio::System* value) { mAudio = value; }

		auto getNetwork() { return mNetwork; }
		void setNetwork(Network::System* value) { mNetwork = value; }

		auto getLocalization() const { return mLocalization; }
		void setLocalization(Shared::LocalizationSystem* value) { mLocalization = value; }

		auto getCache() const { return mCache; }
		void setCache(Shared::CacheSystem* value) { mCache = value; }

		auto getStats() const { return mStats; }
		void setStats(Shared::StatsSystem* value) { mStats = value; }

		auto getCustomSystem() const { return mCustomSystem; }
		void setCustomSystem(void* value) { mCustomSystem = value; }

	private:
		Platform::System* mPlatform = nullptr;
		Renderer::System* mRenderer = nullptr;
		Common::FrameSystem* mFrame = nullptr;
		Common::EventSystem* mEvent = nullptr;
		Graphics::System* mGraphics = nullptr;
		Console::System* mConsole = nullptr;
		Console::Device* mConsoleDevice = nullptr;
		Common::TaskSystem* mTask = nullptr;
		Audio::System* mAudio = nullptr;
		Network::System* mNetwork = nullptr;
		Shared::LocalizationSystem* mLocalization = nullptr;
		Shared::CacheSystem* mCache = nullptr;
		Shared::StatsSystem* mStats = nullptr;
		void* mCustomSystem = nullptr;
	};
}