#pragma once

#include <scene/node.h>

namespace Scene
{
	class Transform3D
	{
	public:
		auto getScale() const { return mScale; }
		void setScale(const glm::vec3& value) { mScale = value; }

		auto getRotation() const { return mRotation; }
		void setRotation(const glm::vec3& value) { mRotation = value; }

		auto getPosition() const { return mPosition; }
		void setPosition(const glm::vec3& value) { mPosition = value; }

	private:
		glm::vec3 mScale = { 1.0f, 1.0f, 1.0f };
		glm::vec3 mRotation = { 0.0f, 0.0f, 0.0f };
		glm::vec3 mPosition = { 0.0f, 0.0f, 0.0f };

	public:
		void setScale(float value) { setScale({ value, value, value }); }
	};

	class Entity3D : public Transform3D
	{
	public:
		using ProvideModelsCallback = std::function<void(std::vector<skygfx::utils::Model>& models)>;

	public:
		void updateTransform();
		virtual void provideModels(std::vector<skygfx::utils::Model>& models);

	public:
		bool isEnabled() const { return mEnabled; }
		void setEnabled(bool value) { mEnabled = value; }

		const auto& getTransform() const { return mTransform; }
		void setProvideModelsCallback(ProvideModelsCallback value) { mProvideModelsCallback = value; }

	private:
		bool mEnabled = true;
		glm::mat4 mTransform = glm::mat4(1.0f);
		ProvideModelsCallback mProvideModelsCallback;
	};

	class SingleMeshEntity : public Entity3D
	{
	public:
		void provideModels(std::vector<skygfx::utils::Model>& models) override;

	public:
		auto getTopology() const { return mTopology; }
		void setTopology(skygfx::Topology value) { mTopology = value; }

		void setVertices(const std::vector<skygfx::utils::Mesh::Vertex>& vertices);
		void setIndices(const std::vector<skygfx::utils::Mesh::Index>& indices);

		const auto& getMesh() const { return mMesh; }

	private:
		skygfx::Topology mTopology = skygfx::Topology::TriangleList;
		skygfx::utils::Mesh mMesh;
	};

	class Viewport3D : public Node
	{
	public:
		Viewport3D();

		void addEntity(std::shared_ptr<Entity3D> model);
		void removeEntity(std::shared_ptr<Entity3D> model);

	protected:
		void update(Clock::Duration dTime) override;
		void draw() override;

	public:
		auto getCamera() const { return mCamera; }

		auto getTechnique() const { return mTechnique; }
		void setTechnique(skygfx::utils::DrawSceneOptions::Technique value) { mTechnique = value; }

	private:
		std::shared_ptr<skygfx::utils::PerspectiveCamera> mCamera;
		std::list<std::shared_ptr<Entity3D>> mEntities;
		skygfx::utils::DrawSceneOptions::Technique mTechnique = skygfx::utils::DrawSceneOptions::Technique::ForwardShading;
	};
}