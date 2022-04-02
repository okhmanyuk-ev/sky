#include "phys_helpers.h"

using namespace Shared::PhysHelpers;

// entity

Entity::Entity()
{
	mNode = std::make_shared<Scene::Node>();
	mNode->setAnchor(0.5f);
	mNode->setPivot(0.5f);
}

void Entity::update()
{
	auto pos = getB2Body()->GetPosition();
	auto angle = getB2Body()->GetAngle();

	pos *= World::Scale;

	mNode->setPosition({ pos.x, pos.y });
	mNode->setRotation(angle);
	mNode->setSize(mSize);
	mNode->setPivot(mPivot);
}

// world

World::World()
{
	mNode = std::make_shared<Scene::Node>();
	mNode->setAnchor(0.5f);
	mNode->setPivot(0.5f);

	mTimestepFixer.setForceTimeCompletion(false);
	mTimestepFixer.setTimestep(Clock::FromSeconds(1.0f / 60.0f));

	mB2World.SetDebugDraw(&mPhysDraw);
	mPhysDraw.SetFlags(
		b2Draw::e_shapeBit
		| b2Draw::e_jointBit
	//	| b2Draw::e_aabbBit
		| b2Draw::e_pairBit
	//	| b2Draw::e_centerOfMassBit
	);
}

void World::update(Clock::Duration delta)
{
	mTimestepFixer.execute(delta, [this](auto delta) {
		mB2World.Step(Clock::ToSeconds(delta), 6, 2);
	});

	for (auto entity : mEntities)
	{
		entity->update();
	}
}

void World::draw()
{
	auto model = glm::mat4(1.0f);
	model = glm::scale(model, { Scale, Scale, 1.0f });

	auto pos = getNode()->getPosition();

	mCamera.setPosition(-pos);

	GRAPHICS->begin();
	GRAPHICS->pushViewMatrix(mCamera.getViewMatrix());
	GRAPHICS->pushProjectionMatrix(mCamera.getProjectionMatrix());
	GRAPHICS->pushModelMatrix(model);
	mB2World.DebugDraw();
	GRAPHICS->pop(3);
	GRAPHICS->end();
}

void World::addEntity(std::shared_ptr<Entity> entity)
{
	mEntities.insert(entity);
	mNode->attach(entity->getNode());

	auto type = entity->getType();
	auto pos = entity->getPosition() / Scale;
	auto size = entity->getSize() / Scale;
	auto pivot = entity->getPivot();
	auto fixed_rotation = entity->isFixedRotation();
	auto enabled = entity->isEnabled();
	auto center = (size * 0.5f) - (pivot * size);

	b2BodyDef bodyDef;

	if (type == Entity::Type::Static)
		bodyDef.type = b2_staticBody;
	else if (type == Entity::Type::Dynamic)
		bodyDef.type = b2_dynamicBody;
	else 
		bodyDef.type = b2_kinematicBody;

	bodyDef.fixedRotation = fixed_rotation;
	bodyDef.position = { pos.x, pos.y };
	bodyDef.enabled = enabled;

	auto body = mB2World.CreateBody(&bodyDef);

	b2PolygonShape shape;
	shape.SetAsBox(size.x / 2.0f, size.y / 2.0f, { center.x, center.y }, 0.0f);
	b2FixtureDef fixtureDef;
	fixtureDef.shape = &shape;
	fixtureDef.density = 10.0f;

	auto fixture = body->CreateFixture(&fixtureDef);
	
	entity->setB2Fixture(fixture);
}

void World::removeEntity(std::shared_ptr<Entity> entity)
{
	assert(mEntities.count(entity) > 0);
	mEntities.erase(entity);
	mNode->detach(entity->getNode());
	mB2World.DestroyBody(entity->getB2Body());
}

void PhysDraw::DrawPolygon(const b2Vec2* vertices, int32 vertexCount, const b2Color& color)
{
	static auto builder = Graphics::MeshBuilder();
	builder.begin();
	builder.color({ color.r, color.g, color.b });
	for (int i = 0; i < vertexCount; i++)
	{
		builder.vertex({ vertices[i].x, vertices[i].y, 0.0f });
	}
	builder.vertex({ vertices[0].x, vertices[0].y, 0.0f });
	auto [b_vertices, count] = builder.end();
	GRAPHICS->draw(Renderer::Topology::LineStrip, b_vertices, count);
}

void PhysDraw::DrawSolidPolygon(const b2Vec2* vertices, int32 vertexCount, const b2Color& color)
{
	static auto builder = Graphics::MeshBuilder();
	{
		builder.begin();
		builder.color({ color.r * 0.5f, color.g * 0.5f, color.b * 0.5f, 0.5f });
		for (int i = 1; i < vertexCount - 1; i++)
		{
			builder.vertex({ vertices[0].x, vertices[0].y, 0.0f });
			builder.vertex({ vertices[i].x, vertices[i].y, 0.0f });
			builder.vertex({ vertices[i + 1].x, vertices[i + 1].y, 0.0f });
		}
		auto [b_vertices, count] = builder.end();
		GRAPHICS->draw(Renderer::Topology::TriangleList, b_vertices, count);
	}
	{
		builder.begin();
		builder.color({ color.r, color.g, color.b });
		for (int i = 0; i < vertexCount; i++)
		{
			builder.vertex({ vertices[i].x, vertices[i].y, 0.0f });
		}
		builder.vertex({ vertices[0].x, vertices[0].y, 0.0f });
		auto [b_vertices, count] = builder.end();
		GRAPHICS->draw(Renderer::Topology::LineStrip, b_vertices, count);
	}
}

void PhysDraw::DrawCircle(const b2Vec2& center, float radius, const b2Color& color)
{
	const float segments = 16.0f;
	const float increment = 2.0f * glm::pi<float>() / segments;

	float sinInc = glm::sin(increment);
	float cosInc = glm::cos(increment);

	static auto builder = Graphics::MeshBuilder();
	builder.begin();
	builder.color({ color.r, color.g, color.b, color.a });

	auto v0 = glm::vec2({ center.x, center.y });
	auto r1 = glm::vec2({ 1.0f, 0.0f });
	auto v1 = v0 + radius * r1;

	for (int i = 0; i < segments; ++i)
	{
		auto r2 = glm::vec2({ cosInc * r1.x - sinInc * r1.y, sinInc * r1.x + cosInc * r1.y });
		auto v2 = v0 + radius * r2;
		builder.vertex(v1);
		builder.vertex(v2);
		r1 = r2;
		v1 = v2;
	}

	auto [b_vertices, count] = builder.end();
	GRAPHICS->draw(Renderer::Topology::LineList, b_vertices, count);
}

void PhysDraw::DrawSolidCircle(const b2Vec2& center, float radius, const b2Vec2& axis, const b2Color& color)
{
	const float segments = 16.0f;
	const float increment = 2.0f * glm::pi<float>() / segments;

	float sinInc = glm::sin(increment);
	float cosInc = glm::cos(increment);

	auto v0 = glm::vec2({ center.x, center.y });
	auto r1 = glm::vec2({ cosInc, sinInc });
	auto v1 = v0 + radius * r1;

	static auto builder = Graphics::MeshBuilder();
	
	{
		builder.begin();
		builder.color({ 0.5f * color.r, 0.5f * color.g, 0.5f * color.b, 0.5f });
		for (int i = 0; i < segments; i++)
		{
			auto r2 = glm::vec2({ cosInc * r1.x - sinInc * r1.y, sinInc * r1.x + cosInc * r1.y });
			auto v2 = v0 + radius * r2;
			builder.vertex(v0);
			builder.vertex(v1);
			builder.vertex(v2);
			r1 = r2;
			v1 = v2;
		}
		auto [b_vertices, count] = builder.end();
		GRAPHICS->draw(Renderer::Topology::TriangleList, b_vertices, count);
	}

	builder.begin();
	builder.color({ color.r, color.g, color.b, color.a });
	r1 = { 1.0f, 0.0f };
	v1 = v0 + radius * r1;
	for (int i = 0; i < segments; ++i)
	{
		auto r2 = glm::vec2({ cosInc * r1.x - sinInc * r1.y, sinInc * r1.x + cosInc * r1.y });
		auto v2 = v0 + radius * r2;
		builder.vertex(v1);
		builder.vertex(v2);
		r1 = r2;
		v1 = v2;
	}
	builder.vertex(v0);
	builder.vertex(v0 + radius * glm::vec2({ axis.x, axis.y }));
	auto [b_vertices, count] = builder.end();
	GRAPHICS->draw(Renderer::Topology::LineList, b_vertices, count);
}

void PhysDraw::DrawSegment(const b2Vec2& p1, const b2Vec2& p2, const b2Color& color)
{
	static auto builder = Graphics::MeshBuilder();
	builder.begin();
	builder.color({ color.r, color.g, color.b, color.a });
	builder.vertex({ p1.x, p1.y });
	builder.vertex({ p2.x, p2.y });
	auto [b_vertices, count] = builder.end();
	GRAPHICS->draw(Renderer::Topology::LineList, b_vertices, count);
}

void PhysDraw::DrawTransform(const b2Transform& xf)
{
	const float AxisScale = 0.4f;
	b2Vec2 p = xf.p;
	b2Vec2 px = p + AxisScale * xf.q.GetXAxis();
	b2Vec2 py = p + AxisScale * xf.q.GetYAxis();

	static auto builder = Graphics::MeshBuilder();
	builder.begin();
	builder.color(Graphics::Color::Red);
	builder.vertex({ p.x, p.y });
	builder.vertex({ px.x, px.y });
	builder.color(Graphics::Color::Lime);
	builder.vertex({ p.x, p.y });
	builder.vertex({ py.x, py.y });
	auto [b_vertices, count] = builder.end();
	GRAPHICS->draw(Renderer::Topology::LineList, b_vertices, count);
}

void PhysDraw::DrawPoint(const b2Vec2& p, float size, const b2Color& color)
{
	static auto builder = Graphics::MeshBuilder();
	builder.begin();
	builder.color({ color.r, color.g, color.b, color.a });
	builder.vertex({ p.x, p.y });
	builder.end();
	auto [b_vertices, count] = builder.end();
	GRAPHICS->draw(Renderer::Topology::PointList, b_vertices, count);
}
