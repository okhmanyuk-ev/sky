#include "system.h"

using namespace Renderer;

void System::draw(size_t vertexCount, size_t vertexOffset)
{
	mDrawcalls += 1;
}

void System::drawIndexed(size_t indexCount, size_t indexOffset, size_t vertexOffset)
{
	mDrawcalls += 1;
}

void System::present()
{
	mDrawcallsPublic = mDrawcalls;
	mDrawcalls = 0;
}