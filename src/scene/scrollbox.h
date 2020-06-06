#pragma once

#include <scene/node.h>

namespace Scene
{
	class Scrollbox : public Node
	{
	public:
		Scrollbox();

	protected:
		void update() override;
		void touch(Touch type, const glm::vec2& pos) override;

	private:
		glm::vec2 mPrevPosition = { 0.0f, 0.0f };
		
	public:
		auto getContent() { return mContent; }
		auto getBounding() { return std::static_pointer_cast<Transform>(mBounding); }

		auto getSensitivity() const { return mSensitivity; }
		void setSensitivity(const glm::vec2& value) { mSensitivity = value; }

		auto getSpeed() const { return mSpeed; }
		void setSpeed(const glm::vec2& value) { mSpeed = value; }

		auto getInertiaFriction() const { return mInertiaFriction; }
		void setInertiaFriction(float value) { mInertiaFriction = value; }

		auto isInertiaEnabled() const { return mInertiaEnabled; }
		void setInertiaEnabled(bool value) { mInertiaEnabled = value; }

	private:
		std::shared_ptr<Node> mBounding;
		std::shared_ptr<Node> mContent;
		glm::vec2 mSensitivity = { 1.0f, 1.0f };
		glm::vec2 mSpeed = { 0.0f, 0.0f };
		float mInertiaFriction = 0.03f;
		bool mInertiaEnabled = true;
	};
}
