#include "model.h"

using namespace Scene3D;

void Model::draw(Driver& driver)
{
	Node::draw(driver);
	
	static std::shared_ptr<Renderer::Shaders::Light> shader = nullptr;

	if (!mVertices.has_value())
	{
		std::set<Renderer::Shaders::Light::Flag> flags;
		std::tie(mVertices, flags) = generateVertices();

		if (mShaderFlags != flags)
		{
			shader = std::make_shared<Renderer::Shaders::Light>(Vertex::Layout, flags);
			mShaderFlags = flags;
		}
	}

	assert(shader);
	driver.prepareShader(*shader, mMaterial);

	GRAPHICS->drawGeneric(mTopology, mVertices.value(), mIndices, getTransform(), shader, mTexture);
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

void Model::setVertices(std::vector<Renderer::Vertex::PositionColorNormal> vertices)
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

void Model::setVertices(std::vector<Renderer::Vertex::PositionTextureNormal> vertices)
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

void Model::setPositionAttribs(const std::vector<PositionAttrib>& value)
{
	if (mPositionAttribs != value)
		mVertices.reset();

	mPositionAttribs = value;
}

void Model::setColorAttribs(const std::vector<ColorAttrib>& value)
{
	if (mColorAttribs != value)
		mVertices.reset();

	mColorAttribs = value;
}

void Model::setNormalAttribs(const std::vector<NormalAttrib>& value)
{
	if (mNormalAttribs != value)
		mVertices.reset();

	mNormalAttribs = value;
}

void Model::setTexCoordAttribs(const std::vector<TexCoordAttrib>& value)
{
	if (mTexCoordAttribs != value)
		mVertices.reset();

	mTexCoordAttribs = value;
}