#include "label.h"

using namespace Scene;

void Label::draw()
{
	Node::draw();

	assert(mFont); // you should setup Label::DefaultFont

	if (mFont == nullptr || mFontSize <= 0.0f)
		return;

	if (getAlpha() <= 0.0f && (mOutlineColor->getAlpha() <= 0.0f || mOutlineThickness <= 0.0f))
		return;

	if (mMesh.vertices.empty())
		return;

	auto scale = mFont->getScaleFactorForSize(mFontSize);
	auto model = glm::scale(getTransform(), { scale, scale, 1.0f });

	GRAPHICS->pushSampler(Renderer::Sampler::Linear);
	GRAPHICS->pushModelMatrix(model);
	GRAPHICS->drawString(*mFont, mMesh, mFontSize, getColor(), mOutlineThickness, mOutlineColor->getColor());
	GRAPHICS->pop(2);
}

void Label::update(Clock::Duration dTime)
{
	Node::update(dTime);
	refresh();
}

void Label::refresh()
{
	if (mFont == nullptr || mFontSize <= 0.0f)
		return;

	auto mesh_dirty = false;
	auto width = getAbsoluteWidth();

	if (mPrevWidth != width && mMultiline)
	{
		mPrevWidth = width;
		mesh_dirty = true;
	}

	if (mPrevText != mText)
	{
		mPrevText = mText;
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

	if (mPrevAlign != mAlign)
	{
		mPrevAlign = mAlign;
		mesh_dirty = true;
	}

	if (mPrevMultiline != mMultiline) 
	{
		mPrevMultiline = mMultiline;
		mesh_dirty = true;
	}

	if (!mesh_dirty)
		return;

	float height = 0.0f;

	if (!mMultiline)
	{
		height = (mFont->getAscent() - (mFont->getDescent() * 2.0f)) * mFont->getScaleFactorForSize(mFontSize);
		mMesh = Graphics::TextMesh::createSinglelineTextMesh(*mFont, mText, -mFont->getDescent() + mFont->getCustomVerticalOffset());
		setWidth(mFont->getStringWidth(mText, mFontSize));
	}
	else 
	{
		std::tie(height, mMesh) = Graphics::TextMesh::createMultilineTextMesh(*mFont, mText, width, mFontSize, mAlign);
	}

	setHeight(height);

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
