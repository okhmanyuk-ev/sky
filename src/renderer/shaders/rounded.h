#pragma once

#include <renderer/shader_custom.h>

namespace Renderer::Shaders
{
	class Rounded : public ShaderCustom
	{
	private:
		struct alignas(16) CustomConstantBuffer
		{
			glm::vec4 color = { 1.0f, 1.0f, 1.0f, 1.0f };
			glm::vec2 size;
			float radius;
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
		Rounded(const Vertex::Layout& layout, const std::set<Flag>& flags);
		Rounded(const Vertex::Layout& layout);
		
	public:
		auto getSize() const { return mCustomConstantBuffer.size; }
		void setSize(const glm::vec2& value) { mCustomConstantBuffer.size = value; markDirty(); }

		auto getRadius() const { return mCustomConstantBuffer.radius; }
		void setRadius(float value) { mCustomConstantBuffer.radius = value; markDirty(); }

		auto getColor() const { return mCustomConstantBuffer.color; }
		void setColor(const glm::vec4& value) { mCustomConstantBuffer.color = value; markDirty(); }

	private:
		CustomConstantBuffer mCustomConstantBuffer;
	};
}