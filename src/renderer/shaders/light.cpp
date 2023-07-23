#include "light.h"
#include <renderer/system.h>

using namespace Renderer;
using namespace Renderer::Shaders;

static std::string src_fragment = R"(
struct DirectionalLight
{
	vec3 direction;
	vec3 ambient;
	vec3 diffuse;
	vec3 specular;
};

struct PointLight
{
	vec3 position;

	float constantAttenuation;
	float linearAttenuation;
	float quadraticAttenuation;

	vec3 ambient;
	vec3 diffuse;
	vec3 specular;
};

struct Material
{
	vec3 ambient;
	vec3 diffuse;
	vec3 specular;
	float shininess;
};

layout(binding = 2) uniform _settings
{
	vec3 eyePosition;
	DirectionalLight directionalLight;
	PointLight pointLight;
	Material material;
} settings;

vec3 CalcLight(vec3 normal, vec3 viewDirection, vec3 lightDirection, vec3 lightAmbient, vec3 lightDiffuse, vec3 lightSpecular)
{
	vec3 lightDir = normalize(lightDirection);

	float diff = max(dot(normal, lightDir), 0.0);

	vec3 reflectDir = reflect(-lightDir, normal);
	float spec = pow(max(dot(viewDirection, reflectDir), 0.0), settings.material.shininess);

	vec3 ambient = lightAmbient * settings.material.diffuse;
	vec3 diffuse = lightDiffuse * diff * settings.material.diffuse;
	vec3 specular = lightSpecular * spec * settings.material.specular;

	return ambient + diffuse + specular;
}

vec3 CalcDirectionalLight(DirectionalLight light, vec3 normal, vec3 viewDirection)
{
	return CalcLight(normal, viewDirection, -light.direction, light.ambient, light.diffuse, light.specular);
}

vec3 CalcPointLight(PointLight light, vec3 normal, vec3 viewDirection, vec3 pos)
{
	vec3 lightDir = light.position - pos;

	float distance = length(lightDir);
	float linearAttn = light.linearAttenuation * distance;
	float quadraticAttn = light.quadraticAttenuation * (distance * distance);
	float attenuation = 1.0 / (light.constantAttenuation + linearAttn + quadraticAttn);

	return CalcLight(normal, viewDirection, lightDir, light.ambient, light.diffuse, light.specular) * attenuation;
}

vec4 fragment(vec4 result)
{
	vec3 pos = vec3(ubo.model * vec4(In.Position, 1.0));

	vec3 normal = normalize(vec3(ubo.model * vec4(In.Normal, 1.0)));
	vec3 viewDirection = normalize(settings.eyePosition - pos);

	vec3 intensity = CalcDirectionalLight(settings.directionalLight, normal, viewDirection);
	intensity += CalcPointLight(settings.pointLight, normal, viewDirection, pos);
		
	result *= vec4(intensity, 1.0);

	return result;
})";

Light::Light(const skygfx::VertexLayout& layout, const std::set<Flag>& flags) : Generic(layout, flags, src_fragment)
{
}

Light::Light(const skygfx::VertexLayout& layout) : Generic(layout, src_fragment)
{
}

void Light::setupUniforms()
{
	Generic::setupUniforms();
	skygfx::SetUniformBuffer(2, mSettings);
}
