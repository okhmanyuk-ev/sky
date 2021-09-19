#pragma once

#include <renderer/shader_custom.h>

namespace Renderer::Shaders
{
	class Shockwave : public ShaderCustom
	{
	private:
		struct alignas(16) CustomConstantBuffer
		{
			float progress = 0.5f;
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
		Shockwave(const Vertex::Layout& layout, const std::set<Flag>& flags);
		Shockwave(const Vertex::Layout& layout);

	public:
		auto getProgress() const { return mCustomConstantBuffer.progress; }
		void setProgress(float value) { mCustomConstantBuffer.progress = value; }

	private:
		CustomConstantBuffer mCustomConstantBuffer;
	};
}