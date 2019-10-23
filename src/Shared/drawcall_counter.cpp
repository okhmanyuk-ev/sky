#include "drawcall_counter.h"

using namespace Shared;

void DrawCallCounter::frame()
{
	mDrawCalls = mDrawCallsInternal;
	mDrawCallsInternal = 0;
}

void DrawCallCounter::event(const Shared::RendererDebugDrawCallEvent& e)
{
	mDrawCallsInternal++;
}