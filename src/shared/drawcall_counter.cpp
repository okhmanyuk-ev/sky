#include "drawcall_counter.h"

using namespace Shared;

void DrawCallCounter::onFrame()
{
	mDrawCalls = mDrawCallsInternal;
	mDrawCallsInternal = 0;
}

void DrawCallCounter::onEvent(const Shared::RendererDebugDrawCallEvent& e)
{
	mDrawCallsInternal++;
}