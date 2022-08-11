#pragma once

#include <renderer/shader.h>

namespace Renderer::Shaders
{
	// https://github.com/cansik/processing-bloom-filter

	class BrightFilter : public Shader, public ShaderMatrices
	{
	private:
		struct alignas(16) Settings
		{
			glm::mat4 projection = glm::mat4(1.0f);
			glm::mat4 view = glm::mat4(1.0f);
			glm::mat4 model = glm::mat4(1.0f);
			float threshold = 0.99f;
		};

	public:
		BrightFilter(const skygfx::Vertex::Layout& layout);

	protected:
		void update() override;

	public:		
		glm::mat4 getProjectionMatrix() const override { return mSettings.projection; }
		void setProjectionMatrix(const glm::mat4& value) override { mSettings.projection = value; }

		glm::mat4 getViewMatrix() const override { return mSettings.view; }
		void setViewMatrix(const glm::mat4& value) override { mSettings.view = value; }

		glm::mat4 getModelMatrix() const override { return mSettings.model; }
		void setModelMatrix(const glm::mat4& value) override { mSettings.model = value; }

		auto getThreshold() const { return mSettings.threshold; }
		void setThreshold(float value) { mSettings.threshold = value; }

	private:
		Settings mSettings;
	};
}