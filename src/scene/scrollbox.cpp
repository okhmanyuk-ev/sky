#include "scrollbox.h"

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

	if (mContent->getX() + mContent->getWidth() < mBounding->getWidth())
		mContent->setX(mBounding->getWidth() - mContent->getWidth());

	if (mContent->getY() + mContent->getHeight() < mBounding->getHeight())
		mContent->setY(mBounding->getHeight() - mContent->getHeight());

	if (mContent->getX() > 0.0f)
		mContent->setX(0.0f);

	if (mContent->getY() > 0.0f)
		mContent->setY(0.0f);
}

void Scrollbox::touch(Touch type, const glm::vec2& pos)
{
	Node::touch(type, pos);

	if (type == Touch::Begin)
	{
		mPrevPosition = pos;
	}
	else if (type == Touch::Continue)
	{
		auto diff = pos - mPrevPosition;
		diff /= PLATFORM->getScale();
		mContent->setPosition(mContent->getPosition() + (diff * mSensitivity));
		mPrevPosition = pos;
	}
	else
	{
		auto diff = pos - mPrevPosition;
		//LOG("touch end diff x: " + std::to_string(diff.x));
		// log always 0.0
	}
}