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
	mAppliedState = std::nullopt;
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

bool System::stateWouldApplied()
{
	assert(!mStates.empty());
	
	if (!mAppliedState.has_value())
		return true;

	return mStates.top() != mAppliedState.value();
}

void System::applyState()
{
	assert(stateWouldApplied());
	
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
	RENDERER->setSampler(state.sampler);

	mAppliedState = state;
}

void System::flush()
{
	if (mBatch.mode == BatchMode::None)
		return;

	const auto& state = mStates.top();
	
	if (mBatch.mode == BatchMode::Sdf)
	{
		mSdfShader.setProjectionMatrix(state.projectionMatrix);
		mSdfShader.setViewMatrix(state.viewMatrix);
		mSdfShader.setModelMatrix(glm::mat4(1.0f));

		RENDERER->setShader(mSdfShader);
		RENDERER->setVertexBuffer(mBatch.positionTextureVertices);
	}
	else if (mBatch.mode == BatchMode::Textured)
	{
		mTexturedShader.setProjectionMatrix(state.projectionMatrix);
		mTexturedShader.setViewMatrix(state.viewMatrix);
		mTexturedShader.setModelMatrix(glm::mat4(1.0f));

		RENDERER->setShader(mTexturedShader);
		RENDERER->setVertexBuffer(mBatch.positionColorTextureVertices);
	}
	else if (mBatch.mode == BatchMode::Colored)
	{
		mColoredShader.setProjectionMatrix(state.projectionMatrix);
		mColoredShader.setViewMatrix(state.viewMatrix);
		mColoredShader.setModelMatrix(glm::mat4(1.0f));

		RENDERER->setShader(mColoredShader);
		RENDERER->setVertexBuffer(mBatch.positionColorVertices);
	}

	if (mBatch.texture.has_value())
		RENDERER->setTexture(*mBatch.texture.value());

	RENDERER->setTopology(mBatch.topology.value());
	RENDERER->setIndexBuffer(mBatch.indices);

	RENDERER->drawIndexed(mBatch.indicesCount);

	mBatch.mode = BatchMode::None;
	mBatch.verticesCount = 0;
	mBatch.indicesCount = 0;
}

void System::clear(const glm::vec4& color)
{
	if (stateWouldApplied())
	{
		flush();
		applyState();
	}
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
	
	if (topology == Renderer::Topology::TriangleStrip)
	{
		auto new_indices = triangulate(topology, indices);
		draw(Renderer::Topology::TriangleList, vertices, new_indices, model);
		return;
	}

	if (stateWouldApplied())
	{
		flush();
		applyState();
	}

	if (mBatching)
	{
		if (mBatch.topology != topology || mBatch.texture != std::nullopt || mBatch.mode != BatchMode::Colored)
			flush();

		mBatch.mode = BatchMode::Colored;
		mBatch.texture = std::nullopt;
		mBatch.topology = topology;
		mBatch.verticesCount += vertices.size();

		if (mBatch.verticesCount > mBatch.positionColorVertices.size())
			mBatch.positionColorVertices.resize(mBatch.verticesCount);

		auto start_vertex = mBatch.verticesCount - vertices.size();

		for (const auto& src_vertex : vertices)
		{
			auto& dst_vertex = mBatch.positionColorVertices.at(start_vertex);
			dst_vertex.pos = project(src_vertex.pos, model);
			dst_vertex.col = src_vertex.col;
			start_vertex += 1;
		}

		pushBatchIndices(indices, vertices.size());
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

void System::draw(Renderer::Topology topology, std::shared_ptr<Renderer::Texture> texture,
	const std::vector<Renderer::Vertex::PositionColorTexture>& vertices,
	const glm::mat4& model)
{
	auto indices = std::vector<uint32_t>(vertices.size());
	std::iota(indices.begin(), indices.end(), 0);
	draw(topology, texture, vertices, indices, model);
}

void System::draw(Renderer::Topology topology, std::shared_ptr<Renderer::Texture> texture, const std::vector<Renderer::Vertex::PositionColorTexture>& vertices,
	const std::vector<uint32_t>& indices, const glm::mat4& model)
{
	assert(mWorking);

	if (topology == Renderer::Topology::TriangleStrip)
	{
		auto new_indices = triangulate(topology, indices);
		draw(Renderer::Topology::TriangleList, texture, vertices, new_indices, model);
		return;
	}

	if (stateWouldApplied())
	{
		flush();
		applyState();
	}

	if (mBatching)
	{
		if (mBatch.topology != topology || mBatch.texture != texture || mBatch.mode != BatchMode::Textured)
			flush();

		mBatch.mode = BatchMode::Textured;
		mBatch.texture = texture;
		mBatch.topology = topology;
		mBatch.verticesCount += vertices.size();

		if (mBatch.verticesCount > mBatch.positionColorTextureVertices.size())
			mBatch.positionColorTextureVertices.resize(mBatch.verticesCount);

		auto start_vertex = mBatch.verticesCount - vertices.size();

		for (const auto& src_vertex : vertices)
		{
			auto& dst_vertex = mBatch.positionColorTextureVertices.at(start_vertex);
			dst_vertex.pos = project(src_vertex.pos, model);
			dst_vertex.col = src_vertex.col;
			dst_vertex.tex = src_vertex.tex;
			start_vertex += 1;
		}

		pushBatchIndices(indices, vertices.size());
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

void System::drawCircle(const glm::mat4& model, int segments, const glm::vec4& color,
	float fill, float begin, float end)
{
	if (begin >= end)
		return;

	const float Radius = 0.5f;
	float increment = 2.0f * glm::pi<float>() / segments;

	float sinInc = glm::sin(increment);
	float cosInc = glm::cos(increment);

	auto r1 = glm::vec2({ sinInc, -cosInc });
	auto vertices = std::vector<Renderer::Vertex::PositionColor>();

	auto radius_inner = Radius * (1.0f - fill);
	auto radius_outer = Radius;

	auto delta_inner = Radius - radius_inner;
	auto delta_outer = radius_outer - Radius;

	auto v1_outer = radius_outer * r1;
	auto v1_inner = radius_inner * r1;

	for (int i = 0; i < segments; i++)
	{
		float progress = (float)(i + 1) / (float)segments;

		if (progress < begin)
			continue;

		if (progress > end)
			continue;

		auto r2 = glm::vec2({
			(cosInc * r1.x) - (sinInc * r1.y),
			(sinInc * r1.x) + (cosInc * r1.y)
		});

		auto v2_outer = radius_outer * r2;
		auto v2_inner = radius_inner * r2;

		auto p1 = glm::vec3({ v1_outer + radius_outer - delta_outer, 0.0f });
		auto p2 = glm::vec3({ v2_outer + radius_outer - delta_outer, 0.0f });
		auto p3 = glm::vec3({ v1_inner + radius_inner + delta_inner, 0.0f });
		auto p4 = glm::vec3({ v2_inner + radius_inner + delta_inner, 0.0f });

		vertices.push_back({ p1, color });
		vertices.push_back({ p2, color });
		vertices.push_back({ p3, color });

		vertices.push_back({ p3, color });
		vertices.push_back({ p2, color });
		vertices.push_back({ p4, color });

		r1 = r2;
		v1_outer = v2_outer;
		v1_inner = v2_inner;
	}

	draw(Renderer::Topology::TriangleList, vertices, model);
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

void System::drawSdf(Renderer::Topology topology, std::shared_ptr<Renderer::Texture> texture,
	const std::vector<Renderer::Vertex::PositionTexture>& vertices,
	const std::vector<uint32_t>& indices, float minValue, float maxValue,
	float smoothFactor, const glm::mat4 model, const glm::vec4& color)
{
	assert(mWorking);

	if (topology == Renderer::Topology::TriangleStrip)
	{
		auto new_indices = triangulate(topology, indices);
		drawSdf(Renderer::Topology::TriangleList, texture, vertices, new_indices, minValue, maxValue, smoothFactor, model, color);
		return;
	}

	if (stateWouldApplied())
	{
		flush();
		applyState();
	}

	if (mBatching)
	{
		if (mBatch.topology != topology ||
			mBatch.texture != texture ||
			mBatch.mode != BatchMode::Sdf ||
			mSdfShader.getMaxValue() != maxValue ||
			mSdfShader.getMinValue() != minValue ||
			mSdfShader.getSmoothFactor() != smoothFactor ||
			mSdfShader.getColor() != color)
		{
			flush();
		}

		mBatch.mode = BatchMode::Sdf;
		mBatch.texture = texture;
		mBatch.topology = topology;

		mSdfShader.setMaxValue(maxValue);
		mSdfShader.setMinValue(minValue);
		mSdfShader.setSmoothFactor(smoothFactor);
		mSdfShader.setColor(color);

		mBatch.verticesCount += vertices.size();

		if (mBatch.verticesCount > mBatch.positionTextureVertices.size())
			mBatch.positionTextureVertices.resize(mBatch.verticesCount);

		auto start_vertex = mBatch.verticesCount - vertices.size();

		for (const auto& src_vertex : vertices)
		{
			auto& dst_vertex = mBatch.positionTextureVertices.at(start_vertex);
			dst_vertex.pos = project(src_vertex.pos, model);
			dst_vertex.tex = src_vertex.tex;
			start_vertex += 1;
		}

		pushBatchIndices(indices, vertices.size());
	}
	else
	{
		flush();

		mSdfShader.setProjectionMatrix(mStates.top().projectionMatrix);
		mSdfShader.setViewMatrix(mStates.top().viewMatrix);
		mSdfShader.setModelMatrix(model);
		mSdfShader.setMinValue(minValue);
		mSdfShader.setMaxValue(maxValue);
		mSdfShader.setSmoothFactor(smoothFactor);
		mSdfShader.setColor(color);

		RENDERER->setTexture(*texture);
		RENDERER->setTopology(topology);
		RENDERER->setIndexBuffer(indices);
		RENDERER->setVertexBuffer(vertices);
		RENDERER->setSampler(mStates.top().sampler);
		RENDERER->setShader(mSdfShader);

		RENDERER->drawIndexed(indices.size());
	}
}

void System::drawString(const Font& font, const TextMesh& mesh, const glm::mat4& model,
	float minValue, float maxValue, float smoothFactor, const glm::vec4& color)
{
	assert(mWorking);
	drawSdf(mesh.topology, font.getTexture(), mesh.vertices, mesh.indices, minValue, 
		maxValue, smoothFactor, model, color);
}

void System::drawString(const Font& font, const TextMesh& mesh, const glm::mat4& model, float size,
	const glm::vec4& color, float outlineThickness, const glm::vec4& outlineColor)
{
	float fixedOutlineThickness = glm::lerp(0.0f, 0.75f, outlineThickness);

	const float min = 0.0f;
	const float mid = Font::SdfOnedge;
	const float max = 1.0f;
	const float outline = glm::lerp(mid, min, fixedOutlineThickness);
	float smoothFactor = 2.0f / size / PLATFORM->getScale();

	if (fixedOutlineThickness > 0.0f)
		drawString(font, mesh, model, outline, mid + (smoothFactor / 2.0f), smoothFactor, outlineColor);

	drawString(font, mesh, model, mid, max, smoothFactor, color);
}

void System::drawString(const Font& font, const utf8_string& text, const glm::mat4& model, float size,
	const glm::vec4& color, float outlineThickness, const glm::vec4& outlineColor)
{
	drawString(font, TextMesh::createSinglelineTextMesh(font, text), model, size, color, outlineThickness, outlineColor);
}

glm::vec3 System::project(const glm::vec3& pos, const glm::mat4& model)
{
	auto width = PLATFORM->getLogicalWidth();
	auto height = PLATFORM->getLogicalHeight();
	glm::vec4 viewport = { 0.0f, height, width, -height };
	return glm::project(pos, model, mStates.top().projectionMatrix, viewport);
}

void System::pushBatchIndices(const std::vector<uint32_t>& indices, size_t vertices_size)
{
	mBatch.indicesCount += indices.size();

	if (mBatch.indicesCount > mBatch.indices.size())
		mBatch.indices.resize(mBatch.indicesCount);

	auto start_index = mBatch.indicesCount - indices.size();

	for (auto src_index : indices)
	{
		auto& dst_index = mBatch.indices.at(start_index);
		dst_index = static_cast<uint32_t>(mBatch.verticesCount - vertices_size) + src_index;
		start_index += 1;
	}
}

std::vector<uint32_t> System::triangulate(Renderer::Topology topology, const std::vector<uint32_t>& indices)
{
	assert(topology == Renderer::Topology::TriangleStrip);

	std::vector<uint32_t> result;

	for (int i = 0; i < indices.size() - 2; i++)
	{
		if (i % 2) {
			result.push_back(indices.at(i + 1));
			result.push_back(indices.at(i));
			result.push_back(indices.at(i + 2));
		}
		else {
			result.push_back(indices.at(i));
			result.push_back(indices.at(i + 1));
			result.push_back(indices.at(i + 2));
		}
	}

	return result;
}

void System::push(const State& value)
{
	assert(mWorking);	
	mStates.push(value);
}

void System::pop(int count)
{
	assert(mWorking);
	assert(mStates.size() >= count);

	for (int i = 0; i < count; i++)
	{
		mStates.pop();
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