#pragma once

#include <Core/engine.h>
#include <Core/clock.h>
#include <Shared/renderer_debug.h>
#include <Common/event_system.h>
#include <Common/timer.h>

namespace Shared
{
	class DrawCallCounter : public Common::FrameSystem::Frameable,
		public Common::EventSystem::Listenable<Shared::RendererDebugDrawCallEvent>
	{
	private:
		void frame() override;
		void event(const Shared::RendererDebugDrawCallEvent& e) override;

	public:
		auto getDrawCalls() const { return mDrawCalls; }
		
	private:
		int mDrawCallsInternal = 0;
		int mDrawCalls = 0;
	};
}