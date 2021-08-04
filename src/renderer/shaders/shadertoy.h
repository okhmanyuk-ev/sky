#pragma once

#include <renderer/shader_custom.h>

namespace Renderer::Shaders
{
	class Shadertoy : public ShaderCustom
	{
	private:
		struct alignas(16) CustomConstantBuffer
		{
			glm::vec3 iResolution = { 0.0f, 0.0f, 0.0f };
			float iTime = 0.0f;
			glm::vec4 iMouse = { 0.0f, 0.0f, 0.0f, 0.0f };
		};

	public:
		Shadertoy(const Vertex::Layout& layout, const std::string& src);
		
	public:
		const auto& getCustomBuffer() const { return mCustomConstantBuffer; }
		void setCustomBuffer(const CustomConstantBuffer& value) { mCustomConstantBuffer = value; }

	private:
		CustomConstantBuffer mCustomConstantBuffer;
	};
}