#include "label.h"

using namespace Scene;

void Label::draw()
{
	Node::draw();

	if (mFont == nullptr || mFontSize <= 0.0f || (mMultiline && getAbsoluteWidth() <= 0.0f))
		return;

	if (getAlpha() <= 0.0f)
		return;

	auto scale = mFont->getScaleFactorForSize(mFontSize);
	auto model = glm::scale(getTransform(), { scale, scale, 1.0f });

	GRAPHICS->pushSampler(Renderer::Sampler::Linear);
	GRAPHICS->drawString(*mFont, mMesh, model, mFontSize, getColor(), mOutlineThickness, mOutlineColor);
	GRAPHICS->pop();
}

void Label::update()
{
	Node::update();
	updateTextMesh();
}

void Label::updateTextMesh()
{
	if (mFont == nullptr || mFontSize <= 0.0f)
		return;

	if (mPrevText != mText)
	{
		mPrevText = mText;
		mMeshDirty = true;
	}

	auto width = getAbsoluteWidth();

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

	if (mPrevMultilineAlign != mMultilineAlign)
	{
		mPrevMultilineAlign = mMultilineAlign;
		mMeshDirty = true;
	}

	if (!mMeshDirty)
		return;

	mMeshDirty = false;

	if (!mMultiline)
	{
		mMeshWidth = mFont->getStringWidth(mText, mFontSize);
		mMeshHeight = (mFont->getAscent() - (mFont->getDescent() * 2.0f)) * mFont->getScaleFactorForSize(mFontSize);
		mMesh = Graphics::TextMesh::createSinglelineTextMesh(*mFont, mText, -mFont->getDescent() + mFont->getCustomVerticalOffset());
	}
	else if (width > 0.0f)
	{
		mMeshWidth = width;
		std::tie(mMeshHeight, mMesh) = Graphics::TextMesh::createMultilineTextMesh(*mFont, mText, width, mFontSize, mMultilineAlign);
	}

	setWidth(mMeshWidth * (1.0f - getHorizontalStretch()));
	setHeight(mMeshHeight * (1.0f - getVerticalStretch()));

	Node::updateAbsoluteSize();
}