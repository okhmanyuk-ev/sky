#pragma once

#include <graphics/all.h>
#include <box2d/box2d.h>
#include <scene/all.h>

namespace Shared::PhysHelpers
{
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

	class Entity
	{
	public:
		enum class Type
		{
			Static,
			Dynamic
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

		auto isFixedRotation() const { return mFixedRotation; }
		void setFixedRotation(bool value) { mFixedRotation = value; }

		auto isEnabled() const { return mEnabled; }
		void setEnabled(bool value) { mEnabled = value; }

	private:
		Type mType = Type::Static;
		glm::vec2 mPosition = { 0.0f, 0.0f };
		glm::vec2 mSize = { 0.0f, 0.0f };
		bool mFixedRotation = false;
		bool mEnabled = true;

	public:
		void update();

	public:
		auto getNode() const { return mNode; }

	private:
		std::shared_ptr<Scene::Node> mNode;

	public:
		auto getB2Body() const { return mB2Body; }
		void setB2Body(b2Body* value) { mB2Body = value; }

	private:
		b2Body* mB2Body;
	};

	class World
	{
	public:
		static inline const float Scale = 100.0f;

	public:
		World();

	public:
		void update(Clock::Duration delta);
		void draw();

	public:
		void addEntity(std::shared_ptr<Entity> entity);
		void removeEntity(std::shared_ptr<Entity> entity);

	private:
		b2World mB2World = b2World({ 0.0f, 10.0f });
		Common::TimestepFixer mTimestepFixer;

	public:
		auto getNode() const { return mNode; }

	private:
		std::shared_ptr<Scene::Node> mNode;
		std::set<std::shared_ptr<Entity>> mEntities;

	private:
		Graphics::Camera2D mCamera;
		PhysDraw mPhysDraw;
	};
}