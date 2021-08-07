#pragma once

#include <renderer/all.h>

namespace Scene3D
{
	class Driver // TODO: driver isnt a good name for it, maybe State/DrawState?
	{
	public:
		void prepareShader(Renderer::Shaders::Light& shader, Renderer::Shaders::Light::Material material);

		void setCameraPosition(const glm::vec3& value) { mCameraPosition = value; }

		auto getPointLight() const { return mPointLight; }
		void setPointLight(const Renderer::Shaders::Light::PointLight& value) { mPointLight = value; }

	private:
		glm::vec3 mCameraPosition;
		Renderer::Shaders::Light::PointLight mPointLight;
	};
}