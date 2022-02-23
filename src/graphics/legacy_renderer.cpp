#include "legacy_renderer.h"

using namespace Graphics;

void LegacyRenderer::begin(Renderer::Topology topology)
{
	assert(!mWorking);
	mWorking = true;
	mTopology = topology;
}

void LegacyRenderer::vertex(const glm::vec2& value)
{
	vertex({ value.x, value.y, 0.0f });
}

void LegacyRenderer::vertex(const glm::vec3& value)
{
	assert(mWorking);
	
	if (mVertexCount + 1 > mVertices.size())
		mVertices.resize(mVertexCount + 1);

	auto& vertex = mVertices[mVertexCount];

	vertex.pos = value;
	vertex.col = mColor;

	mVertexCount += 1;
}

void LegacyRenderer::color(const glm::vec3& value)
{
	color({ value.r, value.g, value.b, 1.0f });
}

void LegacyRenderer::color(const glm::vec4& value)
{
	assert(mWorking);
	mColor = value;
}

void LegacyRenderer::end()
{
	assert(mWorking);
	mWorking = false;

	mShader->setProjectionMatrix(mProjectionMatrix);
	mShader->setViewMatrix(mViewMatrix);
	mShader->setModelMatrix(mModelMatrix);
 
	RENDERER->setViewport(Renderer::Viewport());
	RENDERER->setScissor(nullptr);
	RENDERER->setTopology(mTopology);
	RENDERER->setVertexBuffer({ mVertices.data(), mVertexCount });
	RENDERER->setShader(mShader);
	
	RENDERER->draw(static_cast<int>(mVertexCount));

	mVertexCount = 0;
}

void MeshBuilder::begin()
{
	assert(!mWorking);
	mWorking = true;
	mVertexCount = 0;
}

void MeshBuilder::vertex(const glm::vec2& value)
{
	vertex({ value.x, value.y, 0.0f });
}

void MeshBuilder::vertex(const glm::vec3& value)
{
	assert(mWorking);

	if (mVertexCount + 1 > mVertices.size())
		mVertices.resize(mVertexCount + 1);

	auto& vertex = mVertices[mVertexCount];

	vertex.pos = value;
	vertex.col = mColor;

	mVertexCount += 1;
}

void MeshBuilder::color(const glm::vec3& value)
{
	color({ value.r, value.g, value.b, 1.0f });
}

void MeshBuilder::color(const glm::vec4& value)
{
	assert(mWorking);
	mColor = value;
}

std::tuple<std::vector<Renderer::Vertex::PositionColor>, size_t> MeshBuilder::end()
{
	assert(mWorking);
	mWorking = false;
	return { mVertices, mVertexCount };
}
