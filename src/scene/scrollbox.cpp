#include "scrollbox.h"
#include <common/helpers.h>

using namespace Scene;

Scrollbox::Scrollbox()
{
	setTouchable(true);

	mBounding = std::make_shared<Node>();
	attach(mBounding);

	mContent = std::make_shared<AutoSized<Node>>();
	mContent->setAutoHeightEnabled(false);
	mContent->setAutoWidthEnabled(false);
	mBounding->attach(mContent);
}

void Scrollbox::update(sky::Duration dTime)
{
	Node::update(dTime);

	if (glm::abs(mSpeed.y) <= mSpeedThreshold)
		mSpeed.y = 0.0f;

	if (glm::abs(mSpeed.x) <= mSpeedThreshold)
		mSpeed.x = 0.0f;

	auto scroll_space = getScrollSpace();
	auto overscroll_factor = 1.0f - (glm::abs(mOverscrollSize) / getAbsoluteSize());
	auto speed = sky::sanitize(mSpeed * mSensitivity / scroll_space * glm::pow3(overscroll_factor));

	if (!isTouching())
		speed *= sky::ToSeconds(dTime) * 100.0f;

	mScrollPosition += speed;

	if (getContent()->getAbsoluteWidth() < getBounding()->getAbsoluteWidth())
		mScrollPosition.x = mScrollOrigin.x;

	if (getContent()->getAbsoluteHeight() < getBounding()->getAbsoluteHeight())
		mScrollPosition.y = mScrollOrigin.y;

	if (mInertiaEnabled && !isTouching())
		mSpeed = sky::ease_towards(mSpeed, { 0.0f, 0.0f }, dTime, mInertiaFriction);
	else
		mSpeed = { 0.0f, 0.0f };

	auto clamped_pos = glm::clamp(mScrollPosition);

	auto overscrollPullback = [&] {
		if (!isOverscrolled())
			return;

		if (!mOverscrollEnabled)
		{
			mScrollPosition = clamped_pos;
			return;
		}

		if (isTouching())
			return;

		mScrollPosition = sky::ease_towards(mScrollPosition, clamped_pos, dTime, mInertiaFriction);

		if (glm::distance(mScrollPosition.x, clamped_pos.x) <= mOverscrollThreshold)
			mScrollPosition.x = clamped_pos.x;

		if (glm::distance(mScrollPosition.y, clamped_pos.y) <= mOverscrollThreshold)
			mScrollPosition.y = clamped_pos.y;
	};

	auto alignToPage = [&] {
		if (!mPage.has_value())
			return;

		if (isTouching())
			return;

		if (isOverscrolled())
			return;

		auto normalized_item_size = sky::sanitize(mPage.value() / scroll_space);
		auto remainder = sky::sanitize(glm::mod(mScrollPosition, normalized_item_size));

		auto adjust_scroll_axis = [](auto current_axis_pos, auto item_axis_size, auto remainder_axis) {
			if (remainder_axis >= item_axis_size * 0.5f)
				return current_axis_pos + (item_axis_size - remainder_axis);
			else
				return current_axis_pos - remainder_axis;
		};

		glm::vec2 new_scroll_pos = {
			adjust_scroll_axis(mScrollPosition.x, normalized_item_size.x, remainder.x),
			adjust_scroll_axis(mScrollPosition.y, normalized_item_size.y, remainder.y)
		};

		mScrollPosition = sky::ease_towards(mScrollPosition, new_scroll_pos, dTime, mInertiaFriction);
	};

	overscrollPullback();

	mOverscrollSize = (mScrollPosition - clamped_pos) * scroll_space;

	alignToPage();

	mContent->setAnchor(mScrollPosition);
	mContent->setPivot(mScrollPosition);
}

void Scrollbox::touch(Touch type, const glm::vec2& pos)
{
	Node::touch(type, pos);

	auto local_pos = unproject(pos);

	if (type != Touch::Begin)
		mSpeed -= local_pos - mPrevPosition;

	mPrevPosition = local_pos;
}

void Scrollbox::scroll(float x, float y)
{
	mSpeed.x -= x * 8.0f;
	mSpeed.y -= y * 8.0f;
}

glm::vec2 Scrollbox::screenToScrollPosition(const glm::vec2& projected_screen_pos)
{
	auto unproject_pos = mContent->unproject(projected_screen_pos);
	return unproject_pos / getScrollSpace();
}

glm::vec2 Scrollbox::getScrollSpace() const
{
	return mContent->getAbsoluteSize() - mBounding->getAbsoluteSize();
}

bool Scrollbox::isInerting() const
{
	return glm::length(mSpeed) > 0.0f && !isTouching();
}

bool Scrollbox::isOverscrolled() const
{
	return glm::length(mOverscrollSize) > 0.0f;
}

bool Scrollbox::isPullbacking() const
{
	return isOverscrolled() && !isTouching();
}
