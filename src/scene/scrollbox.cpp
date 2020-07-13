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
		mSpeed += local_pos - mPrevPosition;

	mPrevPosition = local_pos;
}

void Scrollbox::physics(float dTime)
{
	auto speed = mSpeed * mSensitivity;
	auto delta = dTime * 100.0f;

	if (!isTouching())
		speed *= delta;

	mContent->setPosition(mContent->getPosition() + speed);

	if (mInertiaEnabled && !isTouching())
		mSpeed *= 1.0f - (mInertiaFriction * delta);
	else
		mSpeed = { 0.0f, 0.0f };

	if (mContent->getX() + mContent->getWidth() < mBounding->getWidth())
	{
		mContent->setX(mBounding->getWidth() - mContent->getWidth());
		mSpeed.x = 0.0f;
	}

	if (mContent->getY() + mContent->getHeight() < mBounding->getHeight())
	{
		mContent->setY(mBounding->getHeight() - mContent->getHeight());
		mSpeed.y = 0.0f;
	}

	if (mContent->getX() > 0.0f)
	{
		mContent->setX(0.0f);
		mSpeed.x = 0.0f;
	}

	if (mContent->getY() > 0.0f)
	{
		mContent->setY(0.0f);
		mSpeed.y = 0.0f;
	}
}