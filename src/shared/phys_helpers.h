#pragma once

#include <graphics/all.h>
#include <box2d/box2d.h>
#include <scene/all.h>

namespace Shared::PhysHelpers
{
	class Entity : public Scene::Node
	{
	public:
		enum class Type
		{
			Static,
			Dynamic,
			Kinematic
		};

	public:
		auto isFixedRotation() const { return mFixedRotation; }
		void setFixedRotation(bool value) { mFixedRotation = value; }

		auto getType() const { return mType; }
		void setType(Type value) { mType = value; }
	
		auto isBullet() const { return mBullet; }
		void setBullet(bool value) { mBullet = value; }

	private:
		Type mType = Type::Static;
		bool mFixedRotation = false;
		bool mBullet = false;
	
	public:
		auto getB2Fixture() const { return mB2Fixture; }
		void setB2Fixture(b2Fixture* value) { mB2Fixture = value; }

		auto getB2Body() const { return getB2Fixture()->GetBody(); }

	private:
		b2Fixture* mB2Fixture = nullptr;

	public:
		void setFilterCategoryBits(uint16_t value);
		void setFilterMaskBits(uint16_t value);
		void setFilterGroupIndex(int16_t value);
		
	public:
		void applyLinearImpulseToCenter(const glm::vec2& impulse, bool wake);
		void setGravityScale(float value);
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
		void attach(std::shared_ptr<Node> node, AttachDirection attach_direction = AttachDirection::Back) override;
		void detach(std::shared_ptr<Node> node) override;

	public:
		auto& getB2World() { return mB2World; }
		
		auto isPhysDrawEnabled() const { return mPhysDrawEnabled; }
		void setPhysDrawEnabled(bool value) { mPhysDrawEnabled = value; }

	private:
		b2World mB2World = b2World({ 0.0f, 10.0f });
		Common::TimestepFixer mTimestepFixer;
		bool mPhysDrawEnabled = false;

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