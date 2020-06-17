#pragma once

#include <renderer/shader_custom.h>

namespace Renderer::Shaders
{
	class Circle : public ShaderCustom
	{
	private:
		struct alignas(16) CustomConstantBuffer
		{
			glm::vec4 color = { 1.0f, 1.0f, 1.0f, 1.0f };
			glm::vec4 inner_color = { 1.0f, 1.0f, 1.0f, 1.0f };
			glm::vec4 outer_color = { 1.0f, 1.0f, 1.0f, 1.0f };
			float fill = 1.0f;
			float begin = 0.0f;
			float end = 1.0f;
		};

	public:
		enum class Flag
		{
			Textured,
			Colored
		};

	private:
		static std::set<Flag> MakeFlagsFromLayout(const Vertex::Layout& layout);
		static std::string MakeDefinesFromFlags(const Vertex::Layout& layout, const std::string& source, const std::set<Flag>& flags);

	public:
		Circle(const Vertex::Layout& layout, const std::set<Flag>& flags);
		Circle(const Vertex::Layout& layout);
		
	public:
		auto getColor() const { return mCustomConstantBuffer.color; }
		void setColor(const glm::vec4& value) { mCustomConstantBuffer.color = value; markDirty(); }

		auto getFill() const { return mCustomConstantBuffer.fill; }
		void setFill(float value) { mCustomConstantBuffer.fill = value;  markDirty(); }

		auto getBegin() const { return mCustomConstantBuffer.begin; }
		void setBegin(float value) { mCustomConstantBuffer.begin = value;  markDirty(); }

		auto getEnd() const { return mCustomConstantBuffer.end; }
		void setEnd(float value) { mCustomConstantBuffer.end = value;  markDirty(); }

		auto getInnerColor() const { return mCustomConstantBuffer.inner_color; }
		void setInnerColor(const glm::vec4& value) { mCustomConstantBuffer.inner_color = value;  markDirty(); }

		auto getOuterColor() const { return mCustomConstantBuffer.outer_color; }
		void setOuterColor(const glm::vec4& value) { mCustomConstantBuffer.outer_color = value;  markDirty(); }

	private:
		CustomConstantBuffer mCustomConstantBuffer;
	};
}