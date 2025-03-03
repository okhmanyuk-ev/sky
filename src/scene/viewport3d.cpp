#include "viewport3d.h"

using namespace Scene;

void Entity3D::updateTransform()
{
	//mTransform = hasParent() ? getParent()->getTransform() : glm::mat4(1.0f);
	mTransform = glm::mat4(1.0f);
	mTransform = glm::translate(mTransform, getPosition());
	mTransform = glm::rotate(mTransform, getRotation().x, { 1.0f, 0.0f, 0.0f });
	mTransform = glm::rotate(mTransform, getRotation().y, { 0.0f, 1.0f, 0.0f });
	mTransform = glm::rotate(mTransform, getRotation().z, { 0.0f, 0.0f, 1.0f });
	mTransform = glm::scale(mTransform, getScale());
}

void Entity3D::provideModels(std::vector<skygfx::utils::Model>& models)
{
	if (mProvideModelsCallback)
		mProvideModelsCallback(models);
}

void SingleMeshEntity::provideModels(std::vector<skygfx::utils::Model>& models)
{
	Entity3D::provideModels(models);

	auto model = skygfx::utils::Model();
	model.topology = mTopology;
	model.mesh = &mMesh;
	model.matrix = getTransform();
	model.depth_mode = skygfx::DepthMode(skygfx::ComparisonFunc::Less);
	models.push_back(model);
}

void SingleMeshEntity::setVertices(const std::vector<skygfx::utils::Mesh::Vertex>& vertices)
{
	mMesh.setVertices(vertices);
}

void SingleMeshEntity::setIndices(const std::vector<skygfx::utils::Mesh::Index>& indices)
{
	mMesh.setIndices(indices);
}

Viewport3D::Viewport3D()
{
	mCamera = std::make_shared<skygfx::utils::PerspectiveCamera>();
}

void Viewport3D::addEntity(std::shared_ptr<Entity3D> entity)
{
	mEntities.push_back(entity);
}

void Viewport3D::removeEntity(std::shared_ptr<Entity3D> entity)
{
	mEntities.remove(entity);
}

void Viewport3D::update(sky::Duration dTime)
{
	Node::update(dTime);
	for (auto& entity : mEntities)
	{
		entity->updateTransform();
	}
}

void Viewport3D::draw()
{
	Node::draw();

	std::vector<skygfx::utils::Model> models;

	for (auto& entity : mEntities)
	{
		if (!entity->isEnabled())
			continue;

		entity->provideModels(models);
	}

	skygfx::utils::DrawSceneOptions options;
	options.technique = mTechnique;

	skygfx::utils::DirectionalLight light;
	light.direction = { -0.5f, -0.5f, -0.5f };

	skygfx::utils::DrawScene(nullptr, *mCamera, models, { light }, options);
}