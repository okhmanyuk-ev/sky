#include "label.h"

using namespace Scene;

void Label::update()
{
	if (mFont == nullptr || mFontSize <= 0.0f)
	{
		Node::update();
		return;
	}

	if (mPrevText != mText)
	{
		mPrevText = mText;
		mMeshDirty = true;
	}

	auto width = getWidth();

	if (mMultiline && mPrevWidth != width)
	{
		mPrevWidth = width;
		mMeshDirty = true;
	}

	if (mPrevFontSize != mFontSize)
	{
		mPrevFontSize = mFontSize;
		mMeshDirty = true;
	}

	if (mPrevFont != mFont)
	{
		mPrevFont = mFont;
		mMeshDirty = true;
	}

	if (mPrevMultiline != mMultiline)
	{
		mPrevMultiline = mMultiline;
		mMeshDirty = true;
	}

	if (mMeshDirty)
	{
		if (!mMultiline)
		{
			setWidth(mFont->getStringWidth(mText, mFontSize));
			setHeight(mFont->getStringHeight(mText, mFontSize));
			mMesh = GRAPHICS->createTextMesh(*mFont, mText);
		}
		else if (width > 0.0f)
		{
			float height = 0.0f;
			std::tie(height, mMesh) = GRAPHICS->createMultilineTextMesh(*mFont, mText, width, mFontSize);
			setHeight(height);
		}
		mMeshDirty = false;
	}

	Node::update();
}

void Label::draw()
{
	if (mFont == nullptr || mFontSize <= 0.0f || (mMultiline && getWidth() <= 0.0f))
	{
		Node::draw();
		return;
	}

	auto scale = mFont->getScaleFactorForSize(mFontSize);
	auto model = glm::scale(getTransform(), { scale, scale, 1.0f });

	GRAPHICS->push(Renderer::Sampler::Linear);
	GRAPHICS->drawString(*mFont, mMesh, model, mFontSize, getColor(), mOutlineThickness, mOutlineColor);
	GRAPHICS->pop();

	Node::draw();
}