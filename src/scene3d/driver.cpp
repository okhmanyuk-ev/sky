#include "driver.h"

using namespace Scene3D;

void Driver::prepareShader(Renderer::Shaders::Light& shader, Renderer::Shaders::Light::Material material)
{
	shader.setEyePosition(mCameraPosition);
	shader.setPointLight(mPointLight);
	shader.setMaterial(material);
}