#include "label.h"

using namespace Scene;

void Label::draw()
{
	Node::draw();

	assert(mFont); // you should setup Label::DefaultFont

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

	if (mAutoRefreshing)
		refresh();
}

void Label::refresh()
{
	if (mFont == nullptr || mFontSize <= 0.0f)
		return;

	auto mesh_dirty = false;

	if (mPrevText != mText)
	{
		mPrevText = mText;
		mesh_dirty = true;
	}

	auto width = getAbsoluteWidth();

	if (mMultiline && mPrevWidth != width)
	{
		mPrevWidth = width;
		mesh_dirty = true;
	}

	if (mPrevFontSize != mFontSize)
	{
		mPrevFontSize = mFontSize;
		mesh_dirty = true;
	}

	if (mPrevFont != mFont)
	{
		mPrevFont = mFont;
		mesh_dirty = true;
	}

	if (mPrevMultiline != mMultiline)
	{
		mPrevMultiline = mMultiline;
		mesh_dirty = true;
	}

	if (mPrevMultilineAlign != mMultilineAlign)
	{
		mPrevMultilineAlign = mMultilineAlign;
		mesh_dirty = true;
	}

	if (!mesh_dirty)
		return;

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

std::tuple<glm::vec2, glm::vec2> Label::getSymbolBounds(int index)
{
	assert(!mText.empty());
	assert(index >= 0);
	assert(index < mText.length());

	auto scale = mFont->getScaleFactorForSize(mFontSize);
	
	auto pos = mMesh.symbol_positions.at(index) * scale;
	auto size = mMesh.symbol_sizes.at(index) * scale;

	return { pos, size };
}

float Label::getSymbolLineY(int index)
{
	assert(!mText.empty());
	assert(index >= 0);
	assert(index < mText.length());

	auto scale = mFont->getScaleFactorForSize(mFontSize);

	return mMesh.symbol_line_y.at(index) * scale;
}
