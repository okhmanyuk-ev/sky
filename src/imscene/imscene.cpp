#include "imscene.h"
#include <graphics/system.h>

void ImScene::Node::ensureMatrix()
{
	if (!dirty)
		return;

	auto verticalStretch = stretch.y / scale.y;
	auto horizontalStretch = stretch.x / scale.x;
	auto verticalMargin = margin.y;
	auto horizontalMargin = margin.x;

	if (horizontalStretch >= 0.0f)
		size.x = (parent_size.x * horizontalStretch) - horizontalMargin;

	if (verticalStretch >= 0.0f)
		size.y = (parent_size.y * verticalStretch) - verticalMargin;

	matrix = parent_matrix;
	matrix = glm::translate(matrix, { anchor * parent_size, 0.0f });
	matrix = glm::translate(matrix, { position, 0.0f });
	matrix = glm::rotate(matrix, rotation, { 0.0f, 0.0f, 1.0f });
	matrix = glm::scale(matrix, { scale, 1.0f });
	matrix = glm::translate(matrix, { -pivot * size, 0.0f });
	matrix = glm::translate(matrix, { -origin, 0.0f });

	dirty = false;
}

void ImScene::ImScene::begin()
{
	assert(!mWorking);
	mWorking = true;
	mDefaultMatrix = glm::mat4(1.0f);
	mDefaultSize = { PLATFORM->getLogicalWidth(), PLATFORM->getLogicalHeight() };
}

void ImScene::ImScene::begin(const glm::mat4& matrix, const glm::vec2& size)
{
	begin();
	mDefaultMatrix = matrix;
	mDefaultSize = size;
}

void ImScene::ImScene::end()
{
	assert(mWorking);
	assert(mNodeStack.empty());

	mWorking = false;
}

void ImScene::ImScene::push()
{
	assert(mWorking);

	auto node = Node();

	if (!mNodeStack.empty()) 
	{
		auto& parent = mNodeStack.top();
		parent.ensureMatrix();
		
		node.parent_size = parent.size;
		node.parent_matrix = parent.matrix;
	}
	else
	{
		node.parent_size = mDefaultSize;
		node.parent_matrix = mDefaultMatrix;
	}

	mNodeStack.push(node);
}

void ImScene::ImScene::pop(int count)
{
	assert(mWorking);
	for (int i = 0; i < count; i++)
	{
		mNodeStack.pop();
	}
}

glm::vec2 ImScene::ImScene::size() const
{
	auto& node = mNodeStack.top();
	return node.size;
}

void ImScene::ImScene::size(const glm::vec2& value)
{
	auto& node = mNodeStack.top();

	if (node.size == value)
		return;

	node.size = value;
	node.dirty = true;
}

void ImScene::ImScene::size(float value)
{
	size({ value, value });
}

glm::vec2 ImScene::ImScene::scale() const
{
	auto& node = mNodeStack.top();
	return node.scale;
}

void ImScene::ImScene::scale(const glm::vec2& value)
{
	auto& node = mNodeStack.top();

	if (node.scale == value)
		return;

	node.scale = value;
	node.dirty = true;
}

void ImScene::ImScene::scale(float value)
{
	scale({ value, value });
}

glm::vec2 ImScene::ImScene::position() const
{
	auto& node = mNodeStack.top();
	return node.position;
}

void ImScene::ImScene::position(const glm::vec2& value)
{
	auto& node = mNodeStack.top();

	if (node.position == value)
		return;

	node.position = value;
	node.dirty = true;
}

void ImScene::ImScene::position(float value)
{
	position({ value, value });
}

glm::vec2 ImScene::ImScene::stretch() const
{
	auto& node = mNodeStack.top();
	return node.stretch;
}

void ImScene::ImScene::stretch(const glm::vec2& value)
{
	auto& node = mNodeStack.top();

	if (node.stretch == value)
		return;

	node.stretch = value;
	node.dirty = true;
}

void ImScene::ImScene::stretch(float value)
{
	stretch({ value, value });
}

glm::vec2 ImScene::ImScene::anchor() const
{
	auto& node = mNodeStack.top();
	return node.anchor;
}

void ImScene::ImScene::anchor(const glm::vec2& value)
{
	auto& node = mNodeStack.top();

	if (node.anchor == value)
		return;

	node.anchor = value;
	node.dirty = true;
}

void ImScene::ImScene::anchor(float value)
{
	anchor({ value, value });
}

glm::vec2 ImScene::ImScene::pivot() const
{
	auto& node = mNodeStack.top();
	return node.pivot;
}

void ImScene::ImScene::pivot(const glm::vec2& value)
{
	auto& node = mNodeStack.top();

	if (node.pivot == value)
		return;

	node.pivot = value;
	node.dirty = true;
}

void ImScene::ImScene::pivot(float value)
{
	pivot({ value, value });
}

float ImScene::ImScene::rotation() const
{
	auto& node = mNodeStack.top();
	return node.rotation;
}

void ImScene::ImScene::rotation(float value)
{
	auto& node = mNodeStack.top();

	if (node.rotation == value)
		return;

	node.rotation = value;
	node.dirty = true;
}

void ImScene::ImScene::draw(Drawable& drawable)
{
	auto& node = mNodeStack.top();
	node.ensureMatrix();
	drawable.draw(node);
}

// Label

ImScene::Label& ImScene::Label::font(std::shared_ptr<Graphics::Font> value)
{
	mFont = value;
	return *this;
}

ImScene::Label& ImScene::Label::fontSize(float value)
{
	mFontSize = value;
	return *this;
}

ImScene::Label& ImScene::Label::text(const utf8_string& value)
{
	mText = value;
	return *this;
}

void ImScene::Label::draw(Node& node)
{
	auto w = mFont->getStringWidth(mText, mFontSize);
	auto h = (mFont->getAscent() - (mFont->getDescent() * 2.0f)) * mFont->getScaleFactorForSize(mFontSize);
	auto mesh = Graphics::TextMesh::createSinglelineTextMesh(*mFont, mText, -mFont->getDescent() + mFont->getCustomVerticalOffset());

	node.size = { w, h };
	node.dirty = true;
	node.ensureMatrix();

	auto scale = mFont->getScaleFactorForSize(mFontSize);
	auto model = glm::scale(node.matrix, { scale, scale, 1.0f });

	glm::vec4 color = { Graphics::Color::White, 1.0f };
	auto outline_thickness = 0.0f;
	glm::vec4 outline_color = { Graphics::Color::White, 1.0f };

	GRAPHICS->pushSampler(Renderer::Sampler::Linear);
	GRAPHICS->drawString(*mFont, mesh, model, mFontSize, color, outline_thickness, outline_color);
	GRAPHICS->pop();
}

// Rectangle

ImScene::Rectangle& ImScene::Rectangle::color(const glm::vec4& value)
{
	mColor = value;
	return *this;
}

void ImScene::Rectangle::draw(Node& node)
{
	auto model = glm::scale(node.matrix, { node.size, 1.0f });

	std::vector<Renderer::Vertex::PositionColor> vertices = {
		{ { 0.0f, 0.0f, 0.0f }, { mColor } },
		{ { 0.0f, 1.0f, 0.0f }, { mColor } },
		{ { 1.0f, 1.0f, 0.0f }, { mColor } },
		{ { 1.0f, 0.0f, 0.0f }, { mColor } }
	};

	static const std::vector<uint32_t> indices = { 0, 1, 2, 0, 2, 3 };
	GRAPHICS->draw(Renderer::Topology::TriangleList, vertices, indices, model);
}

// Sprite

ImScene::Sprite& ImScene::Sprite::texture(std::shared_ptr<Renderer::Texture> value)
{
	mTexture = value;
	return *this;
}

ImScene::Sprite& ImScene::Sprite::region(const Graphics::TexRegion& value)
{
	mRegion = value;
	return *this;
}

void ImScene::Sprite::draw(Node& node)
{
	if (node.size.x <= 0.0f)
	{
		if (mRegion.size.x > 0.0f)
			node.size.x = mRegion.size.x;
		else
			node.size.x = static_cast<float>(mTexture->getWidth());
	}

	if (node.size.y <= 0.0f)
	{
		if (mRegion.size.y > 0.0f)
			node.size.y = mRegion.size.y;
		else
			node.size.y = static_cast<float>(mTexture->getHeight());
	}

	node.dirty = true;
	node.ensureMatrix();

	auto model = glm::scale(node.matrix, { node.size, 1.0f });

	//GRAPHICS->pushSampler(getSampler());
	//GRAPHICS->pushBlendMode(getBlendMode());
	//GRAPHICS->pushTextureAddress(mTextureAddress);
	GRAPHICS->drawSprite(mTexture, model, mRegion/*, getColor()*/);
	//GRAPHICS->pop(3);
}

// Circle

ImScene::Circle& ImScene::Circle::color(const glm::vec4& value)
{
	mColor = value;
	return *this;
}

ImScene::Circle& ImScene::Circle::fill(float value)
{
	mFill = value;
	return *this;
}

ImScene::Circle& ImScene::Circle::pie(float value)
{
	mPie = value;
	return *this;
}

void ImScene::Circle::draw(Node& node)
{
	auto model = glm::scale(node.matrix, { node.size, 1.0f });

	auto mInnerColor = mColor;
	auto mOuterColor = mColor;

	auto inner_color = mColor;
	auto outer_color = mColor;
	//GRAPHICS->pushBlendMode(getBlendMode());
	GRAPHICS->drawCircle(model, inner_color, outer_color, mFill, mPie);
	//GRAPHICS->pop();
}