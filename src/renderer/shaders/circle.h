#pragma once

#include <renderer/shaders/default.h>

namespace Renderer::Shaders
{
	class Circle : public Default
	{
	private:
		struct alignas(16) CustomConstantBuffer
		{
			glm::vec4 color = { 1.0f, 1.0f, 1.0f, 1.0f };
			glm::vec4 inner_color = { 1.0f, 1.0f, 1.0f, 1.0f };
			glm::vec4 outer_color = { 1.0f, 1.0f, 1.0f, 1.0f };
			float fill = 1.0f;
			float pie = 1.0f;
		};

	public:
		Circle(const Vertex::Layout& layout);
		
	public:
		auto getColor() const { return mCustomConstantBuffer.color; }
		void setColor(const glm::vec4& value) { mCustomConstantBuffer.color = value; }

		auto getFill() const { return mCustomConstantBuffer.fill; }
		void setFill(float value) { mCustomConstantBuffer.fill = value; }

		auto getPie() const { return mCustomConstantBuffer.pie; }
		void setPie(float value) { mCustomConstantBuffer.pie = value; }

		auto getInnerColor() const { return mCustomConstantBuffer.inner_color; }
		void setInnerColor(const glm::vec4& value) { mCustomConstantBuffer.inner_color = value; }

		auto getOuterColor() const { return mCustomConstantBuffer.outer_color; }
		void setOuterColor(const glm::vec4& value) { mCustomConstantBuffer.outer_color = value; }

	private:
		CustomConstantBuffer mCustomConstantBuffer;
	};
}