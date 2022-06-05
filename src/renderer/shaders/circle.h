#pragma once

#include <renderer/shaders/generic.h>

namespace Renderer::Shaders
{
	class Circle : public Generic
	{
	private:
		struct alignas(16) Settings
		{
			glm::vec4 color = { 1.0f, 1.0f, 1.0f, 1.0f };
			glm::vec4 inner_color = { 1.0f, 1.0f, 1.0f, 1.0f };
			glm::vec4 outer_color = { 1.0f, 1.0f, 1.0f, 1.0f };
			float fill = 1.0f;
			float pie = 1.0f;
		};

	public:
		Circle(const Vertex::Layout& layout);

	protected:
		void update() override;

	public:
		auto getColor() const { return mSettings.color; }
		void setColor(const glm::vec4& value) { mSettings.color = value; }

		auto getFill() const { return mSettings.fill; }
		void setFill(float value) { mSettings.fill = value; }

		auto getPie() const { return mSettings.pie; }
		void setPie(float value) { mSettings.pie = value; }

		auto getInnerColor() const { return mSettings.inner_color; }
		void setInnerColor(const glm::vec4& value) { mSettings.inner_color = value; }

		auto getOuterColor() const { return mSettings.outer_color; }
		void setOuterColor(const glm::vec4& value) { mSettings.outer_color = value; }

	private:
		Settings mSettings;
	};
}