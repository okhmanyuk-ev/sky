#pragma once

#include <graphics/all.h>
#include <box2d/box2d.h>
#include <scene/all.h>

namespace Shared::PhysHelpers
{
	class Entity
	{
	public:
		enum class Type
		{
			Static,
			Dynamic,
			Kinematic
		};

	public:
		Entity();

	public:
		auto getType() const { return mType; }
		void setType(Type type) { mType = type; }

		auto getPosition() const { return mPosition; }
		void setPosition(const glm::vec2& value) { mPosition = value; }
		void setPosition(float value) { setPosition({ value, value }); }

		auto getSize() const { return mSize; }
		void setSize(const glm::vec2& value) { mSize = value; }
		void setSize(float value) { setSize({ value, value }); }

		auto getPivot() const { return mPivot; }
		void setPivot(const glm::vec2& value) { mPivot = value; }
		void setPivot(float value) { setPivot({ value, value }); }

		auto isFixedRotation() const { return mFixedRotation; }
		void setFixedRotation(bool value) { mFixedRotation = value; }

		auto isEnabled() const { return mEnabled; }
		void setEnabled(bool value) { mEnabled = value; }

	private:
		Type mType = Type::Static;
		glm::vec2 mPosition = { 0.0f, 0.0f };
		glm::vec2 mSize = { 0.0f, 0.0f };
		glm::vec2 mPivot = { 0.0f, 0.0f };
		bool mFixedRotation = false;
		bool mEnabled = true;

	public:
		void update();

	public:
		auto getNode() const { return mNode; }

	private:
		std::shared_ptr<Scene::Node> mNode;

	public:
		auto getB2Fixture() const { return mB2Fixture; }
		void setB2Fixture(b2Fixture* value) { mB2Fixture = value; }

		auto getB2Body() const { return getB2Fixture()->GetBody(); }

	private:
		b2Fixture* mB2Fixture = nullptr;
	};

	class World : public Scene::Node
	{
	public:
		static inline const float Scale = 100.0f;

	public:
		World();

	public:
		void update(Clock::Duration delta) override;
		void leaveDraw() override;

	public:
		void addEntity(std::shared_ptr<Entity> entity, Scene::Node::AttachDirection node_attach_direction = Scene::Node::AttachDirection::Back);
		void removeEntity(std::shared_ptr<Entity> entity);
		bool hasEntity(std::shared_ptr<Entity> entity) const;

	public:
		auto& getB2World() { return mB2World; }
		
		auto isPhysDrawEnabled() const { return mPhysDrawEnabled; }
		void setPhysDrawEnabled(bool value) { mPhysDrawEnabled = value; }

	private:
		b2World mB2World = b2World({ 0.0f, 10.0f });
		Common::TimestepFixer mTimestepFixer;
		std::set<std::shared_ptr<Entity>> mEntities;
		bool mPhysDrawEnabled = true;

	private:
		class PhysDraw : public b2Draw
		{
		public:
			void DrawPolygon(const b2Vec2* vertices, int32 vertexCount, const b2Color& color) override;
			void DrawSolidPolygon(const b2Vec2* vertices, int32 vertexCount, const b2Color& color) override;
			void DrawCircle(const b2Vec2& center, float radius, const b2Color& color) override;
			void DrawSolidCircle(const b2Vec2& center, float radius, const b2Vec2& axis, const b2Color& color) override;
			void DrawSegment(const b2Vec2& p1, const b2Vec2& p2, const b2Color& color) override;
			void DrawTransform(const b2Transform& xf) override;
			void DrawPoint(const b2Vec2& p, float size, const b2Color& color) override;
		};

	private:
		PhysDraw mPhysDraw;
	};
}