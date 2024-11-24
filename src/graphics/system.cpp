#include "system.h"
#include <vector>
#include <numeric>

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
	mFlushCountPublic = mFlushCount;
	mFlushCount = 0;

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
	flushBatch();
	pop();
	mWorking = false;
}

bool System::isSameBatch(const State& left, const State& right)
{
	return
		left.projection_matrix == right.projection_matrix &&
		left.view_matrix == right.view_matrix &&
		//	left.model_matrix == right.model_matrix && // we should not compare model matrix for fine batching
		left.render_target == right.render_target &&
		left.scissor == right.scissor &&
		left.viewport == right.viewport &&
		left.depth_mode == right.depth_mode &&
		left.cull_mode == right.cull_mode &&
		left.blend_mode == right.blend_mode &&
		left.sampler == right.sampler &&
		left.texture_address == right.texture_address &&
		left.stencil_mode == right.stencil_mode &&
		left.mipmap_bias == right.mipmap_bias;
}

void System::applyState()
{
	assert(!mStates.empty());

	const auto& state = mStates.top();

	if (mAppliedState.has_value() && isSameBatch(mAppliedState.value(), state))
		return;

	flushBatch();

	bool renderTargetChanged = true;

	if (mAppliedState.has_value())
	{
		const auto& applied_state = mAppliedState.value();
		renderTargetChanged = applied_state.render_target != state.render_target;
	}

	if (renderTargetChanged)
		RENDERER->setRenderTarget(state.render_target);

	mAppliedState = state;
}

void System::flushBatch()
{
	if (mBatch.vertices.empty())
		return;

	mFlushCount += 1;

	assert(mAppliedState.has_value());

	mBatch.mesh.setVertices(mBatch.vertices);
	mBatch.mesh.setIndices(mBatch.indices);

	mBatch.vertices.clear();
	mBatch.indices.clear();

	const auto& state = mAppliedState.value();

	float width;
	float height;

	if (state.viewport.has_value())
	{
		width = state.viewport->size.x;
		height = state.viewport->size.y;
	}
	else if (state.render_target)
	{
		width = static_cast<float>(state.render_target->getWidth());
		height = static_cast<float>(state.render_target->getHeight());
	}
	else
	{
		width = static_cast<float>(PLATFORM->getWidth());
		height = static_cast<float>(PLATFORM->getHeight());
	}

	auto scale = PLATFORM->getScale();

	width /= scale;
	height /= scale;

	auto [proj, view] = skygfx::utils::MakeCameraMatrices(skygfx::utils::OrthogonalCamera{
		.width = width,
		.height = height
	});

	skygfx::utils::ExecuteCommands({
		skygfx::utils::commands::SetTopology(mBatch.topology.value()),
		skygfx::utils::commands::SetProjectionMatrix(proj),
		skygfx::utils::commands::SetViewMatrix(view),
		skygfx::utils::commands::SetViewport(state.viewport),
		skygfx::utils::commands::SetScissor(state.scissor),
		skygfx::utils::commands::SetDepthMode(state.depth_mode),
		skygfx::utils::commands::SetStencilMode(state.stencil_mode),
		skygfx::utils::commands::SetCullMode(state.cull_mode),
		skygfx::utils::commands::SetBlendMode(state.blend_mode),
		skygfx::utils::commands::SetSampler(state.sampler),
		skygfx::utils::commands::SetTextureAddress(state.texture_address),
		skygfx::utils::commands::SetMipmapBias(state.mipmap_bias),
		skygfx::utils::commands::SetMesh(&mBatch.mesh),
		skygfx::utils::commands::SetColorTexture(mBatch.texture ? mBatch.texture.get() : nullptr),
		skygfx::utils::commands::DrawMesh()
	});
}

void System::clear(std::optional<glm::vec4> color, std::optional<float> depth, std::optional<uint8_t> stencil)
{
	applyState();
	RENDERER->clear(color, depth, stencil);
}

void System::draw(sky::effects::IEffect* effect, skygfx::Texture* texture, skygfx::Topology topology,
	const skygfx::utils::Mesh& mesh)
{
	applyState();
	flushBatch();

	const auto& state = mStates.top();

	std::vector<skygfx::utils::Command> cmds;

	if (effect != nullptr)
	{
		cmds.push_back(skygfx::utils::commands::SetEffect(effect->getShader(), effect->getUniformBinding(),
			effect->getUniformData(), effect->getUniformSize()));
	}

	cmds.insert(cmds.end(), {
		skygfx::utils::commands::SetTopology(topology),
		skygfx::utils::commands::SetViewport(state.viewport),
		skygfx::utils::commands::SetScissor(state.scissor),
		skygfx::utils::commands::SetDepthMode(state.depth_mode),
		skygfx::utils::commands::SetStencilMode(state.stencil_mode),
		skygfx::utils::commands::SetCullMode(state.cull_mode),
		skygfx::utils::commands::SetBlendMode(state.blend_mode),
		skygfx::utils::commands::SetSampler(state.sampler),
		skygfx::utils::commands::SetTextureAddress(state.texture_address),
		skygfx::utils::commands::SetProjectionMatrix(state.projection_matrix),
		skygfx::utils::commands::SetViewMatrix(state.view_matrix),
		skygfx::utils::commands::SetModelMatrix(state.model_matrix),
		skygfx::utils::commands::SetMesh(&mesh),
		skygfx::utils::commands::SetColorTexture(texture),
		skygfx::utils::commands::DrawMesh()
	});

	skygfx::utils::ExecuteCommands(cmds);
}

void System::draw(sky::effects::IEffect* effect, std::shared_ptr<skygfx::Texture> texture,
	skygfx::Topology topology, skygfx::utils::Mesh::Vertex* vertices, uint32_t vertex_count,
	skygfx::utils::Mesh::Index* indices, uint32_t index_count)
{
	if (!mBatching || vertex_count > 40 || effect != nullptr)
	{
		static skygfx::utils::Mesh mesh;
		mesh.setVertices(vertices, vertex_count);
		mesh.setIndices(indices, index_count);

		draw(effect, texture ? texture.get() : nullptr, topology, mesh);
		return;
	}

	applyState();

	if (mBatch.topology != topology || mBatch.texture != texture)
		flushBatch();

	mBatchesCount += 1;

	mBatch.texture = texture;
	mBatch.topology = topology;

	for (uint32_t i = 0; i < vertex_count; i++)
	{
		const auto& vertex = vertices[i];
		auto projected_pos = project(vertex.pos);
		mBatch.vertices.push_back(skygfx::utils::Mesh::Vertex{
			.pos = projected_pos,
			.color = vertex.color,
			.texcoord = vertex.texcoord
		});
	}

	for (uint32_t i = 0; i < index_count; i++)
	{
		auto index = indices[i];
		mBatch.indices.push_back(static_cast<uint32_t>(mBatch.vertices.size() - vertex_count) + index);
	}
}

void System::draw(sky::effects::IEffect* effect, std::shared_ptr<skygfx::Texture> texture,
	skygfx::Topology topology, const skygfx::utils::Mesh::Vertices& _vertices,
	const skygfx::utils::Mesh::Indices& _indices)
{
	auto vertices = (skygfx::utils::Mesh::Vertex*)_vertices.data();
	auto vertex_count = (uint32_t)_vertices.size();
	auto indices = (skygfx::utils::Mesh::Index*)_indices.data();
	auto index_count = (uint32_t)_indices.size();

	draw(effect, texture, topology, vertices, vertex_count, indices, index_count);
}

void System::draw(sky::effects::IEffect* effect, std::shared_ptr<skygfx::Texture> texture,
	skygfx::utils::MeshBuilder::Mode mode,
	std::function<void(std::function<void(const skygfx::utils::Mesh::Vertex&)>)> callback)
{
	static skygfx::utils::MeshBuilder mesh_builder;
	mesh_builder.reset();
	mesh_builder.begin(mode);
	callback([&](const skygfx::utils::Mesh::Vertex& vertex) {
		mesh_builder.vertex(vertex);
	});
	mesh_builder.end();
	assert(!mesh_builder.isBegan());

	auto topology = mesh_builder.getTopology().value();
	const auto& vertices = mesh_builder.getVertices();
	const auto& indices = mesh_builder.getIndices();

	draw(effect, texture, topology, vertices, indices);
}

void System::drawTexturedRectangle(sky::effects::IEffect* effect, std::shared_ptr<skygfx::Texture> texture,
	const glm::vec2& top_left_uv, const glm::vec2& top_right_uv,
	const glm::vec2& bottom_left_uv, const glm::vec2& bottom_right_uv,
	const glm::vec4& top_left_color, const glm::vec4& top_right_color,
	const glm::vec4& bottom_left_color, const glm::vec4& bottom_right_color)
{
	draw(effect, texture, skygfx::utils::MeshBuilder::Mode::TriangleStrip, [&](auto vertex) {
		vertex({ .pos = { 0.0f, 0.0f, 0.0f }, .color = top_left_color, .texcoord = top_left_uv });
		vertex({ .pos = { 0.0f, 1.0f, 0.0f }, .color = bottom_left_color, .texcoord = bottom_left_uv });
		vertex({ .pos = { 1.0f, 0.0f, 0.0f }, .color = top_right_color, .texcoord = top_right_uv });
		vertex({ .pos = { 1.0f, 1.0f, 0.0f }, .color = bottom_right_color, .texcoord = bottom_right_uv });
	});
}

void System::drawTexturedRectangle(sky::effects::IEffect* effect, std::shared_ptr<skygfx::Texture> texture,
	const TexRegion& region, const glm::vec4& top_left_color, const glm::vec4& top_right_color,
	const glm::vec4& bottom_left_color, const glm::vec4& bottom_right_color)
{
	float tex_w = static_cast<float>(texture->getWidth());
	float tex_h = static_cast<float>(texture->getHeight());

	float s_x1 = region.pos.x / tex_w;
	float s_y1 = region.pos.y / tex_h;
	float s_x2 = (region.size.x > 0.0f ? (region.size.x / tex_w) : 1.0f) + s_x1;
	float s_y2 = (region.size.y > 0.0f ? (region.size.y / tex_h) : 1.0f) + s_y1;

	glm::vec2 top_left_uv = { s_x1, s_y1 };
	glm::vec2 top_right_uv = { s_x2, s_y1 };
	glm::vec2 bottom_left_uv = { s_x1, s_y2 };
	glm::vec2 bottom_right_uv = { s_x2, s_y2 };

	drawTexturedRectangle(effect, texture, top_left_uv, top_right_uv, bottom_left_uv, bottom_right_uv,
		top_left_color, top_right_color, bottom_left_color, bottom_right_color);
}

void System::drawRectangle(sky::effects::IEffect* effect,
	const glm::vec4& top_left_color, const glm::vec4& top_right_color,
	const glm::vec4& bottom_left_color, const glm::vec4& bottom_right_color)
{
	drawTexturedRectangle(effect, nullptr, {}, {}, {}, {}, top_left_color,
		top_right_color, bottom_left_color, bottom_right_color);
}

void System::drawRectangle(sky::effects::IEffect* effect, const glm::vec4& color)
{
	drawRectangle(effect, color, color, color, color);
}

void System::drawRoundedRectangle(const glm::vec4& top_left_color, const glm::vec4& top_right_color,
	const glm::vec4& bottom_left_color, const glm::vec4& bottom_right_color, const glm::vec2& size,
	float rounding, bool absolute_rounding)
{
	static auto effect = sky::effects::Effect<sky::effects::Rounded>();
	effect.uniform.size = size;
	if (absolute_rounding)
	{
		effect.uniform.radius = glm::clamp(rounding, 0.0f, glm::min(size.x, size.y) / 2.0f);
	}
	else
	{
		effect.uniform.radius = (glm::clamp(rounding, 0.0f, 1.0f) * glm::min(size.x, size.y)) / 2.0f;
	}
	drawRectangle(&effect, top_left_color, top_right_color, bottom_left_color, bottom_right_color);
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

	pushSampler(skygfx::Sampler::Linear);
	drawSlicedSprite(nullptr, mWhiteCircleTexture, center_region, size, edge_size, color);
	pop();
}

void System::drawLineRectangle(const glm::vec4& color)
{
	draw(nullptr, nullptr, skygfx::utils::MeshBuilder::Mode::LineLoop, [&](auto vertex) {
		vertex({ .pos = { 0.0f, 0.0f, 0.0f }, .color = color });
		vertex({ .pos = { 0.0f, 1.0f, 0.0f }, .color = color });
		vertex({ .pos = { 1.0f, 1.0f, 0.0f }, .color = color });
		vertex({ .pos = { 1.0f, 0.0f, 0.0f }, .color = color });
	});
}

void System::drawCircle(const glm::vec4& inner_color, const glm::vec4& outer_color,
	float fill, float pie)
{
	static auto effect = sky::effects::Effect<sky::effects::Circle>();
	effect.uniform.fill = fill;
	effect.uniform.pie = pie;
	effect.uniform.inner_color = inner_color;
	effect.uniform.outer_color = outer_color;
	drawRectangle(&effect);
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

	auto radius_inner = Radius * (1.0f - fill);
	auto radius_outer = Radius;

	auto delta_inner = Radius - radius_inner;
	auto delta_outer = radius_outer - Radius;

	auto v1_outer = radius_outer * r1;
	auto v1_inner = radius_inner * r1;

	draw(nullptr, nullptr, skygfx::utils::MeshBuilder::Mode::Triangles, [&](auto vertex) {
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

			vertex({ .pos = p1, .color = outer_color });
			vertex({ .pos = p2, .color = outer_color });
			vertex({ .pos = p3, .color = inner_color });

			vertex({ .pos = p3, .color = inner_color });
			vertex({ .pos = p2, .color = outer_color });
			vertex({ .pos = p4, .color = inner_color });

			r1 = r2;
			v1_outer = v2_outer;
			v1_inner = v2_inner;
		}
	});
}

void System::drawCircleTexture(const glm::vec4& color)
{
	pushSampler(skygfx::Sampler::Linear);
	drawTexturedRectangle(nullptr, mWhiteCircleTexture, {}, color, color, color, color);
	pop();
}

void System::drawSlicedSprite(sky::effects::IEffect* effect, std::shared_ptr<skygfx::Texture> texture,
	const TexRegion& center_region, const glm::vec2& size, std::optional<float> edge_size,
	const glm::vec4& color)
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

	static auto vertices = skygfx::utils::Mesh::Vertices(36);

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

	draw(effect, texture, skygfx::Topology::TriangleList, vertices, indices);
}

void System::drawSdf(skygfx::Topology topology, std::shared_ptr<skygfx::Texture> texture,
	const skygfx::utils::Mesh::Vertices& vertices, const skygfx::utils::Mesh::Indices& indices,
	float minValue, float maxValue, float smoothFactor, const glm::vec4& color)
{
	assert(!vertices.empty());
	assert(!indices.empty());

	static auto effect = sky::effects::Effect<sky::effects::Sdf>();
	effect.uniform.min_value = minValue;
	effect.uniform.max_value = maxValue;
	effect.uniform.smooth_factor = smoothFactor;
	effect.uniform.color = color;

	draw(&effect, texture, topology, vertices, indices);
}

void System::drawString(const Font& font, const TextMesh& mesh, float minValue, float maxValue,
	float smoothFactor, const glm::vec4& color)
{
	assert(!mesh.vertices.empty());
	assert(!mesh.indices.empty());

	drawSdf(mesh.topology, font.getTexture(), mesh.vertices, mesh.indices, minValue,
		maxValue, smoothFactor, color);
}

void System::drawString(const Font& font, const TextMesh& mesh, float size, const glm::vec4& color,
	float outlineThickness, const glm::vec4& outlineColor, float smoothFactorScale)
{
	assert(!mesh.vertices.empty());
	assert(!mesh.indices.empty());

	float fixedOutlineThickness = glm::lerp(0.0f, 0.75f, outlineThickness);

	const float min = 0.0f;
	const float mid = Font::SdfOnedge;
	const float max = 1.0f;
	const float outline = glm::lerp(mid, min, fixedOutlineThickness);
	float smoothFactor = 2.0f / size / PLATFORM->getScale();

	smoothFactor *= mSdfSmoothFactor;
	smoothFactor *= smoothFactorScale;

	if (fixedOutlineThickness > 0.0f)
		drawString(font, mesh, outline, mid + (smoothFactor / 2.0f), smoothFactor, outlineColor);

	drawString(font, mesh, mid, max, smoothFactor, color);
}

void System::drawString(const Font& font, const std::wstring& text, float size, const glm::vec4& color,
	float outlineThickness, const glm::vec4& outlineColor, float smoothFactorScale)
{
	drawString(font, TextMesh::createSinglelineTextMesh(font, text), size, color, outlineThickness, outlineColor,
		smoothFactorScale);
}

glm::vec3 System::project(const glm::vec3& pos)
{
	const auto& state = getCurrentState();

	auto scale = PLATFORM->getScale();

	float width;
	float height;

	if (state.viewport.has_value())
	{
		width = state.viewport->size.x;
		height = state.viewport->size.y;
	}
	else if (state.render_target)
	{
		width = static_cast<float>(state.render_target->getWidth());
		height = static_cast<float>(state.render_target->getHeight());
	}
	else
	{
		width = static_cast<float>(PLATFORM->getWidth());
		height = static_cast<float>(PLATFORM->getHeight());
	}

	width /= scale;
	height /= scale;

	auto projected_pos = state.projection_matrix * state.view_matrix * state.model_matrix * glm::vec4(pos, 1.0f);

	projected_pos.x += 1.0f;
	projected_pos.y -= 1.0f;

	projected_pos.x *= width;
	projected_pos.y *= -height;

	projected_pos *= 0.5f;

	return projected_pos;
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

void System::pushSampler(skygfx::Sampler value)
{
	auto state = mStates.top();
	state.sampler = value;
	push(state);
}

void System::pushBlendMode(skygfx::BlendMode value)
{
	auto state = mStates.top();
	state.blend_mode = value;
	push(state);
}

void System::pushDepthMode(std::optional<skygfx::DepthMode> value)
{
	auto state = mStates.top();
	state.depth_mode = value;
	push(state);
}

void System::pushCullMode(skygfx::CullMode value)
{
	auto state = mStates.top();
	state.cull_mode = value;
	push(state);
}

void System::pushViewport(std::optional<skygfx::Viewport> value)
{
	auto state = mStates.top();
	state.viewport = value;
	push(state);
}

void System::pushRenderTarget(std::shared_ptr<skygfx::RenderTarget> value)
{
	auto state = mStates.top();
	state.render_target = value;
	push(state);
}

void System::pushScissor(std::optional<skygfx::Scissor> value, bool inherit_prev_scissor)
{
	auto state = mStates.top();
	if (inherit_prev_scissor && state.scissor.has_value())
	{
		if (value.has_value())
		{
			auto max_pos_prev_scissor = state.scissor->position + state.scissor->size;
			auto max_pos_new_scissor = value->position + value->size;

			glm::vec2 max_pos_final = {
				glm::min(max_pos_prev_scissor.x, max_pos_new_scissor.x),
				glm::min(max_pos_prev_scissor.y, max_pos_new_scissor.y),
			};

			state.scissor->position.x = glm::max(value->position.x, state.scissor->position.x);
			state.scissor->position.y = glm::max(value->position.y, state.scissor->position.y);

			state.scissor->size.x = max_pos_final.x - state.scissor->position.x;
			state.scissor->size.y = max_pos_final.y - state.scissor->position.y;
		}
	}
	else
	{
		state.scissor = value;
	}
	push(state);
}

void System::pushViewMatrix(const glm::mat4& value)
{
	auto state = mStates.top();
	state.view_matrix = value;
	push(state);
}

void System::pushProjectionMatrix(const glm::mat4& value)
{
	auto state = mStates.top();
	state.projection_matrix = value;
	push(state);
}

void System::pushModelMatrix(const glm::mat4& value)
{
	auto state = mStates.top();
	state.model_matrix = value;
	push(state);
}

void System::pushTextureAddress(skygfx::TextureAddress value)
{
	auto state = mStates.top();
	state.texture_address = value;
	push(state);
}

void System::pushOrthoMatrix(float width, float height)
{
	pushProjectionMatrix(glm::orthoLH(0.0f, width, height, 0.0f, -1.0f, 1.0f));
}

void System::pushOrthoMatrix(std::shared_ptr<skygfx::RenderTarget> target)
{
	if (target)
		pushOrthoMatrix((float)target->getWidth(), (float)target->getHeight());
	else
		pushOrthoMatrix(PLATFORM->getLogicalWidth(), PLATFORM->getLogicalHeight());
}

void System::pushStencilMode(std::optional<skygfx::StencilMode> value)
{
	auto state = mStates.top();
	state.stencil_mode = value;
	push(state);
}

void System::pushMipmapBias(float bias)
{
	auto state = mStates.top();
	state.mipmap_bias = bias;
	push(state);
}

void System::setBatching(bool value)
{
	if (!value && mBatching)
		flushBatch();

	mBatching = value;
}

std::shared_ptr<skygfx::RenderTarget> System::getRenderTarget(const std::string& name, uint32_t width, uint32_t height)
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
		auto target = std::make_shared<skygfx::RenderTarget>(width, height);
		mRenderTargets.insert({ name, target });
	}

	mUnusedRenderTargets.erase(name);

	return mRenderTargets.at(name);
}

std::shared_ptr<skygfx::RenderTarget> System::getRenderTarget(const std::string& name)
{
	return getRenderTarget(name, PLATFORM->getWidth(), PLATFORM->getHeight());
}

std::shared_ptr<skygfx::Texture> System::makeGenericTexture(const glm::ivec2& size, std::function<void()> callback)
{
	auto result = std::make_shared<skygfx::RenderTarget>(size.x, size.y);

	auto working = mWorking;

	if (!working)
		begin();

	pushCleanState();
	pushRenderTarget(result);
	pushOrthoMatrix(1.0f, 1.0f);
	clear(glm::vec4{ Graphics::Color::White, 0.0f });
	callback();
	pop(3);

	if (!working)
		end();

	return result;
}
