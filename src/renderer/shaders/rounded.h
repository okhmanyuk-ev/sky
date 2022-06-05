#pragma once

#include <renderer/shaders/generic.h>

namespace Renderer::Shaders
{
	class Rounded : public Generic
	{
	private:
		struct alignas(16) Settings
		{
			glm::vec4 color = { 1.0f, 1.0f, 1.0f, 1.0f };
			glm::vec2 size;
			float radius;
		};

	public:
		Rounded(const Vertex::Layout& layout);

	protected:
		void update() override;

	public:
		auto getSize() const { return mSettings.size; }
		void setSize(const glm::vec2& value) { mSettings.size = value; }

		auto getRadius() const { return mSettings.radius; }
		void setRadius(float value) { mSettings.radius = value; }

		auto getColor() const { return mSettings.color; }
		void setColor(const glm::vec4& value) { mSettings.color = value; }

	private:
		Settings mSettings;
	};
}