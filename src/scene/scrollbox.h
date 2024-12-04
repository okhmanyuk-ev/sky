#pragma once

#include <scene/node.h>
#include <scene/auto_sized.h>
#include <common/timestep_fixer.h>

namespace Scene
{
	class Scrollbox : public Node
	{
	public:
		static inline float DefaultInertiaFriction = 0.03f;

	public:
		Scrollbox();

	protected:
		void update(Clock::Duration dTime) override;
		void touch(Touch type, const glm::vec2& pos) override;
		void scroll(float x, float y) override;

	private:
		glm::vec2 mPrevPosition = { 0.0f, 0.0f };

	public:
		glm::vec2 screenToScrollPosition(const glm::vec2& projected_screen_pos);

	public:
		auto getContent() { return mContent; }
		auto getBounding() { return mBounding; }

		auto getSensitivity() const { return mSensitivity; }
		void setSensitivity(const glm::vec2& value) { mSensitivity = value; }

		auto getSpeed() const { return mSpeed; }
		void setSpeed(const glm::vec2& value) { mSpeed = value; }

		auto isInertiaEnabled() const { return mInertiaEnabled; }
		void setInertiaEnabled(bool value) { mInertiaEnabled = value; }

		auto getInertiaFriction() const { return mInertiaFriction; }
		void setInertiaFriction(float value) { mInertiaFriction = value; }

		auto getScrollPosition() const { return mScrollPosition; }
		void setScrollPosition(const glm::vec2& value) { mScrollPosition = value; }
		void setScrollPosition(float value) { setScrollPosition({ value, value }); }

		auto getHorizontalScrollPosition() const { return mScrollPosition.x; }
		void setHorizontalScrollPosition(float value) { mScrollPosition.x = value; }

		auto getVerticalScrollPosition() const { return mScrollPosition.y; }
		void setVerticalScrollPosition(float value) { mScrollPosition.y = value; }

		auto getScrollOrigin() const { return mScrollOrigin; }
		void setScrollOrigin(const glm::vec2& value) { mScrollOrigin = value; }
		void setScrollOrigin(float value) { setScrollOrigin({ value, value }); }

		glm::vec2 getScrollSpace() const;
		auto getHorizontalScrollSpace() const { return getScrollSpace().x; }
		auto getVerticalScrollSpace() const { return getScrollSpace().y; }

		auto getSpeedThreshold() const { return mSpeedThreshold; }
		void setSpeedThreshold(float value) { mSpeedThreshold = value; }

		auto getOverscrollThreshold() const { return mOverscrollThreshold; }
		void setOverscrollThreshold(float value) { mOverscrollThreshold = value; }

		bool isOverscrollEnabled() const { return mOverscrollEnabled; }
		void setOverscrollEnabled(bool value) { mOverscrollEnabled = value; }

		bool isInerting() const;
		bool isPullbacking() const;
		const auto& getOverscrollSize() const { return mOverscrollSize; }

	private:
		std::shared_ptr<Node> mBounding;
		std::shared_ptr<AutoSized<Node>> mContent;
		glm::vec2 mSensitivity = { 1.0f, 1.0f };
		glm::vec2 mSpeed = { 0.0f, 0.0f };
		glm::vec2 mScrollPosition = { 0.0f, 0.0f };
		glm::vec2 mScrollOrigin = { 0.0f, 0.0f };
		bool mInertiaEnabled = true;
		float mInertiaFriction = DefaultInertiaFriction;
		float mSpeedThreshold = 0.01f;
		float mOverscrollThreshold = 0.001f;
		bool mOverscrollEnabled = true;
		glm::vec2 mOverscrollSize = { 0.0f, 0.0 };
	};
}
