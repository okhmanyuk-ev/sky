#include "scrollbox.h"
#include "console/device.h"

using namespace Scene;

Scrollbox::Scrollbox()
{
	setTouchable(true);

	attach(mBounding);

	mBounding->attach(mContent);
}

void Scrollbox::update()
{
	Node::update();

	mContent->setPosition(mContent->getPosition() + (mSpeed * mSensitivity));

	if (mInertiaEnabled && !isTouching())
		mSpeed *= 1.0f - (mInertiaFriction * Clock::ToSeconds(FRAME->getTimeDelta()) * 100.0f);
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

void Scrollbox::touch(Touch type, const glm::vec2& pos)
{
	Node::touch(type, pos);
	
	if (type == Touch::Continue)
		mSpeed += (pos - mPrevPosition) / PLATFORM->getScale();

	mPrevPosition = pos;
}