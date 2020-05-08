#pragma once

#include <scene/node.h>
#include <scene/color.h>
#include <common/easing.h>

namespace Scene
{
	class Trail : public Node, public Color
	{
	public:
		Trail(std::weak_ptr<Node> holder);

	public:
		void updateTransform() override;
	
	protected:
		void draw() override;

	public:
		void clearTrail();

	public:
		auto getLifetime() const { return mLifetime; }
		void setLifetime(float value) { mLifetime = value; }

		auto getNarrowing() const { return mNarrowing; }
		void setNarrowing(bool value) { mNarrowing = value; }

		auto getSegmentsCount() const { return mSegments.size(); }

	private:
		float mLifetime = 1.0f;
		bool mNarrowing = false;
		std::weak_ptr<Node> mHolder;

		struct Segment
		{
			glm::vec2 pos = { 0.0f, 0.0f };
			Clock::Duration time;
		};

		std::deque<Segment> mSegments;
	};
}
