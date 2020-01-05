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