#include "label.h"

using namespace Scene;

void Label::update()
{
	Node::update();

	if (mFont == nullptr || mFontSize <= 0.0f)
		return;

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
			mMeshWidth = mFont->getStringWidth(mText, mFontSize);
			mMeshHeight = mFont->getStringHeight(mText, mFontSize);
			mMesh = GRAPHICS->createSinglelineTextMesh(*mFont, mText);
		}
		else if (width > 0.0f)
		{
			mMeshWidth = width;
			std::tie(mMeshHeight, mMesh) = GRAPHICS->createMultilineTextMesh(*mFont, mText, width, mFontSize);
		}
		mMeshDirty = false;
	}

	setWidth(mMeshWidth);
	setHeight(mMeshHeight);
}

void Label::draw()
{
	Node::draw();

	if (mFont == nullptr || mFontSize <= 0.0f || (mMultiline && getWidth() <= 0.0f))
		return;

	auto scale = mFont->getScaleFactorForSize(mFontSize);
	auto model = glm::scale(getTransform(), { scale, scale, 1.0f });

	GRAPHICS->push(Renderer::Sampler::Linear);
	GRAPHICS->drawString(*mFont, mMesh, model, mFontSize, getColor(), mOutlineThickness, mOutlineColor);
	GRAPHICS->pop();
}