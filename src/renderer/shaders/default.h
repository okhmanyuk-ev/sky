#pragma once

#include <renderer/shader_custom.h>

namespace Renderer::Shaders
{
	class Default : public ShaderCustom
	{
	private:
		struct alignas(16) CustomConstantBuffer
		{
			glm::vec4 color = { 1.0f, 1.0f, 1.0f, 1.0f };
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
		Default(const Vertex::Layout& layout, const std::set<Flag>& flags);
		Default(const Vertex::Layout& layout);
		
	public:
		auto getColor() const { return mCustomConstantBuffer.color; }
		void setColor(const glm::vec4& value) { mCustomConstantBuffer.color = value; markDirty(); }

	private:
		CustomConstantBuffer mCustomConstantBuffer;
	};
}