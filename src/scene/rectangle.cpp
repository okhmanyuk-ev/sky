#include "rectangle.h"

using namespace Scene;

void Rectangle::draw()
{
	auto model = glm::scale(getTransform(), { getSize(), 1.0f });
	auto color = getColor();

	std::vector<Renderer::Vertex::PositionColor> vertices = {
		{ { 0.0f, 0.0f, 0.0f }, { mCornerColors.at(Corner::TopLeft) * color } },
		{ { 0.0f, 1.0f, 0.0f }, { mCornerColors.at(Corner::BottomLeft) * color } },
		{ { 1.0f, 1.0f, 0.0f }, { mCornerColors.at(Corner::BottomRight) * color } },
		{ { 1.0f, 0.0f, 0.0f }, { mCornerColors.at(Corner::TopRight) * color } }
	};

	static const std::vector<uint32_t> indices = { 0, 1, 2, 0, 2, 3 };

	GRAPHICS->draw(Renderer::Topology::TriangleList, vertices, indices, model);

	Node::draw();
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