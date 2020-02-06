#include "shader.h"

using namespace Renderer;

void Shader::checkRequiredAttribs(const std::set<Vertex::Attribute::Type>& requiredAttribs, const Vertex::Layout& layout)
{
	for (auto& attrib : requiredAttribs)
	{
		assert(layout.hasAttribute(attrib));
	}
}