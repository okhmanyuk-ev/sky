#include "trail.h"

using namespace Scene;

Trail::Trail(std::weak_ptr<Node> holder) : mHolder(holder)
{
	//
}

void Trail::update()
{
	Node::update();

	auto holder = mHolder.lock();

	auto segment = Segment();
	segment.pos = holder->unproject(project(getSize() * 0.5f));
	segment.time = Clock::Now();
	mSegments.push_back(segment);
}

void Trail::draw()
{
	Node::draw();

	auto holder = mHolder.lock();

	auto now = Clock::Now();

	while (!mSegments.empty() && now - mSegments.front().time > Clock::FromSeconds(mLifetime) / FRAME->getTimeScale())
		mSegments.pop_front();

	if (mSegments.size() < 2)
		return;
	
	auto vertices = std::vector<Renderer::Vertex::PositionColor>();

	for (int i = 0; i < mSegments.size(); i++)
	{
		auto p1 = glm::vec2();
		auto p2 = glm::vec2();

		bool last = i == mSegments.size() - 1;

		p1 = mSegments[i].pos;
		p2 = mSegments[last ? i - 1 : i + 1].pos;

		auto perp = glm::normalize(p1 - p2);
		perp = { -perp.y, perp.x };

		float thickness = (getSize().x + getSize().y) / 4.0f;

		perp *= thickness;
		
		auto interp = Common::Easing::Linear(((float)i / (float)mSegments.size()));

		if (mNarrowing)
			perp *= interp;
		
		auto v1 = glm::vec2(p1.x + perp.x, p1.y + perp.y);
		auto v2 = glm::vec2(p1.x - perp.x, p1.y - perp.y);

		auto alpha = 1.0f * interp * getAlpha();
		auto color = getColor();

		vertices.push_back({ { last ? v2 : v1, 0.0f }, { color.r, color.g, color.b, alpha } });
		vertices.push_back({ { last ? v1 : v2, 0.0f }, { color.r, color.g, color.b, alpha } });
	}

	GRAPHICS->draw(Renderer::Topology::TriangleStrip, vertices, holder->getTransform());
}

void Trail::clearTrail()
{
	mSegments.clear();
}