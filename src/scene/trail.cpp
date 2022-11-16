#include "trail.h"

using namespace Scene;

Trail::Trail(std::weak_ptr<Node> holder) : mHolder(holder)
{
	//
}

void Trail::update(Clock::Duration dTime)
{
	Node::update(dTime);
	mUptime += dTime;
}

void Trail::updateTransform()
{
	Node::updateTransform();

	auto now = mUptime;

	auto isSegmentOutdated = [now, this](size_t index) {
		return now - mSegments.at(index).time > Clock::FromSeconds(mLifetime);
	};

	while (!mSegments.empty())
	{
		if (!isSegmentOutdated(0))
			break;

		bool should_delete = true;

		if (mSegments.size() > 1)
			should_delete = isSegmentOutdated(1);

		if (should_delete)
		{
			mSegments.pop_front();
			continue;
		}

		auto& segment = mSegments[0];
		const auto& prev = mSegments[1];

		auto lifetime = mLifetime;
		auto segment_uptime = Clock::ToSeconds(now - segment.time);
		auto prev_uptime = Clock::ToSeconds(now - prev.time);

		auto percent =  (segment_uptime - lifetime) / (segment_uptime - prev_uptime);

		auto fixed_pos = glm::lerp(segment.pos, prev.pos, percent);

		segment.pos = fixed_pos;
		segment.time = now - Clock::FromSeconds(lifetime);
	}

	auto holder = mHolder.lock();
	auto pos = holder->unproject(project(getAbsoluteSize() * 0.5f));

	auto addSegment = [this, now](const auto& pos) {
		auto segment = Segment();
		segment.pos = pos;
		segment.time = now;
		mSegments.push_back(segment);
	};

	if (mSegments.empty())
		addSegment(pos);

	auto last_pos = mSegments.rbegin()->pos;
	auto last_time = mSegments.rbegin()->time;

	int added_count = 0;

	while (glm::distance(last_pos, pos) >= 0.5f)
	{
		auto dir = glm::normalize(pos - last_pos);
		last_pos += 0.5f * dir;
		addSegment(last_pos);
		added_count += 1;
	}

	auto this_frame_duration = Clock::ToSeconds(now - last_time);

	for (int i = 0; i < added_count; i++)
	{
		auto index = mSegments.size() - added_count + i;
		auto& segment = mSegments.at(index);
		auto fix_percent = (float)(i + 1) / (float)added_count;
		auto duration_fix = glm::lerp(this_frame_duration, 0.0f, fix_percent);
		segment.time -= Clock::FromSeconds(duration_fix);
	}
}

void Trail::draw()
{
	Node::draw();

	if (mSegments.size() < 2)
		return;

	auto vertices = std::vector<skygfx::Vertex::PositionColor>();

	for (int i = (int)mSegments.size() - 1; i >= 0; i--)
	{
		bool last = i == mSegments.size() - 1;

		const auto& s1 = mSegments.at(i);
		const auto& s2 = mSegments.at(last ? i - 1 : i + 1);

		auto perp = glm::normalize(s1.pos - s2.pos); // perpendicular
		perp = { -perp.y, perp.x };

		float thickness = (getAbsoluteSize().x + getAbsoluteSize().y) / 4.0f; // TODO: bad size effect when parent node was scaled

		perp *= thickness;
		
		auto interp = Easing::Linear((float)i / (float)mSegments.size());
		
		if (mNarrowing)
			perp *= interp;
		
		auto v1 = s1.pos + perp;
		auto v2 = s1.pos - perp;

		auto color = glm::lerp(mEndColor, mBeginColor, interp);
		color *= getColor();
		
		vertices.push_back({ { last ? v2 : v1, 0.0f }, color });
		vertices.push_back({ { last ? v1 : v2, 0.0f }, color });
	}

	auto holder = mHolder.lock();

	auto stencil = skygfx::StencilMode();
	stencil.write_mask = 255;
	stencil.read_mask = 255;
	stencil.func = skygfx::ComparisonFunc::Greater;
	stencil.depth_fail_op = skygfx::StencilOp::Increment;
	stencil.fail_op = skygfx::StencilOp::Increment;
	stencil.pass_op = skygfx::StencilOp::Increment;

	GRAPHICS->pushStencilMode(stencil);
	GRAPHICS->pushModelMatrix(holder->getTransform());
	GRAPHICS->draw(skygfx::Topology::TriangleStrip, vertices);
	GRAPHICS->pop(2);
}

void Trail::clearTrail()
{
	mSegments.clear();
}
