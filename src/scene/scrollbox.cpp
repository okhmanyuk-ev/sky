#include "scrollbox.h"
#include <console/device.h>
#include <common/helpers.h>

using namespace Scene;

Scrollbox::Scrollbox()
{
	setTouchable(true);

	mBounding = std::make_shared<Node>();
	attach(mBounding);

	mContent = std::make_shared<Node>();
	mBounding->attach(mContent);
}

void Scrollbox::update(Clock::Duration dTime)
{
	Node::update(dTime);

	if (glm::abs(mSpeed.y) <= mInsignificantSpeed)
		mSpeed.y = 0.0f;

	if (glm::abs(mSpeed.x) <= mInsignificantSpeed)
		mSpeed.x = 0.0f;

	auto scroll_space = getScrollSpace();
	auto overscroll_factor = 1.0f - (glm::abs(mOverscrollDistance) / getAbsoluteSize());
	auto speed = mSpeed * mSensitivity / scroll_space * glm::pow3(overscroll_factor);
	
	if (glm::isnan(speed.x) || glm::isinf(speed.x))
		speed.x = 0.0f;

	if (glm::isnan(speed.y) || glm::isinf(speed.y))
		speed.y = 0.0f;

	if (!isTouching())
		speed *= Clock::ToSeconds(dTime) * 100.0f;

	mScrollPosition += speed;

	if (getContent()->getAbsoluteWidth() < getBounding()->getAbsoluteWidth())
		mScrollPosition.x = mScrollOrigin.x;

	if (getContent()->getAbsoluteHeight() < getBounding()->getAbsoluteHeight())
		mScrollPosition.y = mScrollOrigin.y;

	if (mInertiaEnabled && !isTouching())
		mSpeed = Common::Helpers::SmoothValue(mSpeed, { 0.0f, 0.0f }, dTime, mInertiaFriction);
	else
		mSpeed = { 0.0f, 0.0f };

	auto clamped_pos = glm::clamp(mScrollPosition);

	if (!mOverscrollEnabled)
		mScrollPosition = clamped_pos;
	else if (!isTouching())
		mScrollPosition = Common::Helpers::SmoothValue(mScrollPosition, clamped_pos, dTime);

	mOverscrollDistance = (mScrollPosition - clamped_pos) * scroll_space;

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
	return glm::length(mSpeed) != 0.0f && !isTouching();
}

bool Scrollbox::isPullbacking() const 
{
	return glm::length(mOverscrollDistance) != 0.0f && !isTouching();
}
