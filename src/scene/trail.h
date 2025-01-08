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
		void update(sky::Duration dTime) override;
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

		auto getBeginColor() const { return mBeginColor; }
		void setBeginColor(const glm::vec4& value) { mBeginColor = value; }

		auto getEndColor() const { return mEndColor; }
		void setEndColor(const glm::vec4& value) { mEndColor = value; }

	private:
		float mLifetime = 1.0f;
		bool mNarrowing = false;
		glm::vec4 mBeginColor = { 1.0f, 1.0f, 1.0f, 1.0f };
		glm::vec4 mEndColor = { 1.0f, 1.0f, 1.0f, 0.0f };
		sky::Duration mUptime;
		std::weak_ptr<Node> mHolder;

		struct Segment
		{
			glm::vec2 pos = { 0.0f, 0.0f };
			sky::Duration time;
		};

		std::deque<Segment> mSegments;
	};
}
