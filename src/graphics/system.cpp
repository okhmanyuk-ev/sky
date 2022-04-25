#include "system.h"
#include <vector>
#include <numeric>
#include <vulkan/vulkan.hpp>

using namespace Graphics;

System::System()
{
	mWhiteCircleTexture = makeGenericTexture({ 256, 256 }, [this] {
		drawCircle();
	});
}

void System::onFrame()
{
	mBatchesCountPublic = mBatchesCount;
	mBatchesCount = 0;

	for (const auto& name : mUnusedRenderTargets)
	{
		mRenderTargets.erase(name);
	}

	mUnusedRenderTargets.clear();

	for (const auto& [name, target] : mRenderTargets)
	{
		mUnusedRenderTargets.insert(name);
	}
}

void System::begin()
{
	assert(!mWorking);
	mWorking = true;
	pushCleanState();
	mAppliedState = std::nullopt;
}

void System::end()
{
	assert(mWorking);
	assert(mStates.size() == 1);
	applyState();
	flush();
	pop();
	mWorking = false;
}

void System::applyState()
{
	assert(!mStates.empty());

	const auto& state = mStates.top();

	if (mAppliedState.has_value() && mAppliedState.value() == state)
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
		const auto& applied_state = mAppliedState.value();

		if (applied_state.scissor.has_value() && state.scissor.has_value())
			scissorChanged = applied_state.scissor.value() != state.scissor.value();
		else if (!applied_state.scissor.has_value() && !state.scissor.has_value())
			scissorChanged = false;

		renderTargetChanged = applied_state.renderTarget != state.renderTarget;
		viewportChanged = applied_state.viewport != state.viewport;
		depthModeChanged = applied_state.depthMode != state.depthMode;
		cullModeChanged = applied_state.cullMode != state.cullMode;
		blendModeChanged = applied_state.blendMode != state.blendMode;
		samplerChanged = applied_state.sampler != state.sampler;
		textureAddressChanged = applied_state.textureAddress != state.textureAddress;
		stencilChanged = applied_state.stencilMode != state.stencilMode;
	}

	if (scissorChanged)
	{
		if (state.scissor.has_value())
			RENDERER->setScissor(state.scissor.value());
		else
			RENDERER->setScissor(nullptr);
	}
	
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

	auto view = glm::lookAtLH(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(0.0f, 1.0f, 0.0f));
	auto projection = glm::orthoLH(0.0f, state.viewport.size.x / scale, state.viewport.size.y / scale, 0.0f, -1.0f, 1.0f);
	auto model = glm::mat4(1.0f);

	shader_matrices->setProjectionMatrix(projection);
	shader_matrices->setViewMatrix(view);
	shader_matrices->setModelMatrix(model);

	RENDERER->setShader(mBatch.shader);
	RENDERER->setTexture(mBatch.texture.value_or(nullptr));
	RENDERER->setTopology(mBatch.topology.value());
	RENDERER->setIndexBuffer(mBatch.indices);
	RENDERER->setVertexBuffer(mBatch.vertices);

	RENDERER->drawIndexed(mBatch.indicesCount);

	mBatch.mode = BatchMode::None;
	mBatch.shader = nullptr;
	mBatch.verticesCount = 0;
	mBatch.indicesCount = 0;
}

void System::clear(std::optional<glm::vec4> color, std::optional<float> depth, std::optional<uint8_t> stencil)
{
	applyState();
	RENDERER->clear(color, depth, stencil);
}

void System::drawGeneric(Renderer::Topology topology, const Renderer::Buffer& vertices, 
	const Renderer::Buffer& indices, std::shared_ptr<Renderer::ShaderMatrices> shader,
	std::optional<std::shared_ptr<Renderer::Texture>> texture,
	std::optional<size_t> count, size_t start)
{
	assert(shader);

	applyState();
	flush();

	const auto& state = mStates.top();

	shader->setProjectionMatrix(state.projectionMatrix);
	shader->setViewMatrix(state.viewMatrix);
	shader->setModelMatrix(state.modelMatrix);

	RENDERER->setTexture(texture.value_or(nullptr));
	RENDERER->setTopology(topology);
	RENDERER->setIndexBuffer(indices);
	RENDERER->setVertexBuffer(vertices);
	RENDERER->setShader(std::dynamic_pointer_cast<Renderer::Shader>(shader));
	RENDERER->drawIndexed(count.value_or(indices.size / indices.stride), start);
}

void System::draw(Renderer::Topology topology, const std::vector<Renderer::Vertex::PositionColor>& vertices,
	std::optional<size_t> count, size_t start)
{
	auto indices = std::vector<uint32_t>(count.value_or(vertices.size() - start));
	std::iota(indices.begin(), indices.end(), start);
	draw(topology, vertices, indices);
}

void System::draw(Renderer::Topology topology, const std::vector<Renderer::Vertex::PositionColor>& vertices,
	const std::vector<uint32_t>& indices, std::shared_ptr<Renderer::ShaderMatrices> shader)
{
	assert(mWorking);
	
	applyState();

	if (mBatching && vertices.size() <= 40 && !shader)
	{
		if (mBatch.topology != topology || mBatch.mode != BatchMode::Colored)
			flush();
		else
			mBatchesCount += 1;

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
			dst_vertex.pos = project(src_vertex.pos);
			dst_vertex.col = src_vertex.col;
			start_vertex += 1;
		}

		pushBatchIndices(indices, vertices.size());
	}
	else
	{
		if (!shader)
			shader = mColoredShader;

		drawGeneric(topology, vertices, indices, shader);
	}
}

void System::draw(Renderer::Topology topology, std::shared_ptr<Renderer::Texture> texture,
	const std::vector<Renderer::Vertex::PositionColorTexture>& vertices,
	std::shared_ptr<Renderer::ShaderMatrices> shader)
{
	auto indices = std::vector<uint32_t>(vertices.size());
	std::iota(indices.begin(), indices.end(), 0);
	draw(topology, texture, vertices, indices, shader);
}

void System::draw(Renderer::Topology topology, std::shared_ptr<Renderer::Texture> texture, const std::vector<Renderer::Vertex::PositionColorTexture>& vertices,
	const std::vector<uint32_t>& indices, std::shared_ptr<Renderer::ShaderMatrices> shader)
{
	assert(mWorking);
	assert(texture);

	applyState();
	
	if (mBatching && vertices.size() <= 40 && !shader)
	{
		if (mBatch.topology != topology || mBatch.texture != texture || mBatch.mode != BatchMode::Textured)
			flush();
		else
			mBatchesCount += 1;

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
			dst_vertex.pos = project(src_vertex.pos);
			dst_vertex.col = src_vertex.col;
			dst_vertex.tex = src_vertex.tex;
			start_vertex += 1;
		}

		pushBatchIndices(indices, vertices.size());
	}
	else
	{
		if (!shader)
			shader = mTexturedShader;

		drawGeneric(topology, vertices, indices, shader, texture);
	}
}

void System::drawRectangle(const glm::vec4& top_left_color,
	const glm::vec4& top_right_color, const glm::vec4& bottom_left_color, const glm::vec4& bottom_right_color, 
	std::shared_ptr<Renderer::ShaderMatrices> shader)
{
	static auto vertices = std::vector<Renderer::Vertex::PositionColor>(4);

	vertices[0] = { { 0.0f, 0.0f, 0.0f }, top_left_color };
	vertices[1] = { { 0.0f, 1.0f, 0.0f }, bottom_left_color };
	vertices[2] = { { 1.0f, 1.0f, 0.0f }, bottom_right_color };
	vertices[3] = { { 1.0f, 0.0f, 0.0f }, top_right_color };

	static const std::vector<uint32_t> indices = { 0, 1, 2, 0, 2, 3 };

	draw(Renderer::Topology::TriangleList, vertices, indices, shader);
}

void System::drawRectangle(const glm::vec4& color,
	std::shared_ptr<Renderer::ShaderMatrices> shader)
{
	drawRectangle(color, color, color, color, shader);
}

void System::drawRectangle(std::shared_ptr<Renderer::ShaderMatrices> shader)
{
	drawRectangle({ Color::White, 1.0f }, shader);
}

void System::drawRoundedRectangle(const glm::vec4& top_left_color, const glm::vec4& top_right_color,
	const glm::vec4& bottom_left_color, const glm::vec4& bottom_right_color, const glm::vec2& size, float rounding, bool absolute_rounding)
{
	static auto shader = std::make_shared<Renderer::Shaders::Rounded>(Renderer::Vertex::PositionColor::Layout);
	shader->setSize(size);
	if (absolute_rounding)
	{
		shader->setRadius(glm::clamp(rounding, 0.0f, glm::min(size.x, size.y) / 2.0f));
	}
	else
	{
		shader->setRadius((glm::clamp(rounding, 0.0f, 1.0f) * glm::min(size.x, size.y)) / 2.0f);
	}
	drawRectangle(top_left_color, top_right_color, bottom_left_color, bottom_right_color, shader);
}

void System::drawRoundedRectangle(const glm::vec4& color,
	const glm::vec2& size, float rounding, bool absolute_rounding)
{
	drawRoundedRectangle(color, color, color, color, size, rounding, absolute_rounding);
}

void System::drawRoundedSlicedRectangle(const glm::vec4& color,
	const glm::vec2& size, float rounding, bool absolute_rounding)
{
	static Graphics::TexRegion center_region = { 
		{ 
			(mWhiteCircleTexture->getWidth() / 2.0f) - 1.0f,
			(mWhiteCircleTexture->getHeight() / 2.0f) - 1.0f
		}, 
		{ 
			2.0f, 2.0f 
		} 
	};

	float edge_size = 0.0f;

	if (absolute_rounding)
	{
		edge_size = glm::clamp(rounding, 0.0f, glm::min(size.x, size.y) / 2.0f);
	}
	else
	{
		edge_size = (glm::clamp(rounding, 0.0f, 1.0f) * glm::min(size.x, size.y)) / 2.0f;
	}

	pushSampler(Renderer::Sampler::Linear);
	drawSlicedSprite(mWhiteCircleTexture, center_region, size, edge_size, color);
	pop();
}

void System::drawLineRectangle(const glm::vec4& color)
{
	static auto vertices = std::vector<Renderer::Vertex::PositionColor>(4);

	vertices[0] = { { 0.0f, 0.0f, 0.0f }, color };
	vertices[1] = { { 0.0f, 1.0f, 0.0f }, color };
	vertices[2] = { { 1.0f, 1.0f, 0.0f }, color };
	vertices[3] = { { 1.0f, 0.0f, 0.0f }, color };
	
	static const std::vector<uint32_t> indices = { 0, 1, 1, 2, 2, 3, 3, 0 };

	draw(Renderer::Topology::LineList, vertices, indices);
}

void System::drawCircle(const glm::vec4& inner_color, const glm::vec4& outer_color, 
	float fill, float pie)
{
	static auto shader = std::make_shared<Renderer::Shaders::Circle>(Renderer::Vertex::PositionColor::Layout);
	shader->setFill(fill);
	shader->setPie(pie);
	shader->setInnerColor(inner_color);
	shader->setOuterColor(outer_color);
	drawRectangle(shader);
}

void System::drawSegmentedCircle(int segments, const glm::vec4& inner_color,
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

	draw(Renderer::Topology::TriangleList, vertices);
}

void System::drawCircleTexture(const glm::vec4& color)
{
	pushSampler(Renderer::Sampler::Linear);
	drawSprite(mWhiteCircleTexture, {}, color);
	pop();
}

void System::drawSprite(std::shared_ptr<Renderer::Texture> texture, const glm::vec2 & top_left_uv,
	const glm::vec2& top_right_uv, const glm::vec2& bottom_left_uv, const glm::vec2& bottom_right_uv,
	const glm::vec4& color, std::shared_ptr<Renderer::ShaderMatrices> shader)
{
	static auto vertices = std::vector<Renderer::Vertex::PositionColorTexture>(4);

	vertices[0] = { { 0.0f, 0.0f, 0.0f }, color, top_left_uv };
	vertices[1] = { { 0.0f, 1.0f, 0.0f }, color, bottom_left_uv };
	vertices[2] = { { 1.0f, 1.0f, 0.0f }, color, bottom_right_uv };
	vertices[3] = { { 1.0f, 0.0f, 0.0f }, color, top_right_uv };

	static const std::vector<uint32_t> indices = { 0, 1, 2, 0, 2, 3 };

	draw(Renderer::Topology::TriangleList, texture, vertices, indices, shader);
}

void System::drawSprite(std::shared_ptr<Renderer::Texture> texture,
	const TexRegion& tex_region, const glm::vec4& color, std::shared_ptr<Renderer::ShaderMatrices> shader)
{
	float tex_w = static_cast<float>(texture->getWidth());
	float tex_h = static_cast<float>(texture->getHeight());

	float s_x1 = tex_region.pos.x / tex_w;
	float s_y1 = tex_region.pos.y / tex_h;
	float s_x2 = (tex_region.size.x > 0.0f ? (tex_region.size.x / tex_w) : 1.0f) + s_x1;
	float s_y2 = (tex_region.size.y > 0.0f ? (tex_region.size.y / tex_h) : 1.0f) + s_y1;

	glm::vec2 top_left_uv = { s_x1, s_y1 };
	glm::vec2 top_right_uv = { s_x2, s_y1 };
	glm::vec2 bottom_left_uv = { s_x1, s_y2 };
	glm::vec2 bottom_right_uv = { s_x2, s_y2 };

	drawSprite(texture, top_left_uv, top_right_uv, bottom_left_uv, bottom_right_uv, color, shader);
}

void System::drawSprite(std::shared_ptr<Renderer::Texture> texture, std::shared_ptr<Renderer::ShaderMatrices> shader)
{
	drawSprite(texture, { }, { Color::White, 1.0f }, shader);
}

void System::drawSlicedSprite(std::shared_ptr<Renderer::Texture> texture, 
	const TexRegion& center_region, const glm::vec2& size, std::optional<float> edge_size,
	const glm::vec4& color, std::shared_ptr<Renderer::ShaderMatrices> shader)
{
	glm::vec2 tex_size = { static_cast<float>(texture->getWidth()), static_cast<float>(texture->getHeight()) };
	
	glm::vec2 s1 = center_region.pos / tex_size;
	glm::vec2 s2 = (center_region.size / tex_size) + s1;

	glm::vec2 p1 = center_region.pos / size;
	glm::vec2 p2 = 1.0f - ((tex_size - (center_region.pos + center_region.size)) / size);

	if (edge_size.has_value())
	{
		p1 = edge_size.value() / size;
		p2 = (size - edge_size.value()) / size;
	}

	static auto vertices = std::vector<Renderer::Vertex::PositionColorTexture>(36);

	// top left

	vertices[0] = { { 0.0f, 0.0f, 0.0f }, color, { 0.0f, 0.0f } };
	vertices[1] = { { 0.0f, p1.y, 0.0f }, color, { 0.0f, s1.y } };
	vertices[2] = { { p1.x, p1.y, 0.0f }, color, { s1.x, s1.y } };
	vertices[3] = { { p1.x, 0.0f, 0.0f }, color, { s1.x, 0.0f } };
	
	// top center

	vertices[4] = { { p1.x, 0.0f, 0.0f }, color, { s1.x, 0.0f } };
	vertices[5] = { { p1.x, p1.y, 0.0f }, color, { s1.x, s1.y } };
	vertices[6] = { { p2.x, p1.y, 0.0f }, color, { s2.x, s1.y } };
	vertices[7] = { { p2.x, 0.0f, 0.0f }, color, { s2.x, 0.0f } };

	// top right

	vertices[8] = { { p2.x, 0.0f, 0.0f }, color, { s2.x, 0.0f } };
	vertices[9] = { { p2.x, p1.y, 0.0f }, color, { s2.x, s1.y } };
	vertices[10] = { { 1.0f, p1.y, 0.0f }, color, { 1.0f, s1.y } };
	vertices[11] = { { 1.0f, 0.0f, 0.0f }, color, { 1.0f, 0.0f } };

	// center left

	vertices[12] = { { 0.0f, p1.y, 0.0f }, color, { 0.0f, s1.y } };
	vertices[13] = { { 0.0f, p2.y, 0.0f }, color, { 0.0f, s2.y } };
	vertices[14] = { { p1.x, p2.y, 0.0f }, color, { s1.x, s2.y } };
	vertices[15] = { { p1.x, p1.y, 0.0f }, color, { s1.x, s1.y } };

	// center

	vertices[16] = { { p1.x, p1.y, 0.0f }, color, { s1.x, s1.y } };
	vertices[17] = { { p1.x, p2.y, 0.0f }, color, { s1.x, s2.y } };
	vertices[18] = { { p2.x, p2.y, 0.0f }, color, { s2.x, s2.y } };
	vertices[19] = { { p2.x, p1.y, 0.0f }, color, { s2.x, s1.y } };

	// center right

	vertices[20] = { { p2.x, p1.y, 0.0f }, color, { s2.x, s1.y } };
	vertices[21] = { { p2.x, p2.y, 0.0f }, color, { s2.x, s2.y } };
	vertices[22] = { { 1.0f, p2.y, 0.0f }, color, { 1.0f, s2.y } };
	vertices[23] = { { 1.0f, p1.y, 0.0f }, color, { 1.0f, s1.y } };

	// bottom left

	vertices[24] = { { 0.0f, p2.y, 0.0f }, color, { 0.0f, s2.y } };
	vertices[25] = { { 0.0f, 1.0f, 0.0f }, color, { 0.0f, 1.0f } };
	vertices[26] = { { p1.x, 1.0f, 0.0f }, color, { s1.x, 1.0f } };
	vertices[27] = { { p1.x, p2.y, 0.0f }, color, { s1.x, s2.y } };

	// bottom center

	vertices[28] = { { p1.x, p2.y, 0.0f }, color, { s1.x, s2.y } };
	vertices[29] = { { p1.x, 1.0f, 0.0f }, color, { s1.x, 1.0f } };
	vertices[30] = { { p2.x, 1.0f, 0.0f }, color, { s2.x, 1.0f } };
	vertices[31] = { { p2.x, p2.y, 0.0f }, color, { s2.x, s2.y } };

	// bottom right

	vertices[32] = { { p2.x, p2.y, 0.0f }, color, { s2.x, s2.y } };
	vertices[33] = { { p2.x, 1.0f, 0.0f }, color, { s2.x, 1.0f } };
	vertices[34] = { { 1.0f, 1.0f, 0.0f }, color, { 1.0f, 1.0f } };
	vertices[35] = { { 1.0f, p2.y, 0.0f }, color, { 1.0f, s2.y } };

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

	draw(Renderer::Topology::TriangleList, texture, vertices, indices, shader);
}

void System::drawSdf(Renderer::Topology topology, std::shared_ptr<Renderer::Texture> texture,
	const std::vector<Renderer::Vertex::PositionColorTexture>& vertices,
	const std::vector<uint32_t>& indices, float minValue, float maxValue,
	float smoothFactor, const glm::vec4& color)
{
	static auto shader = std::make_shared<Renderer::Shaders::Sdf>(Renderer::Vertex::PositionColorTexture::Layout);
	shader->setMinValue(minValue);
	shader->setMaxValue(maxValue);
	shader->setSmoothFactor(smoothFactor);
	shader->setColor(color);

	draw(topology, texture, vertices, indices, shader);
}

void System::drawString(const Font& font, const TextMesh& mesh, float minValue, float maxValue, 
	float smoothFactor, const glm::vec4& color)
{
	drawSdf(mesh.topology, font.getTexture(), mesh.vertices, mesh.indices, minValue, 
		maxValue, smoothFactor, color);
}

void System::drawString(const Font& font, const TextMesh& mesh, float size,
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
		drawString(font, mesh, outline, mid + (smoothFactor / 2.0f), smoothFactor, outlineColor);

	drawString(font, mesh, mid, max, smoothFactor, color);
}

void System::drawString(const Font& font, const utf8_string& text, float size,
	const glm::vec4& color, float outlineThickness, const glm::vec4& outlineColor)
{
	drawString(font, TextMesh::createSinglelineTextMesh(font, text), size, color, outlineThickness, outlineColor);
}

glm::vec3 System::project(const glm::vec3& pos)
{
	const auto& state = getCurrentState();
	assert(state == mAppliedState.value());

	auto scale = PLATFORM->getScale();

	auto width = state.viewport.size.x / scale;
	auto height = state.viewport.size.y / scale;

	auto projected_pos = state.projectionMatrix * state.viewMatrix * state.modelMatrix * glm::vec4(pos, 1.0f);

	projected_pos.x += 1.0f;
	projected_pos.y -= 1.0f;

	projected_pos.x *= width;
	projected_pos.y *= -height;

	projected_pos *= 0.5f;

	return projected_pos;
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

void System::pushCleanState()
{
	push(State{});
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

void System::pushCullMode(Renderer::CullMode value) 
{
	auto state = mStates.top();
	state.cullMode = value;
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

void System::pushModelMatrix(const glm::mat4& value)
{
	auto state = mStates.top();
	state.modelMatrix = value;
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
	pushProjectionMatrix(glm::orthoLH(0.0f, width, height, 0.0f, -1.0f, 1.0f));
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

std::shared_ptr<Renderer::RenderTarget> System::getRenderTarget(const std::string& name, int width, int height)
{
	if (mRenderTargets.contains(name))
	{
		auto target = mRenderTargets.at(name);

		if (target->getWidth() != width || target->getHeight() != height)
		{
			mRenderTargets.erase(name);
		}
	}

	if (!mRenderTargets.contains(name)) 
	{
		auto target = std::make_shared<Renderer::RenderTarget>(width, height);
		mRenderTargets.insert({ name, target });
	}

	mUnusedRenderTargets.erase(name);
	
	return mRenderTargets.at(name);
}

std::shared_ptr<Renderer::RenderTarget> System::getRenderTarget(const std::string& name)
{
	return getRenderTarget(name, PLATFORM->getWidth(), PLATFORM->getHeight());
}

std::shared_ptr<Renderer::Texture> System::makeGenericTexture(const glm::ivec2& size, std::function<void()> callback)
{
	auto result = std::make_shared<Renderer::RenderTarget>(size.x, size.y);

	auto working = mWorking;

	if (!working)
		begin();

	pushCleanState();
	pushRenderTarget(result);
	pushViewport(result);
	pushOrthoMatrix(1.0f, 1.0f);
	clear(glm::vec4{ Graphics::Color::White, 0.0f });
	callback();
	pop(4);

	if (!working)
		end();

	return result;
}
