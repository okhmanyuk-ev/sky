#include "model.h"

using namespace Scene3D;

void Model::draw()
{
	Node::draw();

	RENDERER->setTopology(mTopology);
	RENDERER->setIndexBuffer(mIndices);
	RENDERER->setVertexBuffer(mVertices);

	RENDERER->drawIndexed(mIndices.size());

}

// cube

Cube::Cube()
{
	const glm::vec4 VertexColor = { Graphics::Color::White, 1.0f };

	setVertices({
		/* front */
		/* 0  */ { { -1.0f,  1.0f,  1.0f }, VertexColor, { 0.0f, 0.0f, 1.0f } },
		/* 1  */ { {  1.0f,  1.0f,  1.0f }, VertexColor, { 0.0f, 0.0f, 1.0f } },
		/* 2  */ { { -1.0f, -1.0f,  1.0f }, VertexColor, { 0.0f, 0.0f, 1.0f } },
		/* 3  */ { {  1.0f, -1.0f,  1.0f }, VertexColor, { 0.0f, 0.0f, 1.0f } },

		/* top */
		/* 4  */ { { -1.0f,  1.0f,  1.0f }, VertexColor, { 0.0f, 1.0f, 0.0f } },
		/* 5  */ { { -1.0f,  1.0f, -1.0f }, VertexColor, { 0.0f, 1.0f, 0.0f } },
		/* 6  */ { {  1.0f,  1.0f,  1.0f }, VertexColor, { 0.0f, 1.0f, 0.0f } },
		/* 7  */ { {  1.0f,  1.0f, -1.0f }, VertexColor, { 0.0f, 1.0f, 0.0f } },

		/* left */
		/* 8  */ { { -1.0f,  1.0f, -1.0f }, VertexColor, { -1.0f, 0.0f, 0.0f } },
		/* 9  */ { { -1.0f,  1.0f,  1.0f }, VertexColor, { -1.0f, 0.0f, 0.0f } },
		/* 10 */ { { -1.0f, -1.0f, -1.0f }, VertexColor, { -1.0f, 0.0f, 0.0f } },
		/* 11 */ { { -1.0f, -1.0f,  1.0f }, VertexColor, { -1.0f, 0.0f, 0.0f } },

		/* back */
		/* 12 */ { { -1.0f,  1.0f, -1.0f }, VertexColor, { 0.0f, 0.0f, -1.0f } },
		/* 13 */ { { -1.0f, -1.0f, -1.0f }, VertexColor, { 0.0f, 0.0f, -1.0f } },
		/* 14 */ { {  1.0f,  1.0f, -1.0f }, VertexColor, { 0.0f, 0.0f, -1.0f } },
		/* 15 */ { {  1.0f, -1.0f, -1.0f }, VertexColor, { 0.0f, 0.0f, -1.0f } },

		/* bottom */
		/* 16 */ { { -1.0f, -1.0f,  1.0f }, VertexColor, { 0.0f, -1.0f, 0.0f } },
		/* 17 */ { {  1.0f, -1.0f,  1.0f }, VertexColor, { 0.0f, -1.0f, 0.0f } },
		/* 18 */ { { -1.0f, -1.0f, -1.0f }, VertexColor, { 0.0f, -1.0f, 0.0f } },
		/* 19 */ { {  1.0f, -1.0f, -1.0f }, VertexColor, { 0.0f, -1.0f, 0.0f } },

		/* right */
		/* 20 */ { { 1.0f, -1.0f, -1.0f }, VertexColor, { 1.0f, 0.0f, 0.0f } },
		/* 21 */ { { 1.0f, -1.0f,  1.0f }, VertexColor, { 1.0f, 0.0f, 0.0f } },
		/* 22 */ { { 1.0f,  1.0f, -1.0f }, VertexColor, { 1.0f, 0.0f, 0.0f } },
		/* 23 */ { { 1.0f,  1.0f,  1.0f }, VertexColor, { 1.0f, 0.0f, 0.0f } },
	});

	setIndices({
		0, 1, 2, 1, 3, 2, // front
		4, 5, 6, 5, 7, 6, // top
		8, 9, 10, 9, 11, 10, // left
		12, 13, 14, 13, 15, 14, // back
		16, 17, 18, 17, 19, 18, // bottom
		20, 21, 22, 21, 23, 22, // right
	});
}