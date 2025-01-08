#include "phys_helpers.h"
#include <console/system.h>
#include <common/console_commands.h>
#include <shared/stats_system.h>

using namespace Shared::PhysHelpers;

inline b2BodyType EntTypeToB2Type(Entity::Type type)
{
	if (type == Entity::Type::Static)
		return b2_staticBody;
	else if (type == Entity::Type::Dynamic)
		return b2_dynamicBody;
	else
		return b2_kinematicBody;
}

// entity

uint16_t Entity::getFilterCategoryBits() const
{
	return getB2Fixture()->GetFilterData().categoryBits;
}

void Entity::setFilterCategoryBits(uint16_t value)
{
	auto filter = getB2Fixture()->GetFilterData();
	filter.categoryBits = value;
	getB2Fixture()->SetFilterData(filter);
}

void Entity::setFilterMaskBits(uint16_t value)
{
	auto filter = getB2Fixture()->GetFilterData();
	filter.maskBits = value;
	getB2Fixture()->SetFilterData(filter);
}

void Entity::setFilterGroupIndex(int16_t value)
{
	auto filter = getB2Fixture()->GetFilterData();
	filter.groupIndex = value;
	getB2Fixture()->SetFilterData(filter);
}

void Entity::applyLinearImpulseToCenter(const glm::vec2& impulse, bool wake)
{
	getB2Body()->ApplyLinearImpulseToCenter({ impulse.x, impulse.y }, wake);
}

void Entity::setGravityScale(float value)
{
	getB2Body()->SetGravityScale(value);
}

// world

World::World()
{
	mTimestepFixer.setForceTimeCompletion(false);
	mTimestepFixer.setTimestep(sky::FromSeconds(1.0f / 120.0f));

	mB2World.SetContactFilter(&mContactFilter);
	mB2World.SetContactListener(&mContactListener);
	mB2World.SetDebugDraw(&mDraw);
	mDraw.SetFlags(
		b2Draw::e_shapeBit
		| b2Draw::e_jointBit
	//	| b2Draw::e_aabbBit
	//	| b2Draw::e_pairBit
	//	| b2Draw::e_centerOfMassBit
	);

	b2BodyDef dummy_body_def;
	mDummyBody = mB2World.CreateBody(&dummy_body_def);

	CONSOLE->registerCVar("phys_debug", { "bool" }, CVAR_GETTER_BOOL_FUNC(isDebug), CVAR_SETTER_BOOL_FUNC(setDebug));
	CONSOLE->registerCVar("phys_stats", { "bool" }, CVAR_GETTER_BOOL_FUNC(getShowStats), CVAR_SETTER_BOOL_FUNC(setShowStats));
	CONSOLE->registerCVar("phys_allow_sleep", { "bool" }, CVAR_GETTER_BOOL_FUNC(mB2World.GetAllowSleeping), CVAR_SETTER_BOOL_FUNC(mB2World.SetAllowSleeping));
	CONSOLE->registerCVar("phys_velocity_iterations", { "int" }, CVAR_GETTER_INT(mVelocityIterations), CVAR_SETTER_INT(mVelocityIterations));
	CONSOLE->registerCVar("phys_position_iterations", { "int" }, CVAR_GETTER_INT(mPositionIterations), CVAR_SETTER_INT(mPositionIterations));

	auto getter = [this] {
		auto fps = 1.0f / sky::ToSeconds(mTimestepFixer.getTimestep());
		return std::vector<std::string>({ std::to_string(fps) });
	};

	auto setter = [this](CON_ARGS) {
		auto sec = std::stof(CON_ARG(0));
		mTimestepFixer.setTimestep(sky::FromSeconds(1.0f / sec));
	};

	CONSOLE->registerCVar("phys_timestep_fps", { "float" }, getter, setter);

	CONSOLE->registerCVar("phys_timestep_enabled", { "bool" },
		CVAR_GETTER_BOOL_FUNC(mTimestepFixer.isEnabled),
		CVAR_SETTER_BOOL_FUNC(mTimestepFixer.setEnabled));

	CONSOLE->registerCVar("phys_timestep_force_time_completion", { "bool" },
		CVAR_GETTER_BOOL_FUNC(mTimestepFixer.getForceTimeCompletion),
		CVAR_SETTER_BOOL_FUNC(mTimestepFixer.setForceTimeCompletion));
}

World::~World()
{
	CONSOLE->removeCVar("phys_debug");
	CONSOLE->removeCVar("phys_stats");
	CONSOLE->removeCVar("phys_allow_sleep");
	CONSOLE->removeCVar("phys_velocity_iterations");
	CONSOLE->removeCVar("phys_position_iterations");
	CONSOLE->removeCVar("phys_timestep_fps");
	CONSOLE->removeCVar("phys_timestep_enabled");
	CONSOLE->removeCVar("phys_timestep_force_time_completion");
}

void World::onEvent(const Shared::TouchEmulator::Event& e)
{
	class QueryCallback : public b2QueryCallback
	{
	public:
		bool ReportFixture(b2Fixture* _fixture) override
		{
			if (_fixture->GetBody()->GetType() == b2_dynamicBody)
			{
				if (_fixture->TestPoint(point))
				{
					fixture = _fixture;
					return false;
				}
			}
			return true;
		}

		b2Vec2 point;
		b2Fixture* fixture = nullptr;
	};

	auto unprojected_pos = unproject(e.pos);
	auto world_pos = unprojected_pos / Scale;

	if (e.type == Shared::TouchEmulator::Event::Type::Begin)
	{
		if (!isDebug())
			return;

		if (!getScene()->interactTest(e.pos))
			return;

		auto p = b2Vec2(world_pos.x, world_pos.y);
		auto d = b2Vec2(0.001f, 0.001f);

		b2AABB aabb;
		aabb.lowerBound = p - d;
		aabb.upperBound = p + d;

		QueryCallback query;
		query.point = p;
		mB2World.QueryAABB(&query, aabb);

		if (query.fixture)
		{
			float frequencyHz = 5.0f;
			float dampingRatio = 0.7f;

			auto body = query.fixture->GetBody();
			b2MouseJointDef jd;
			jd.bodyA = mDummyBody;
			jd.bodyB = body;
			jd.target = p;
			jd.maxForce = 1000.0f * body->GetMass();
			b2LinearStiffness(jd.stiffness, jd.damping, frequencyHz, dampingRatio, jd.bodyA, jd.bodyB);

			mMouseJoint = (b2MouseJoint*)mB2World.CreateJoint(&jd);
			body->SetAwake(true);
		}
	}
	else if (e.type == Shared::TouchEmulator::Event::Type::End)
	{
		if (mMouseJoint)
		{
			mB2World.DestroyJoint(mMouseJoint);
			mMouseJoint = nullptr;
		}
	}
	else
	{
		if (mMouseJoint)
		{
			mMouseJoint->SetTarget({ world_pos.x, world_pos.y });
		}
	}
}

void World::update(sky::Duration delta)
{
	Scene::Node::update(delta);

	if (mShowStats)
	{
		STATS_INDICATE_GROUP("phys", "phys bodies", mB2World.GetBodyCount());
		STATS_INDICATE_GROUP("phys", "phys contacts", mB2World.GetContactCount());
		STATS_INDICATE_GROUP("phys", "phys joints", mB2World.GetJointCount());

		const auto& profile = mB2World.GetProfile();

		STATS_INDICATE_GROUP("phys", "phys step", fmt::format("{:.3f}", profile.step));
		STATS_INDICATE_GROUP("phys", "phys collide", fmt::format("{:.3f}", profile.collide));
		STATS_INDICATE_GROUP("phys", "phys solve", fmt::format("{:.3f}", profile.solve));
		STATS_INDICATE_GROUP("phys", "phys solve init", fmt::format("{:.3f}", profile.solveInit));
		STATS_INDICATE_GROUP("phys", "phys solve velocity", fmt::format("{:.3f}", profile.solveVelocity));
		STATS_INDICATE_GROUP("phys", "phys solve pos", fmt::format("{:.3f}", profile.solvePosition));
		STATS_INDICATE_GROUP("phys", "phys broadphase", fmt::format("{:.3f}", profile.broadphase));
		STATS_INDICATE_GROUP("phys", "phys solve toi", fmt::format("{:.3f}", profile.solveTOI));
	}

	for (auto body = mB2World.GetBodyList(); body; body = body->GetNext())
	{
		auto ptr = (void*)body->GetUserData().pointer;

		if (ptr == nullptr)
			continue;

		auto entity = static_cast<Entity*>(ptr);

		auto ent_pos = entity->getPosition() / Scale;
		auto ent_angle = entity->getRotation();

		auto body_pos = body->GetPosition();
		auto body_angle = body->GetAngle();

		if (ent_pos.x != body_pos.x || ent_pos.y != body_pos.y || ent_angle != body_angle)
			body->SetTransform({ ent_pos.x, ent_pos.y }, ent_angle);

		body->SetEnabled(entity->isEnabled());
		body->SetFixedRotation(entity->isFixedRotation());
		body->SetType(EntTypeToB2Type(entity->getType()));
		body->SetBullet(entity->isBullet());
	}

	mTimestepFixer.execute(delta, [&](auto delta) {
		mB2World.Step(sky::ToSeconds(delta), mVelocityIterations, mPositionIterations);
	});

	for (auto body = mB2World.GetBodyList(); body; body = body->GetNext())
	{
		auto ptr = (void*)body->GetUserData().pointer;

		if (ptr == nullptr)
			continue;

		auto entity = static_cast<Entity*>(ptr);

		auto pos = Scale * body->GetPosition();
		auto angle = body->GetAngle();

		entity->setPosition({ pos.x, pos.y });
		entity->setRotation(angle);
	}
}

void World::leaveDraw()
{
	Scene::Node::leaveDraw();

	if (!isDebug())
		return;

	auto model = glm::scale(getTransform(), { Scale, Scale, 1.0f });

	GRAPHICS->pushModelMatrix(model);
	mB2World.DebugDraw();
	GRAPHICS->pop();
}

void World::attach(std::shared_ptr<Node> node, AttachDirection attach_direction)
{
	Scene::Node::attach(node, attach_direction);

	auto entity = std::dynamic_pointer_cast<Entity>(node);

	if (!entity)
		return;

	auto size = entity->getSize() / Scale;
	auto pivot = entity->getPivot();
	auto center = (size * 0.5f) - (pivot * size);

	b2BodyDef body_def;
	body_def.userData.pointer = (uintptr_t)node.get();
	body_def.type = EntTypeToB2Type(entity->getType());

	b2PolygonShape box_shape;
	box_shape.SetAsBox(size.x / 2.0f, size.y / 2.0f, { center.x, center.y }, 0.0f);

	b2CircleShape circle_shape;
	circle_shape.m_radius = glm::min(size.x, size.y) * 0.5f;

	b2FixtureDef fixture_def;
	fixture_def.density = 10.0f;

	if (entity->getShape() == Entity::Shape::Box)
		fixture_def.shape = &box_shape;
	else
		fixture_def.shape = &circle_shape;

	auto body = mB2World.CreateBody(&body_def);
	auto fixture = body->CreateFixture(&fixture_def);

	entity->setB2Fixture(fixture);
}

void World::detach(std::shared_ptr<Node> node)
{
	Scene::Node::detach(node);

	auto entity = std::dynamic_pointer_cast<Entity>(node);

	if (!entity)
		return;

	mB2World.DestroyBody(entity->getB2Body());
}

// draw

void World::Draw::DrawPolygon(const b2Vec2* vertices, int32 vertexCount, const b2Color& color)
{
	GRAPHICS->draw(nullptr, nullptr, skygfx::utils::MeshBuilder::Mode::LineLoop, [&](auto vertex) {
		for (int i = 0; i < vertexCount; i++)
		{
			vertex({
				.pos = { vertices[i].x, vertices[i].y, 0.0f },
				.color = *(glm::vec4*)&color
			});
		}
	});
}

void World::Draw::DrawSolidPolygon(const b2Vec2* vertices, int32 vertexCount, const b2Color& color)
{
	GRAPHICS->draw(nullptr, nullptr, skygfx::utils::MeshBuilder::Mode::TriangleFan, [&](auto vertex) {
		for (int i = 0; i < vertexCount; i++)
		{
			vertex({
				.pos = { vertices[i].x, vertices[i].y, 0.0f },
				.color = *(glm::vec4*)&color * 0.5f
			});
		}
	});
	GRAPHICS->draw(nullptr, nullptr, skygfx::utils::MeshBuilder::Mode::LineLoop, [&](auto vertex) {
		for (int i = 0; i < vertexCount; i++)
		{
			vertex({
				.pos = { vertices[i].x, vertices[i].y, 0.0f },
				.color = *(glm::vec4*)&color
			});
		}
	});
}

void World::Draw::DrawCircle(const b2Vec2& center, float radius, const b2Color& color)
{
	float segments = 16.0f;
	float increment = 2.0f * glm::pi<float>() / segments;

	float sinInc = glm::sin(increment);
	float cosInc = glm::cos(increment);

	GRAPHICS->draw(nullptr, nullptr, skygfx::utils::MeshBuilder::Mode::Lines, [&](auto vertex) {
		auto v0 = glm::vec2({ center.x, center.y });
		auto r1 = glm::vec2({ 1.0f, 0.0f });
		auto v1 = v0 + radius * r1;

		for (int i = 0; i < segments; ++i)
		{
			auto r2 = glm::vec2({ cosInc * r1.x - sinInc * r1.y, sinInc * r1.x + cosInc * r1.y });
			auto v2 = v0 + radius * r2;
			vertex({ .pos = { v1.x, v1.y, 0.0f }, .color = *(glm::vec4*)&color });
			vertex({ .pos = { v2.x, v2.y, 0.0f }, .color = *(glm::vec4*)&color });
			r1 = r2;
			v1 = v2;
		}
	});
}

void World::Draw::DrawSolidCircle(const b2Vec2& center, float radius, const b2Vec2& axis, const b2Color& color)
{
	float segments = 16.0f;
	float increment = 2.0f * glm::pi<float>() / segments;

	float sinInc = glm::sin(increment);
	float cosInc = glm::cos(increment);

	auto v0 = glm::vec2({ center.x, center.y });
	auto r1 = glm::vec2({ cosInc, sinInc });
	auto v1 = v0 + radius * r1;

	GRAPHICS->draw(nullptr, nullptr, skygfx::utils::MeshBuilder::Mode::TriangleFan, [&](auto vertex) {
		auto _color = *(glm::vec4*)&color * 0.5f;
		vertex({ .pos = { v0.x, v0.y, 0.0f }, .color = _color });
		vertex({ .pos = { v1.x, v1.y, 0.0f }, .color = _color });
		for (int i = 0; i < segments; i++)
		{
			auto r2 = glm::vec2({ cosInc * r1.x - sinInc * r1.y, sinInc * r1.x + cosInc * r1.y });
			auto v2 = v0 + radius * r2;
			vertex({ .pos = { v2.x, v2.y, 0.0f }, .color = _color });
			r1 = r2;
		}
	});
	GRAPHICS->draw(nullptr, nullptr, skygfx::utils::MeshBuilder::Mode::Lines, [&](auto vertex) {
		auto _color = *(glm::vec4*)&color;
		r1 = { 1.0f, 0.0f };
		v1 = v0 + radius * r1;
		for (int i = 0; i < segments; ++i)
		{
			auto r2 = glm::vec2({ cosInc * r1.x - sinInc * r1.y, sinInc * r1.x + cosInc * r1.y });
			auto v2 = v0 + radius * r2;
			vertex({ .pos = { v1.x, v1.y, 0.0f }, .color = _color });
			vertex({ .pos = { v2.x, v2.y, 0.0f }, .color = _color });
			r1 = r2;
			v1 = v2;
		}
		vertex({ .pos = { v0.x, v0.y, 0.0f }, .color = _color });
		auto v3 = v0 + radius * glm::vec2({ axis.x, axis.y });
		vertex({ .pos = { v3.x, v3.y, 0.0f }, .color = _color });
	});
}

void World::Draw::DrawSegment(const b2Vec2& p1, const b2Vec2& p2, const b2Color& color)
{
	GRAPHICS->draw(nullptr, nullptr, skygfx::utils::MeshBuilder::Mode::Lines, [&](auto vertex) {
		vertex({ .pos = { p1.x, p1.y, 0.0f }, .color = *(glm::vec4*)&color });
		vertex({ .pos = { p2.x, p2.y, 0.0f }, .color = *(glm::vec4*)&color });
	});
}

void World::Draw::DrawTransform(const b2Transform& xf)
{
	const float AxisScale = 0.4f;
	b2Vec2 p = xf.p;
	b2Vec2 px = p + AxisScale * xf.q.GetXAxis();
	b2Vec2 py = p + AxisScale * xf.q.GetYAxis();

	glm::vec4 color1 = { Graphics::Color::Red, 1.0f };
	glm::vec4 color2 = { Graphics::Color::Lime, 1.0f };

	GRAPHICS->draw(nullptr, nullptr, skygfx::utils::MeshBuilder::Mode::Lines, [&](auto vertex) {
		vertex({ .pos = { p.x, p.y, 0.0f }, .color = color1 });
		vertex({ .pos = { px.x, px.y, 0.0f }, .color = color1 });
		vertex({ .pos = { p.x, p.y, 0.0f }, .color = color2 });
		vertex({ .pos = { py.x, py.y, 0.0f }, .color = color2 });
	});
}

void World::Draw::DrawPoint(const b2Vec2& p, float size, const b2Color& color)
{
	auto _color = glm::vec4(color.r, color.g, color.b, color.a);
	auto model = GRAPHICS->getCurrentState().model_matrix;

	auto unscaled_size = size / Scale;
	auto half_size = unscaled_size * 0.5f;

	model = glm::translate(model, { p.x, p.y, 0.0f });
	model = glm::translate(model, { -half_size, -half_size, 0.0f });
	model = glm::scale(model, { unscaled_size, unscaled_size, 1.0f });

	GRAPHICS->pushModelMatrix(model);
	GRAPHICS->drawCircle(_color, _color);
	GRAPHICS->pop();
}

// contact filter

bool World::ContactFilter::ShouldCollide(b2Fixture* fixtureA, b2Fixture* fixtureB)
{
	auto entA = static_cast<Entity*>((void*)fixtureA->GetBody()->GetUserData().pointer);
	auto entB = static_cast<Entity*>((void*)fixtureB->GetBody()->GetUserData().pointer);

	if (entA->getFilterLayer() != entB->getFilterLayer())
		return false;

	return b2ContactFilter::ShouldCollide(fixtureA, fixtureB);
}

// contact listener

void World::ContactListener::BeginContact(b2Contact* contact)
{
	auto entA = static_cast<Entity*>((void*)contact->GetFixtureA()->GetBody()->GetUserData().pointer);
	auto entB = static_cast<Entity*>((void*)contact->GetFixtureB()->GetBody()->GetUserData().pointer);

	auto callbackA = entA->getContactCallback();
	auto callbackB = entB->getContactCallback();

	if (callbackA)
		callbackA(*entB);

	if (callbackB)
		callbackB(*entA);
}
