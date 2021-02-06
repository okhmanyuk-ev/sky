#include "scrollbox.h"
#include "console/device.h"

using namespace Scene;

Scrollbox::Scrollbox()
{
	setTouchable(true);

	mBounding = std::make_shared<Node>();
	attach(mBounding);

	mContent = std::make_shared<Node>();
	mBounding->attach(mContent);

	mTimestepFixer.setTimestep(1.0f / 120.0f);
	mTimestepFixer.setCallback([this](float dTime) {
		physics(dTime);
	});
}

void Scrollbox::update()
{
	Node::update();
	mTimestepFixer.execute();
}

void Scrollbox::touch(Touch type, const glm::vec2& pos)
{
	Node::touch(type, pos);

	auto local_pos = unproject(pos);

	if (type != Touch::Begin)
		mSpeed -= local_pos - mPrevPosition;

	mPrevPosition = local_pos;
}

void Scrollbox::physics(float dTime)
{
	auto speed = mSpeed * mSensitivity / getScrollSpace();

	if (glm::isnan(speed.x))
		speed.x = 0.0f;

	if (glm::isnan(speed.y))
		speed.y = 0.0f;

	auto delta = dTime * 100.0f;

	if (!isTouching())
		speed *= delta;

	mScrollPosition += speed;

	if (getContent()->getAbsoluteWidth() < getBounding()->getAbsoluteWidth())
		mScrollPosition.x = mScrollOrigin.x;

	if (getContent()->getAbsoluteHeight() < getBounding()->getAbsoluteHeight())
		mScrollPosition.y = mScrollOrigin.y;

	if (mInertiaEnabled && !isTouching())
		mSpeed *= 1.0f - (mInertiaFriction * delta);
	else
		mSpeed = { 0.0f, 0.0f };

	mScrollPosition = glm::clamp(mScrollPosition);

	mContent->setAnchor(mScrollPosition);
	mContent->setPivot(mScrollPosition);
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