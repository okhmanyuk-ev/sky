#pragma once

#include <core/engine.h>
#include <core/clock.h>
#include <shared/renderer_debug.h>
#include <common/event_system.h>
#include <common/timer.h>

namespace Shared
{
	class DrawCallCounter : public Common::FrameSystem::Frameable,
		public Common::Event::Listenable<Shared::RendererDebugDrawCallEvent>
	{
	private:
		void onFrame() override;
		void onEvent(const Shared::RendererDebugDrawCallEvent& e) override;

	public:
		auto getDrawCalls() const { return mDrawCalls; }
		
	private:
		int mDrawCallsInternal = 0;
		int mDrawCalls = 0;
	};
}