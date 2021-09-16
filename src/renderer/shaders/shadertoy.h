#pragma once

#include <renderer/shader_custom.h>

namespace Renderer::Shaders
{
	class Shadertoy : public ShaderCustom
	{
	public:
		enum class Flag
		{
			Textured,
			Colored
		};

	private:
		struct alignas(16) CustomConstantBuffer
		{
			glm::vec3 iResolution = { 1.0f, 1.0f, 0.0f };
			float iTime = 0.0f;
			glm::vec4 iMouse = { 0.0f, 0.0f, 0.0f, 0.0f };
			int iFrame = 0;
		};

	private:
		static std::set<Flag> MakeFlagsFromLayout(const Vertex::Layout& layout);
		static std::string MakeDefinesFromFlags(const Vertex::Layout& layout, const std::string& source, const std::set<Flag>& flags);

	public:
		Shadertoy(const Vertex::Layout& layout, const std::string& src, const std::set<Flag>& flags);
		Shadertoy(const Vertex::Layout& layout, const std::string& src);
		
	public:
		const auto& getCustomBuffer() const { return mCustomConstantBuffer; }
		void setCustomBuffer(const CustomConstantBuffer& value) { mCustomConstantBuffer = value; }

	private:
		CustomConstantBuffer mCustomConstantBuffer;
	};
}