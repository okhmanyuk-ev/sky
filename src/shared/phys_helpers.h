#pragma once

#include <graphics/all.h>
#include <box2d/box2d.h>
#include <scene/all.h>
#include <shared/touch_emulator.h>

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

		enum class Shape
		{
			Box,
			Circle
		};

	public:
		auto getType() const { return mType; }
		void setType(Type value) { mType = value; }
	
		auto getShape() const { return mShape; }
		void setShape(Shape value) { mShape = value; }

		auto isFixedRotation() const { return mFixedRotation; }
		void setFixedRotation(bool value) { mFixedRotation = value; }
		
		auto isBullet() const { return mBullet; }
		void setBullet(bool value) { mBullet = value; }

	private:
		Type mType = Type::Static;
		Shape mShape = Shape::Box;
		bool mFixedRotation = false;
		bool mBullet = false;
	
	public:
		auto getB2Fixture() const { return mB2Fixture; }
		void setB2Fixture(b2Fixture* value) { mB2Fixture = value; }

		auto getB2Body() const { return getB2Fixture()->GetBody(); }

	private:
		b2Fixture* mB2Fixture = nullptr;

	public:
		uint16_t getFilterCategoryBits() const;
		void setFilterCategoryBits(uint16_t value);

		void setFilterMaskBits(uint16_t value);
		void setFilterGroupIndex(int16_t value);

	public:
		auto getFilterLayer() const { return mFilterLayer; }
		void setFilterLayer(int value) { mFilterLayer = value; };
		
	private:
		int mFilterLayer = 0;

	public:
		void applyLinearImpulseToCenter(const glm::vec2& impulse, bool wake);
		void setGravityScale(float value);

	public:
		using ContactCallback = std::function<void(Entity&)>;

	public:
		auto getContactCallback() const { return mContactCallback; }
		void setContactCallback(ContactCallback value) { mContactCallback = value; }

	private:
		ContactCallback mContactCallback = nullptr;
	};

	class World : public Scene::Node,
		public Common::Event::Listenable<Shared::TouchEmulator::Event>
	{
	public:
		static inline const float Scale = 100.0f;

	public:
		World();
		~World();

	private:
		void onEvent(const Shared::TouchEmulator::Event& e) override;

	public:
		void update(Clock::Duration delta) override;
		void leaveDraw() override;

	public:
		void attach(std::shared_ptr<Node> node, AttachDirection attach_direction = AttachDirection::Back) override;
		void detach(std::shared_ptr<Node> node) override;

	public:
		auto& getB2World() { return mB2World; }

		auto isDebug() const { return mDebug; }
		void setDebug(bool value) { mDebug = value; }

		auto getShowStats() const { return mShowStats; }
		void setShowStats(bool value) { mShowStats = value; }

	private:
		b2World mB2World = b2World({ 0.0f, 10.0f });
		b2Body* mDummyBody = nullptr; // used for mouse moving
		b2MouseJoint* mMouseJoint = nullptr;
		Common::TimestepFixer mTimestepFixer;
		bool mDebug = false;
		bool mShowStats = false;

	private:
		class Draw : public b2Draw
		{
			void DrawPolygon(const b2Vec2* vertices, int32 vertexCount, const b2Color& color) override;
			void DrawSolidPolygon(const b2Vec2* vertices, int32 vertexCount, const b2Color& color) override;
			void DrawCircle(const b2Vec2& center, float radius, const b2Color& color) override;
			void DrawSolidCircle(const b2Vec2& center, float radius, const b2Vec2& axis, const b2Color& color) override;
			void DrawSegment(const b2Vec2& p1, const b2Vec2& p2, const b2Color& color) override;
			void DrawTransform(const b2Transform& xf) override;
			void DrawPoint(const b2Vec2& p, float size, const b2Color& color) override;
		};

		class ContactFilter : public b2ContactFilter
		{
			bool ShouldCollide(b2Fixture* fixtureA, b2Fixture* fixtureB) override;
		};

		class ContactListener : public b2ContactListener
		{
			void BeginContact(b2Contact* contact) override;
		};

	private:
		Draw mDraw;
		ContactFilter mContactFilter;
		ContactListener mContactListener;
	};
}