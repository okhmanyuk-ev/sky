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

	auto speed = mSpeed * mSensitivity / getScrollSpace();

	if (glm::isnan(speed.x))
		speed.x = 0.0f;

	if (glm::isnan(speed.y))
		speed.y = 0.0f;

	if (!isTouching())
		speed *= Clock::ToSeconds(dTime) * 100.0f;

	mScrollPosition += speed;

	if (getContent()->getAbsoluteWidth() < getBounding()->getAbsoluteWidth())
		mScrollPosition.x = mScrollOrigin.x;

	if (getContent()->getAbsoluteHeight() < getBounding()->getAbsoluteHeight())
		mScrollPosition.y = mScrollOrigin.y;

	if (mInertiaEnabled && !isTouching())
		mSpeed = Common::Helpers::SmoothValueAssign(mSpeed, { 0.0f, 0.0f }, dTime, mInertiaFriction);
	else
		mSpeed = { 0.0f, 0.0f };

	mScrollPosition = glm::clamp(mScrollPosition);

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

glm::vec2 Scrollbox::screenToScrollPosition(const glm::vec2& projected_screen_pos)
{
	auto unproject_pos = mContent->unproject(projected_screen_pos);
	return unproject_pos / getScrollSpace();
}

glm::vec2 Scrollbox::getScrollSpace() const
{
	return mContent->getAbsoluteSize() - mBounding->getAbsoluteSize();
}