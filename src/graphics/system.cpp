#include "system.h"

#include <vector>
#include <numeric>

using namespace Graphics;

void System::begin()
{
	assert(!mWorking);
	mWorking = true;
	mStates.push(State());
	mAppliedState = std::nullopt;
}

void System::end()
{
	assert(mWorking);
	assert(mStates.size() == 1);
	mWorking = false;
	applyState();
	flush();
	mStates.pop();
}

void System::applyState()
{
	assert(!mStates.empty());

	const auto& state = mStates.top();

	if (mAppliedState.has_value() && state == mAppliedState.value())
		return;

	flush();

	bool scissorChanged = true;
	bool renderTargetChanged = true;
	bool depthModeChanged = true;
	bool cullModeChanged = true;
	bool viewportChanged = true;
	bool blendModeChanged = true;
	bool samplerChanged = true;
	bool textureAddressChanged = true;
	bool stencilChanged = true;

	if (mAppliedState.has_value())
	{
		if (mAppliedState->scissor.has_value() && state.scissor.has_value())
			scissorChanged = mAppliedState->scissor.value() != state.scissor.value();
		else if (!mAppliedState->scissor.has_value() && !state.scissor.has_value())
			scissorChanged = false;

		renderTargetChanged = mAppliedState->renderTarget != state.renderTarget;
		depthModeChanged = mAppliedState->depthMode != state.depthMode;
		cullModeChanged = mAppliedState->cullMode != state.cullMode;
		viewportChanged = mAppliedState->viewport != state.viewport;
		blendModeChanged = mAppliedState->blendMode != state.blendMode;
		samplerChanged = mAppliedState->sampler != state.sampler;
		textureAddressChanged = mAppliedState->textureAddress != state.textureAddress;
		stencilChanged = mAppliedState->stencilMode != state.stencilMode;
	}

	if (scissorChanged)
		if (state.scissor.has_value())
			RENDERER->setScissor(state.scissor.value());
		else
			RENDERER->setScissor(nullptr);
	
	if (renderTargetChanged)
		RENDERER->setRenderTarget(state.renderTarget);

	if (depthModeChanged)
		RENDERER->setDepthMode(state.depthMode);
	
	if (cullModeChanged)
		RENDERER->setCullMode(state.cullMode);
	
	if (viewportChanged)
		RENDERER->setViewport(state.viewport);
	
	if (blendModeChanged)
		RENDERER->setBlendMode(state.blendMode);

	if (samplerChanged)
		RENDERER->setSampler(state.sampler);
	
	if (textureAddressChanged)
		RENDERER->setTextureAddressMode(state.textureAddress);
	
	if (stencilChanged)
		RENDERER->setStencilMode(state.stencilMode);

	mAppliedState = state;
}

void System::flush()
{
	if (mBatch.mode == BatchMode::None)
		return;

	assert(mAppliedState.has_value());

	const auto& state = mAppliedState.value();
	auto scale = PLATFORM->getScale();
	
	auto shader_matrices = std::dynamic_pointer_cast<Renderer::ShaderMatrices>(mBatch.shader);
	assert(shader_matrices);

	shader_matrices->setProjectionMatrix(glm::orthoLH(0.0f, state.viewport.size.x / scale, state.viewport.size.y / scale, 0.0f, -1.0f, 1.0f));
	shader_matrices->setViewMatrix(glm::lookAtLH(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(0.0f, 1.0f, 0.0f)));
	shader_matrices->setModelMatrix(glm::mat4(1.0f));

	RENDERER->setShader(mBatch.shader);

	if (mBatch.texture.has_value())
		RENDERER->setTexture(mBatch.texture.value());

	RENDERER->setTopology(mBatch.topology.value());
	RENDERER->setIndexBuffer(mBatch.indices);
	RENDERER->setVertexBuffer(mBatch.vertices);

	RENDERER->drawIndexed(mBatch.indicesCount);

	mBatch.mode = BatchMode::None;
	mBatch.shader = nullptr;
	mBatch.verticesCount = 0;
	mBatch.indicesCount = 0;
}

void System::clear(const glm::vec4& color)
{
	applyState();
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
	const std::vector<uint32_t>& indices, const glm::mat4& model, std::shared_ptr<Renderer::ShaderMatrices> shader)
{
	assert(mWorking);
	
	applyState();

	if (mBatching && vertices.size() <= 4 && !shader)
	{
		if (mBatch.topology != topology || mBatch.mode != BatchMode::Colored)
			flush();

		mBatch.mode = BatchMode::Colored;
		mBatch.shader = mBatchColorShader;
		mBatch.topology = topology;
		mBatch.verticesCount += vertices.size();

		if (mBatch.verticesCount > mBatch.vertices.size())
			mBatch.vertices.resize(mBatch.verticesCount);

		auto start_vertex = mBatch.verticesCount - vertices.size();

		for (const auto& src_vertex : vertices)
		{
			auto& dst_vertex = mBatch.vertices.at(start_vertex);
			dst_vertex.pos = project(src_vertex.pos, model);
			dst_vertex.col = src_vertex.col;
			start_vertex += 1;
		}

		pushBatchIndices(indices, vertices.size());
	}
	else
	{
		flush();

		if (!shader)
			shader = mColoredShader;

		const auto& state = mStates.top();

		shader->setProjectionMatrix(state.projectionMatrix);
		shader->setViewMatrix(state.viewMatrix);
		shader->setModelMatrix(model);

		RENDERER->setTopology(topology);
		RENDERER->setIndexBuffer(indices);
		RENDERER->setVertexBuffer(vertices);
		RENDERER->setShader(std::dynamic_pointer_cast<Renderer::Shader>(shader));

		RENDERER->drawIndexed(indices.size());
	}
}

void System::draw(Renderer::Topology topology, std::shared_ptr<Renderer::Texture> texture,
	const std::vector<Renderer::Vertex::PositionColorTexture>& vertices,
	const glm::mat4& model, std::shared_ptr<Renderer::ShaderMatrices> shader)
{
	auto indices = std::vector<uint32_t>(vertices.size());
	std::iota(indices.begin(), indices.end(), 0);
	draw(topology, texture, vertices, indices, model, shader);
}

void System::draw(Renderer::Topology topology, std::shared_ptr<Renderer::Texture> texture, const std::vector<Renderer::Vertex::PositionColorTexture>& vertices,
	const std::vector<uint32_t>& indices, const glm::mat4& model, std::shared_ptr<Renderer::ShaderMatrices> shader)
{
	assert(mWorking);

	applyState();
	
	if (mBatching && vertices.size() <= 4 && !shader)
	{
		if (mBatch.topology != topology || mBatch.texture != texture || mBatch.mode != BatchMode::Textured)
			flush();

		mBatch.mode = BatchMode::Textured;
		mBatch.shader = mTexturedShader;
		mBatch.texture = texture;
		mBatch.topology = topology;
		mBatch.verticesCount += vertices.size();

		if (mBatch.verticesCount > mBatch.vertices.size())
			mBatch.vertices.resize(mBatch.verticesCount);

		auto start_vertex = mBatch.verticesCount - vertices.size();

		for (const auto& src_vertex : vertices)
		{
			auto& dst_vertex = mBatch.vertices.at(start_vertex);
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

		if (!shader)
			shader = mTexturedShader;

		const auto& state = mStates.top();

		shader->setProjectionMatrix(state.projectionMatrix);
		shader->setViewMatrix(state.viewMatrix);
		shader->setModelMatrix(model);

		RENDERER->setTexture(texture);
		RENDERER->setTopology(topology);
		RENDERER->setIndexBuffer(indices);
		RENDERER->setVertexBuffer(vertices);
		RENDERER->setShader(std::dynamic_pointer_cast<Renderer::Shader>(shader));

		RENDERER->drawIndexed(indices.size());
	}
}

void System::drawRectangle(const glm::mat4& model, const glm::vec4& color,
	std::shared_ptr<Renderer::ShaderMatrices> shader)
{
	static auto vertices = std::vector<Renderer::Vertex::PositionColor>(4);
		
	vertices[0] = { { 0.0f, 0.0f, 0.0f }, color };
	vertices[1] = { { 0.0f, 1.0f, 0.0f }, color };
	vertices[2] = { { 1.0f, 1.0f, 0.0f }, color };
	vertices[3] = { { 1.0f, 0.0f, 0.0f }, color };
	
	static const std::vector<uint32_t> indices = { 0, 1, 2, 0, 2, 3 };

	draw(Renderer::Topology::TriangleList, vertices, indices, model, shader);
}

void System::drawLineRectangle(const glm::mat4& model, const glm::vec4& color)
{
	static auto vertices = std::vector<Renderer::Vertex::PositionColor>(4);

	vertices[0] = { { 0.0f, 0.0f, 0.0f }, color };
	vertices[1] = { { 0.0f, 1.0f, 0.0f }, color };
	vertices[2] = { { 1.0f, 1.0f, 0.0f }, color };
	vertices[3] = { { 1.0f, 0.0f, 0.0f }, color };
	
	static const std::vector<uint32_t> indices = { 0, 1, 1, 2, 2, 3, 3, 0 };

	draw(Renderer::Topology::LineList, vertices, indices, model);
}

void System::drawCircle(const glm::mat4& model, const glm::vec4& inner_color, const glm::vec4& outer_color, 
	float fill, float begin, float end)
{
	mCircleShader->setFill(fill);
	mCircleShader->setBegin(begin);
	mCircleShader->setEnd(end);
	mCircleShader->setInnerColor(inner_color);
	mCircleShader->setOuterColor(outer_color);
	drawRectangle(model, { Color::White, 1.0f }, mCircleShader);
}

void System::drawSegmentedCircle(const glm::mat4& model, int segments, const glm::vec4& inner_color,
	const glm::vec4& outer_color, float fill)
{
	assert(segments >= 3);

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

		vertices.push_back({ p1, outer_color });
		vertices.push_back({ p2, outer_color });
		vertices.push_back({ p3, inner_color });

		vertices.push_back({ p3, inner_color });
		vertices.push_back({ p2, outer_color });
		vertices.push_back({ p4, inner_color });

		r1 = r2;
		v1_outer = v2_outer;
		v1_inner = v2_inner;
	}

	draw(Renderer::Topology::TriangleList, vertices, model);
}

void System::drawSprite(std::shared_ptr<Renderer::Texture> texture, const glm::mat4& model,
	const TexRegion& tex_region, const glm::vec4& color, std::shared_ptr<Renderer::ShaderMatrices> shader)
{
	float tex_w = static_cast<float>(texture->getWidth());
	float tex_h = static_cast<float>(texture->getHeight());

	float s_x1 = tex_region.pos.x / tex_w;
	float s_y1 = tex_region.pos.y / tex_h;
	float s_x2 = (tex_region.size.x > 0.0f ? (tex_region.size.x / tex_w) : 1.0f) + s_x1;
	float s_y2 = (tex_region.size.y > 0.0f ? (tex_region.size.y / tex_h) : 1.0f) + s_y1;

	static auto vertices = std::vector<Renderer::Vertex::PositionColorTexture>(4);

	vertices[0] = { { 0.0f, 0.0f, 0.0f }, color, { s_x1, s_y1 } };
	vertices[1] = { { 0.0f, 1.0f, 0.0f }, color, { s_x1, s_y2 } };
	vertices[2] = { { 1.0f, 1.0f, 0.0f }, color, { s_x2, s_y2 } };
	vertices[3] = { { 1.0f, 0.0f, 0.0f }, color, { s_x2, s_y1 } };
	
	static const std::vector<uint32_t> indices = { 0, 1, 2, 0, 2, 3 };
	
	draw(Renderer::Topology::TriangleList, texture, vertices, indices, model, shader);
}

void System::drawSprite(std::shared_ptr<Renderer::Texture> texture, const glm::mat4& model,
	std::shared_ptr<Renderer::ShaderMatrices> shader)
{
	drawSprite(texture, model, { }, { Color::White, 1.0f }, shader);
}

void System::drawSlicedSprite(std::shared_ptr<Renderer::Texture> texture, const glm::mat4& model,
	const TexRegion& center_region, const glm::vec2& size, const glm::vec4& color, std::shared_ptr<Renderer::ShaderMatrices> shader)
{
	float tex_w = static_cast<float>(texture->getWidth());
	float tex_h = static_cast<float>(texture->getHeight());

	float s_x1 = center_region.pos.x / tex_w;
	float s_y1 = center_region.pos.y / tex_h;
	float s_x2 = (center_region.size.x / tex_w) + s_x1;
	float s_y2 = (center_region.size.y / tex_h) + s_y1;

	float p_x1 = center_region.pos.x / size.x;
	float p_y1 = center_region.pos.y / size.y;
	float p_x2 = 1.0f - ((tex_w - (center_region.pos.x + center_region.size.x)) / size.x);
	float p_y2 = 1.0f - ((tex_h - (center_region.pos.y + center_region.size.y)) / size.y);

	static auto vertices = std::vector<Renderer::Vertex::PositionColorTexture>(36);

	// top left

	vertices[0] = { { 0.0f, 0.0f, 0.0f }, color, { 0.0f, 0.0f } };
	vertices[1] = { { 0.0f, p_y1, 0.0f }, color, { 0.0f, s_y1 } };
	vertices[2] = { { p_x1, p_y1, 0.0f }, color, { s_x1, s_y1 } };
	vertices[3] = { { p_x1, 0.0f, 0.0f }, color, { s_x1, 0.0f } };
	
	// top center

	vertices[4] = { { p_x1, 0.0f, 0.0f }, color, { s_x1, 0.0f } };
	vertices[5] = { { p_x1, p_y1, 0.0f }, color, { s_x1, s_y1 } };
	vertices[6] = { { p_x2, p_y1, 0.0f }, color, { s_x2, s_y1 } };
	vertices[7] = { { p_x2, 0.0f, 0.0f }, color, { s_x2, 0.0f } };

	// top right

	vertices[8] = { { p_x2, 0.0f, 0.0f }, color, { s_x2, 0.0f } };
	vertices[9] = { { p_x2, p_y1, 0.0f }, color, { s_x2, s_y1 } };
	vertices[10] = { { 1.0f, p_y1, 0.0f }, color, { 1.0f, s_y1 } };
	vertices[11] = { { 1.0f, 0.0f, 0.0f }, color, { 1.0f, 0.0f } };

	// center left

	vertices[12] = { { 0.0f, p_y1, 0.0f }, color, { 0.0f, s_y1 } };
	vertices[13] = { { 0.0f, p_y2, 0.0f }, color, { 0.0f, s_y2 } };
	vertices[14] = { { p_x1, p_y2, 0.0f }, color, { s_x1, s_y2 } };
	vertices[15] = { { p_x1, p_y1, 0.0f }, color, { s_x1, s_y1 } };

	// center

	vertices[16] = { { p_x1, p_y1, 0.0f }, color, { s_x1, s_y1 } };
	vertices[17] = { { p_x1, p_y2, 0.0f }, color, { s_x1, s_y2 } };
	vertices[18] = { { p_x2, p_y2, 0.0f }, color, { s_x2, s_y2 } };
	vertices[19] = { { p_x2, p_y1, 0.0f }, color, { s_x2, s_y1 } };

	// center right

	vertices[20] = { { p_x2, p_y1, 0.0f }, color, { s_x2, s_y1 } };
	vertices[21] = { { p_x2, p_y2, 0.0f }, color, { s_x2, s_y2 } };
	vertices[22] = { { 1.0f, p_y2, 0.0f }, color, { 1.0f, s_y2 } };
	vertices[23] = { { 1.0f, p_y1, 0.0f }, color, { 1.0f, s_y1 } };

	// bottom left

	vertices[24] = { { 0.0f, p_y2, 0.0f }, color, { 0.0f, s_y2 } };
	vertices[25] = { { 0.0f, 1.0f, 0.0f }, color, { 0.0f, 1.0f } };
	vertices[26] = { { p_x1, 1.0f, 0.0f }, color, { s_x1, 1.0f } };
	vertices[27] = { { p_x1, p_y2, 0.0f }, color, { s_x1, s_y2 } };

	// bottom center

	vertices[28] = { { p_x1, p_y2, 0.0f }, color, { s_x1, s_y2 } };
	vertices[29] = { { p_x1, 1.0f, 0.0f }, color, { s_x1, 1.0f } };
	vertices[30] = { { p_x2, 1.0f, 0.0f }, color, { s_x2, 1.0f } };
	vertices[31] = { { p_x2, p_y2, 0.0f }, color, { s_x2, s_y2 } };

	// bottom right

	vertices[32] = { { p_x2, p_y2, 0.0f }, color, { s_x2, s_y2 } };
	vertices[33] = { { p_x2, 1.0f, 0.0f }, color, { s_x2, 1.0f } };
	vertices[34] = { { 1.0f, 1.0f, 0.0f }, color, { 1.0f, 1.0f } };
	vertices[35] = { { 1.0f, p_y2, 0.0f }, color, { 1.0f, s_y2 } };

	static const std::vector<uint32_t> indices = { 
		0, 1, 2, 0, 2, 3,
		4, 5, 6, 4, 6, 7,
		8, 9, 10, 8, 10, 11,
		12, 13, 14, 12, 14, 15,
		16, 17, 18, 16, 18, 19,
		20, 21, 22, 20, 22, 23,
		24, 25, 26, 24, 26, 27,
		28, 29, 30, 28, 30, 31,
		32, 33, 34, 32, 34, 35
	};

	draw(Renderer::Topology::TriangleList, texture, vertices, indices, model, shader);
}

void System::drawSdf(Renderer::Topology topology, std::shared_ptr<Renderer::Texture> texture,
	const std::vector<Renderer::Vertex::PositionColorTexture>& vertices,
	const std::vector<uint32_t>& indices, float minValue, float maxValue,
	float smoothFactor, const glm::mat4& model, const glm::vec4& color)
{
	mSdfShader->setMinValue(minValue);
	mSdfShader->setMaxValue(maxValue);
	mSdfShader->setSmoothFactor(smoothFactor);
	mSdfShader->setColor(color);
	draw(topology, texture, vertices, indices, model, mSdfShader);	
}

void System::drawString(const Font& font, const TextMesh& mesh, const glm::mat4& model,
	float minValue, float maxValue, float smoothFactor, const glm::vec4& color)
{
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

	smoothFactor *= mSdfSmoothFactor;

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
	const auto& state = mStates.top();
	
	assert(state == mAppliedState);
	
	auto scale = PLATFORM->getScale();

	auto width = state.viewport.size.x / scale;
	auto height = state.viewport.size.y / scale;

	glm::vec4 viewport = { 0.0f, height, width, -height }; // TODO: { 0.0f, 0.0f, width, height }
		
	auto view = state.viewMatrix;
	auto proj = state.projectionMatrix;

	return glm::project(pos, view * model, proj, viewport);
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

void System::pushSampler(Renderer::Sampler value)
{
	auto state = mStates.top();
	state.sampler = value;
	push(state);
}

void System::pushBlendMode(Renderer::BlendMode value)
{
	auto state = mStates.top();
	state.blendMode = value;
	push(state);
}

void System::pushDepthMode(Renderer::DepthMode value)
{
	auto state = mStates.top();
	state.depthMode = value;
	push(state);
}

void System::pushViewport(const Renderer::Viewport& value)
{
	auto state = mStates.top();
	state.viewport = value;
	push(state);
}

void System::pushViewport(std::shared_ptr<Renderer::RenderTarget> target)
{
	pushViewport(Renderer::Viewport(target));
}

void System::pushRenderTarget(std::shared_ptr<Renderer::RenderTarget> value)
{
	auto state = mStates.top();
	state.renderTarget = value;
	push(state);
}

void System::pushScissor(std::optional<Renderer::Scissor> value)
{
	auto state = mStates.top();
	state.scissor = value;
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

void System::pushTextureAddress(Renderer::TextureAddress value)
{
	auto state = mStates.top();
	state.textureAddress = value;
	push(state);
}

void System::pushOrthoMatrix(float width, float height)
{
	auto state = mStates.top();
	state.projectionMatrix = glm::orthoLH(0.0f, width, height, 0.0f, -1.0f, 1.0f);
	push(state);
}

void System::pushOrthoMatrix(std::shared_ptr<Renderer::RenderTarget> target)
{
	if (target)
		pushOrthoMatrix((float)target->getWidth(), (float)target->getHeight());
	else
		pushOrthoMatrix(PLATFORM->getLogicalWidth(), PLATFORM->getLogicalHeight());
}

void System::pushStencilMode(const Renderer::StencilMode& value)
{
	auto state = mStates.top();
	state.stencilMode = value;
	push(state);
}

void System::setBatching(bool value) 
{ 
	if (!value && mBatching) 
		flush(); 
	
	mBatching = value; 
}
