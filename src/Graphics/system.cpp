#include "system.h"

#include <vector>
#include <numeric>

using namespace Graphics;

void System::begin(const glm::mat4& viewMatrix, const glm::mat4& projectionMatrix)
{
	assert(!mWorking);
	mWorking = true;

	auto state = State();
	state.viewport = Renderer::Viewport::FullScreen();
	state.viewMatrix = viewMatrix;
	state.projectionMatrix = projectionMatrix;
	mStates.push(state);
	applyState();
}

void System::beginOrtho()
{
	auto viewMatrix = glm::lookAtLH(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f),
		glm::vec3(0.0f, 1.0f, 0.0f));

	auto projectionMatrix = glm::orthoLH(0.0f, PLATFORM->getLogicalWidth(), PLATFORM->getLogicalHeight(), 0.0f, -1.0f, 1.0f);
	
	begin(viewMatrix, projectionMatrix);
}

void System::beginOrtho(const Renderer::RenderTarget& target)
{
	auto viewMatrix = glm::lookAtLH(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f),
		glm::vec3(0.0f, 1.0f, 0.0f));

	auto projectionMatrix = glm::orthoLH(0.0f, (float)target.getWidth(), (float)target.getHeight(), 0.0f, -1.0f, 1.0f);

	begin(viewMatrix, projectionMatrix);
}

void System::begin(const Camera& camera)
{
	begin(camera.getViewMatrix(), camera.getProjectionMatrix());
}

void System::end()
{
	assert(mWorking);
	assert(mStates.size() == 1);
	mWorking = false;
	flush();
	mStates.pop();
}

void System::applyState()
{
	assert(!mStates.empty());

	const auto& state = mStates.top();
	
	if (state.scissor.has_value())
		RENDERER->setScissor(state.scissor.value());
	else
		RENDERER->setScissor(nullptr);

	if (state.renderTarget)
		RENDERER->setRenderTarget(*state.renderTarget);
	else
		RENDERER->setRenderTarget(nullptr);

	RENDERER->setDepthMode(state.depthMode);
	RENDERER->setCullMode(Renderer::CullMode::None);
	RENDERER->setViewport(state.viewport);
	RENDERER->setBlendMode(state.blendMode);
}

void System::flush()
{
	if (mBatchFlushed)
		return;

	if (mBatchTexture.has_value()) // means we need textured shader
	{
		mTexturedShader.setProjectionMatrix(mStates.top().projectionMatrix);
		mTexturedShader.setViewMatrix(mStates.top().viewMatrix);
		mTexturedShader.setModelMatrix(glm::mat4(1.0f));

		RENDERER->setShader(mTexturedShader);
		RENDERER->setVertexBuffer(mBatchTexturedVertices);
		RENDERER->setTexture(*mBatchTexture.value());
	}
	else
	{
		mColoredShader.setProjectionMatrix(mStates.top().projectionMatrix);
		mColoredShader.setViewMatrix(mStates.top().viewMatrix);
		mColoredShader.setModelMatrix(glm::mat4(1.0f));

		RENDERER->setShader(mColoredShader);
		RENDERER->setVertexBuffer(mBatchColoredVertices);
	}

	RENDERER->setSampler(mStates.top().sampler);

	RENDERER->setTopology(mBatchTopology.value());
	RENDERER->setIndexBuffer(mBatchIndices);
	RENDERER->drawIndexed(mBatchIndicesCount);

	mBatchFlushed = true;
	mBatchVerticesCount = 0;
	mBatchIndicesCount = 0;
}

void System::clear(const glm::vec4& color)
{
	RENDERER->clear(color);
}

void System::draw(Renderer::Topology topology, const std::vector<Renderer::Vertex::PositionColor>& vertices,
	const glm::mat4& model)
{
	auto indices = std::vector<uint32_t>(vertices.size());
	std::iota(indices.begin(), indices.end(), 0);
	draw(topology, vertices, indices, model);
}

void System::draw(Renderer::Topology topology, const std::vector<Renderer::Vertex::PositionColor>& vertices,
	const std::vector<uint32_t>& indices, const glm::mat4& model)
{
	assert(mWorking);

	if (mBatching)
	{
		if (mBatchTexture != std::nullopt || mBatchTopology != topology)
			flush();

		mBatchTexture = std::nullopt;
		mBatchTopology = topology;

		auto project = [&model, this](const glm::vec3& pos) {
			auto width = PLATFORM->getLogicalWidth();
			auto height = PLATFORM->getLogicalHeight();
			glm::vec4 viewport = { 0.0f, height, width, -height };
			return glm::project(pos, model, mStates.top().projectionMatrix, viewport);
		};

		mBatchVerticesCount += vertices.size();

		if (mBatchVerticesCount > mBatchColoredVertices.size())
			mBatchColoredVertices.resize(mBatchVerticesCount);

		auto start_vertex = mBatchVerticesCount - vertices.size();

		for (const auto& vertex : vertices)
		{
			mBatchColoredVertices[start_vertex] = vertex;
			mBatchColoredVertices[start_vertex].pos = project(mBatchColoredVertices.at(start_vertex).pos);
			start_vertex += 1;
		}

		mBatchIndicesCount += indices.size();

		if (mBatchIndicesCount > mBatchIndices.size())
			mBatchIndices.resize(mBatchIndicesCount);

		auto start_index = mBatchIndicesCount - indices.size();

		for (auto index : indices)
		{
			mBatchIndices[start_index] = static_cast<uint32_t>(mBatchVerticesCount - vertices.size() + static_cast<size_t>(index));
			start_index += 1;
		}

		mBatchFlushed = false;
	}
	else
	{
		flush();

		mColoredShader.setProjectionMatrix(mStates.top().projectionMatrix);
		mColoredShader.setViewMatrix(mStates.top().viewMatrix);
		mColoredShader.setModelMatrix(model);

		RENDERER->setTopology(topology);
		RENDERER->setIndexBuffer(indices);
		RENDERER->setVertexBuffer(vertices);
		RENDERER->setSampler(mStates.top().sampler);
		RENDERER->setShader(mColoredShader);

		RENDERER->drawIndexed(indices.size());
	}
}

void System::draw(Renderer::Topology topology, std::shared_ptr<Renderer::Texture> texture, const std::vector<Renderer::Vertex::PositionColorTexture>& vertices,
	const std::vector<uint32_t>& indices, const glm::mat4& model)
{
	assert(mWorking);

	if (mBatching)
	{
		if (mBatchTexture != texture || mBatchTopology != topology)
			flush();

		mBatchTexture = texture;
		mBatchTopology = topology;

		auto project = [&model, this](const glm::vec3& pos) {
			auto width = PLATFORM->getLogicalWidth();
			auto height = PLATFORM->getLogicalHeight();
			glm::vec4 viewport = { 0.0f, height, width, -height };
			return glm::project(pos, model, mStates.top().projectionMatrix, viewport);
		};

		mBatchVerticesCount += vertices.size();

		if (mBatchVerticesCount > mBatchTexturedVertices.size())
			mBatchTexturedVertices.resize(mBatchVerticesCount);

		auto start_vertex = mBatchVerticesCount - vertices.size();

		for (const auto& vertex : vertices)
		{
			mBatchTexturedVertices[start_vertex] = vertex;
			mBatchTexturedVertices[start_vertex].pos = project(mBatchTexturedVertices.at(start_vertex).pos);
			start_vertex += 1;
		}

		mBatchIndicesCount += indices.size();

		if (mBatchIndicesCount > mBatchIndices.size())
			mBatchIndices.resize(mBatchIndicesCount);

		auto start_index = mBatchIndicesCount - indices.size();

		for (auto index : indices)
		{
			mBatchIndices[start_index] = static_cast<uint32_t>(mBatchVerticesCount - vertices.size() + static_cast<size_t>(index));
			start_index += 1;
		}

		mBatchFlushed = false;
	}
	else
	{
		flush();

		mTexturedShader.setProjectionMatrix(mStates.top().projectionMatrix);
		mTexturedShader.setViewMatrix(mStates.top().viewMatrix);
		mTexturedShader.setModelMatrix(model);

		RENDERER->setTexture(*texture);
		RENDERER->setTopology(topology);
		RENDERER->setIndexBuffer(indices);
		RENDERER->setVertexBuffer(vertices);
		RENDERER->setSampler(mStates.top().sampler);
		RENDERER->setShader(mTexturedShader);

		RENDERER->drawIndexed(indices.size());
	}
}

void System::drawRectangle(const glm::mat4& model, const glm::vec4& color)
{
	std::vector<Renderer::Vertex::PositionColor> vertices = {
		{ { 0.0f, 0.0f, 0.0f }, color },
		{ { 0.0f, 1.0f, 0.0f }, color },
		{ { 1.0f, 1.0f, 0.0f }, color },
		{ { 1.0f, 0.0f, 0.0f }, color }
	};

	static const std::vector<uint32_t> indices = { 0, 1, 2, 0, 2, 3 };

	draw(Renderer::Topology::TriangleList, vertices, indices, model);
}

void System::drawLineRectangle(const glm::mat4& model, const glm::vec4& color)
{
	std::vector<Renderer::Vertex::PositionColor> vertices = {
		{ { 0.0f, 0.0f, 0.0f }, color },
		{ { 0.0f, 1.0f, 0.0f }, color },
		{ { 1.0f, 1.0f, 0.0f }, color },
		{ { 1.0f, 0.0f, 0.0f }, color }
	};

	static const std::vector<uint32_t> indices = { 0, 1, 1, 2, 2, 3, 3, 0 };

	draw(Renderer::Topology::LineList, vertices, indices, model);
}

void System::drawCircle(const glm::mat4& model, int segments, const glm::vec4& color)
{
	const float Radius = 0.5f;
	const float Increment = 2.0f * glm::pi<float>() / segments;

	float sinInc = glm::sin(Increment);
	float cosInc = glm::cos(Increment);

	auto r1 = glm::vec2({ cosInc, sinInc });
	auto v1 = Radius * r1;

	std::vector<Renderer::Vertex::PositionColor> vertices;

	for (int i = 0; i < segments; i++)
	{
		auto r2 = glm::vec2({ cosInc * r1.x - sinInc * r1.y, sinInc * r1.x + cosInc * r1.y });
		auto v2 = Radius * r2;
		vertices.push_back({ { Radius, Radius, 0.0f }, color });
		vertices.push_back({ { v1 + Radius, 0.0f }, color });
		vertices.push_back({ { v2 + Radius, 0.0f }, color });
		r1 = r2;
		v1 = v2;
	}

	draw(Renderer::Topology::TriangleList, vertices, model);
}

void System::drawLineCircle(const glm::mat4& model, int segments, const glm::vec4& color)
{
	const float Radius = 0.5f;
	const float Increment = 2.0f * glm::pi<float>() / segments;

	float sinInc = glm::sin(Increment);
	float cosInc = glm::cos(Increment);

	auto r1 = glm::vec2({ cosInc, sinInc });
	auto v1 = Radius * r1;

	std::vector<Renderer::Vertex::PositionColor> vertices;

	for (int i = 0; i < segments; i++)
	{
		auto r2 = glm::vec2({ cosInc * r1.x - sinInc * r1.y, sinInc * r1.x + cosInc * r1.y });
		auto v2 = Radius * r2;
		vertices.push_back({ { v1 + Radius, 0.0f }, color });
		vertices.push_back({ { v2 + Radius, 0.0f }, color });
		r1 = r2;
		v1 = v2;
	}

	draw(Renderer::Topology::LineList, vertices, model);
}

void System::draw(std::shared_ptr<Renderer::Texture> texture, const glm::mat4& model,
	const TexRegion& tex_region, const glm::vec4& color)
{
	float tex_w = static_cast<float>(texture->getWidth());
	float tex_h = static_cast<float>(texture->getHeight());

	float s_x1 = tex_region.pos.x / tex_w;
	float s_y1 = tex_region.pos.y / tex_h;
	float s_x2 = (tex_region.size.x > 0.0f ? (tex_region.size.x / tex_w) : 1.0f) + s_x1;
	float s_y2 = (tex_region.size.y > 0.0f ? (tex_region.size.y / tex_h) : 1.0f) + s_y1;

	std::vector<Renderer::Vertex::PositionColorTexture> vertices = {
		{ { 0.0f, 0.0f, 0.0f }, color, { s_x1, s_y1 } },
		{ { 0.0f, 1.0f, 0.0f }, color, { s_x1, s_y2 } },
		{ { 1.0f, 1.0f, 0.0f }, color, { s_x2, s_y2 } },
		{ { 1.0f, 0.0f, 0.0f }, color, { s_x2, s_y1 } }
	};
	
	static const std::vector<uint32_t> indices = { 0, 1, 2, 0, 2, 3 };
	
	draw(Renderer::Topology::TriangleList, texture, vertices, indices, model);
}

void System::draw(const Font& font, const std::string& text, const glm::mat4& model,
	const glm::vec4& color, float minValue, float maxValue, float smoothFactor)
{
	assert(mWorking);

	if (text.empty())
		return;

	flush();

	const auto texture = font.getTexture();
	
	float str_w = font.getStringWidth(text);
	float str_h = font.getStringHeight(text);

	float tex_w = static_cast<float>(texture->getWidth());
	float tex_h = static_cast<float>(texture->getHeight());

	std::vector<Renderer::Vertex::PositionColorTexture> vertices(4 * text.size());
	std::vector<uint16_t> indices(6 * text.size());

	auto pos = glm::vec2(0.0f);
	
	for (int i = 0; i < static_cast<int>(text.size()); i++)
	{
		const auto& glyph = font.getGlyph((uint8_t)text.at(i));

		float glyph_w = static_cast<float>(glyph.w);
		float glyph_h = static_cast<float>(glyph.h);
		float glyph_x = static_cast<float>(glyph.x);
		float glyph_y = static_cast<float>(glyph.y);
		
		auto vtx = &vertices[i * 4];
		auto idx = &indices[i * 6];

		pos.y = glyph.yoff;
		pos.x += glyph.xoff;

		float x1 = pos.x;
		float x2 = pos.x + glyph_w ;
		float y1 = pos.y + str_h;
		float y2 = pos.y + str_h + glyph_h;

		pos.x -= glyph.xoff;
		pos.x += glyph.xadvance;
		
		if (i < static_cast<int>(text.size()) - 1)
		{
			pos.x += font.getKerning((uint8_t)text.at(i), (uint8_t)text.at(i + 1));
		}

		float u1 = glyph_x / tex_w;
		float v1 = glyph_y / tex_h;
		float u2 = (glyph_x + glyph_w) / tex_w;
		float v2 = (glyph_y + glyph_h) / tex_h;

		vtx[0] = { { x1, y1, 0.0f }, color, { u1, v1 } };
		vtx[1] = { { x1, y2, 0.0f }, color, { u1, v2 } };
		vtx[2] = { { x2, y2, 0.0f }, color, { u2, v2 } };
		vtx[3] = { { x2, y1, 0.0f }, color, { u2, v1 } };

		auto start_vtx = i * 4;

		idx[0] = start_vtx + 0;
		idx[1] = start_vtx + 1;
		idx[2] = start_vtx + 2;
		idx[3] = start_vtx + 0;
		idx[4] = start_vtx + 2;
		idx[5] = start_vtx + 3;
	}
	
	if (mSdfEnabled)
	{
		mSdfShader.setProjectionMatrix(mStates.top().projectionMatrix);
		mSdfShader.setViewMatrix(mStates.top().viewMatrix);
		mSdfShader.setModelMatrix(model);
		mSdfShader.setMinValue(minValue);
		mSdfShader.setMaxValue(maxValue);
		mSdfShader.setSmoothFactor(smoothFactor);
		RENDERER->setShader(mSdfShader);
	}
	else
	{
		mTexturedShader.setProjectionMatrix(mStates.top().projectionMatrix);
		mTexturedShader.setViewMatrix(mStates.top().viewMatrix);
		mTexturedShader.setModelMatrix(model);
		RENDERER->setShader(mTexturedShader);
	}

	RENDERER->setTexture(*texture);
	RENDERER->setTopology(Renderer::Topology::TriangleList);
	RENDERER->setIndexBuffer(indices);
	RENDERER->setVertexBuffer(vertices);
	RENDERER->setSampler(Renderer::Sampler::Linear);

	RENDERER->drawIndexed(indices.size());
}

void System::draw(const Font& font, const std::string& text, const glm::mat4& model,
	float size, const glm::vec4& color, float outlineThickness, const glm::vec4& outlineColor)
{
	if (size <= 0.0f)
		return;
	
	float fixedOutlineThickness = glm::lerp(0.0f, 0.75f, outlineThickness);

	const float min = 0.0f;
	const float mid = Font::SdfOnedge;
	const float max = 1.0f;
	const float outline = glm::lerp(mid, min, fixedOutlineThickness);
	float smoothFactor = 2.0f / size / PLATFORM->getScale();
	
	if (fixedOutlineThickness > 0.0f && mSdfEnabled)
	{
		draw(font, text, model, outlineColor, outline,
			mid + (smoothFactor / 2.0f), smoothFactor);
	}
	draw(font, text, model, color, mid, max, smoothFactor);
}

void System::push(const State& value)
{
	assert(mWorking);

	if (mStates.top() != value)
		flush();
	
	mStates.push(value);
	applyState();
}

void System::pop(int count)
{
	assert(mWorking);
	
	for (int i = 0; i < count; i++)
	{
		assert(!mStates.empty());
		auto state = mStates.top();

		mStates.pop();

		if (state != mStates.top())
		{
			mStates.push(state);
			flush();
			mStates.pop();
		}

		if (!mStates.empty())
			applyState();
	}
}

void System::push(Renderer::Sampler value)
{
	auto state = mStates.top();
	state.sampler = value;
	push(state);
}

void System::push(std::shared_ptr<Renderer::RenderTarget> value)
{
	auto state = mStates.top();
	state.renderTarget = value;
	push(state);
}

void System::push(std::optional<Renderer::Scissor> value)
{
	auto state = mStates.top();
	state.scissor = value;
	push(state);
}

void System::push(Renderer::BlendMode value)
{
	auto state = mStates.top();
	state.blendMode = value;
	push(state);
}

void System::push(Renderer::DepthMode value)
{
	auto state = mStates.top();
	state.depthMode = value;
	push(state);
}

void System::push(const Renderer::Viewport& value)
{
	auto state = mStates.top();
	state.viewport = value;
	push(state);
}

void System::pushViewMatrix(const glm::mat4& value)
{
	auto state = mStates.top();
	state.viewMatrix = value;
	push(state);
}

void System::pushProjectionMatrix(const glm::mat4& value)
{
	auto state = mStates.top();
	state.projectionMatrix = value;
	push(state);
}