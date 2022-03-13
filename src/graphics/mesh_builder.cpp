#include "mesh_builder.h"

using namespace Graphics;

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
