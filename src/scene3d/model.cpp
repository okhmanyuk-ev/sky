#include "model.h"
#include <numeric>

using namespace Scene3D;

void Model::draw(Driver& driver)
{
	Node::draw(driver);
	
	static std::map<std::set<Renderer::Shaders::Light::Flag>, std::shared_ptr<Renderer::Shaders::Light>> shaders;

	if (!mVertices.has_value())
		std::tie(mVertices, mShaderFlags) = generateVertices();

	if (!shaders.contains(mShaderFlags))
		shaders[mShaderFlags] = std::make_shared<Renderer::Shaders::Light>(Vertex::Layout, mShaderFlags);
	
	auto shader = shaders.at(mShaderFlags);

	driver.prepareShader(*shader, mMaterial);

	if (mIndices.empty())
	{
		mIndices.resize(mPositionAttribs.size());
		std::iota(mIndices.begin(), mIndices.end(), 0);
	}

	GRAPHICS->pushModelMatrix(getTransform());
	GRAPHICS->drawGeneric(mTopology, mVertices.value(), mIndices, shader, mTexture);
	GRAPHICS->pop();
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