#include "model.h"
#include <numeric>

using namespace Scene3D;

void Model::draw(Renderer::Technique& technique)
{
	Node::draw(technique);

	if (!mVertices.has_value())
		std::tie(mVertices, mShaderFlags) = generateVertices(); // TODO: shader flags is unused

	if (mIndices.empty())
	{
		mIndices.resize(mPositionAttribs.size());
		std::iota(mIndices.begin(), mIndices.end(), 0);
	}

	technique.setModelMatrix(getTransform());
	technique.setMaterial(mMaterial);
	
	if (std::holds_alternative<Renderer::Technique::TexturesMap>(mTexturesMap))
	{
		auto textures_map = std::get<Renderer::Technique::TexturesMap>(mTexturesMap);
		technique.draw(mVertices.value(), mIndices, Vertex::Layout, textures_map);
	}
	else
	{
		auto texture = std::get<std::shared_ptr<Renderer::Texture>>(mTexturesMap);
		auto index_range = Renderer::Technique::IndexRange();
		index_range.count = mIndices.size();
		technique.draw(mVertices.value(), mIndices, Vertex::Layout, { { texture, index_range } });
	}
}

std::tuple<std::vector<Model::Vertex>, std::set<Renderer::Shaders::Light::Flag>> Model::generateVertices()
{
	std::vector<Vertex> result;

	auto count = mPositionAttribs.size();

	bool has_col = mColorAttribs.size() == count;
	bool has_tex = mTexCoordAttribs.size() == count;
	bool has_normal = mNormalAttribs.size() == count;

	assert(has_normal); // normals should be

	for (int i = 0; i < count; i++)
	{
		Vertex vertex;
		vertex.pos = mPositionAttribs.at(i);

		if (has_col)
			vertex.col = mColorAttribs.at(i);
		
		if (has_tex)
			vertex.tex = mTexCoordAttribs.at(i);
		
		if (has_normal)
			vertex.normal = mNormalAttribs.at(i);
		
		result.push_back(vertex);
	}

	std::set<Renderer::Shaders::Light::Flag> flags;

	if (has_col)
		flags.insert(Renderer::Shaders::Light::Flag::Colored);

	if (has_tex)
		flags.insert(Renderer::Shaders::Light::Flag::Textured);

	return { result, flags };
}

void Model::setVertices(const std::vector<Renderer::Vertex::PositionColorNormal>& vertices)
{
	mPositionAttribs.clear();
	mColorAttribs.clear();
	mNormalAttribs.clear();

	for (const auto& vertex : vertices)
	{
		mPositionAttribs.push_back(vertex.pos);
		mColorAttribs.push_back(vertex.col);
		mNormalAttribs.push_back(vertex.normal);
	}
}

void Model::setVertices(const std::vector<Renderer::Vertex::PositionTextureNormal>& vertices)
{
	mPositionAttribs.clear();
	mTexCoordAttribs.clear();
	mNormalAttribs.clear();

	for (const auto& vertex : vertices)
	{
		mPositionAttribs.push_back(vertex.pos);
		mTexCoordAttribs.push_back(vertex.tex);
		mNormalAttribs.push_back(vertex.normal);
	}
}

void Model::setVertices(const std::vector<Renderer::Vertex::PositionColorTextureNormal>& vertices)
{
	mPositionAttribs.clear();
	mTexCoordAttribs.clear();
	mNormalAttribs.clear();
	mColorAttribs.clear();

	for (const auto& vertex : vertices)
	{
		mPositionAttribs.push_back(vertex.pos);
		mTexCoordAttribs.push_back(vertex.tex);
		mNormalAttribs.push_back(vertex.normal);
		mColorAttribs.push_back(vertex.col);
	}
}

void Model::setPositionAttribs(const PositionAttribs& value)
{
	if (mPositionAttribs != value)
		mVertices.reset();

	mPositionAttribs = value;
}

void Model::setColorAttribs(const ColorAttribs& value)
{
	if (mColorAttribs != value)
		mVertices.reset();

	mColorAttribs = value;
}

void Model::setNormalAttribs(const NormalAttribs& value)
{
	if (mNormalAttribs != value)
		mVertices.reset();

	mNormalAttribs = value;
}

void Model::setTexCoordAttribs(const TexCoordAttribs& value)
{
	if (mTexCoordAttribs != value)
		mVertices.reset();

	mTexCoordAttribs = value;
}