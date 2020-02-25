#include "trail.h"

using namespace Scene;

Trail::Trail(std::weak_ptr<Node> holder) : mHolder(holder)
{
	//
}

void Trail::update()
{
	Node::update();

	auto now = FRAME->getUptime();

	while (!mSegments.empty() && now - mSegments.front().time > Clock::FromSeconds(mLifetime))
		mSegments.pop_front();

	auto holder = mHolder.lock();
	auto pos = holder->unproject(project(getSize() * 0.5f));

	if (!mSegments.empty() && glm::distance(mSegments.rbegin()->pos, pos) < 0.5f)
		return;

	auto segment = Segment();
	segment.pos = pos;
	segment.time = FRAME->getUptime();
	mSegments.push_back(segment);
}

void Trail::draw()
{
	Node::draw();

	if (mSegments.size() < 2)
		return;

	auto vertices = std::vector<Renderer::Vertex::PositionColor>();

	for (int i = 0; i < mSegments.size(); i++)
	{
		bool last = i == mSegments.size() - 1;

		auto p1 = mSegments[i].pos;
		auto p2 = mSegments[last ? i - 1 : i + 1].pos;

		auto perp = glm::normalize(p1 - p2);
		perp = { -perp.y, perp.x };

		float thickness = (getSize().x + getSize().y) / 4.0f; // TODO: bad size effect when parent node was scaled

		perp *= thickness;
		
		auto interp = Common::Easing::Linear((float)i / (float)mSegments.size());

		if (mNarrowing)
			perp *= interp;
		
		auto v1 = glm::vec2(p1.x + perp.x, p1.y + perp.y);
		auto v2 = glm::vec2(p1.x - perp.x, p1.y - perp.y);

		auto color = getColor();
		color.a = 1.0f * interp * getAlpha();

		vertices.push_back({ { last ? v2 : v1, 0.0f }, color });
		vertices.push_back({ { last ? v1 : v2, 0.0f }, color });
	}

	auto holder = mHolder.lock();
	GRAPHICS->draw(Renderer::Topology::TriangleStrip, vertices, holder->getTransform());
}

void Trail::clearTrail()
{
	mSegments.clear();
}