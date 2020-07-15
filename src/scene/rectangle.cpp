#include "rectangle.h"
#include <renderer/shaders/rounded.h>

using namespace Scene;

void Rectangle::draw()
{
	Node::draw();

	if (getAlpha() <= 0.0f)
		return;

	auto model = glm::scale(getTransform(), { getSize(), 1.0f });
	auto color = getColor();

	std::vector<Renderer::Vertex::PositionColor> vertices = {
		{ { 0.0f, 0.0f, 0.0f }, { mCornerColors.at(Corner::TopLeft) * color } },
		{ { 0.0f, 1.0f, 0.0f }, { mCornerColors.at(Corner::BottomLeft) * color } },
		{ { 1.0f, 1.0f, 0.0f }, { mCornerColors.at(Corner::BottomRight) * color } },
		{ { 1.0f, 0.0f, 0.0f }, { mCornerColors.at(Corner::TopRight) * color } }
	};

	static const std::vector<uint32_t> indices = { 0, 1, 2, 0, 2, 3 };
	
	if (mRounding > 0.0f)
	{
        static auto shader = std::make_shared<Renderer::Shaders::Rounded>(Renderer::Vertex::PositionColor::Layout);

		auto size = getSize();

		shader->setSize(size);
		shader->setRadius((mRounding * glm::min(size.x, size.y)) / 2.0f);

		GRAPHICS->draw(Renderer::Topology::TriangleList, vertices, indices, model, shader);
	}
	else
	{
		GRAPHICS->draw(Renderer::Topology::TriangleList, vertices, indices, model);
	}
}

void Rectangle::setHorizontalGradient(const glm::vec4& left, const glm::vec4& right)
{
	setCornerColor(Corner::TopLeft, left);
	setCornerColor(Corner::BottomLeft, left);
	setCornerColor(Corner::TopRight, right);
	setCornerColor(Corner::BottomRight, right);
}

void Rectangle::setHorizontalGradient(const glm::vec3& left, const glm::vec3& right)
{
	setHorizontalGradient({ left, 1.0f }, { right, 1.0f });
}

void Rectangle::setVerticalGradient(const glm::vec4& top, const glm::vec4& bottom)
{
	setCornerColor(Corner::TopLeft, top);
	setCornerColor(Corner::TopRight, top);
	setCornerColor(Corner::BottomLeft, bottom);
	setCornerColor(Corner::BottomRight, bottom);
}

void Rectangle::setVerticalGradient(const glm::vec3& top, const glm::vec3& bottom)
{
	setVerticalGradient({ top, 1.0f }, { bottom, 1.0f });
}
