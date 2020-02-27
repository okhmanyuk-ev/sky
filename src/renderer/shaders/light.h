#pragma once

#include <Renderer/shader_custom.h>

namespace Renderer
{
	class ShaderLight : public ShaderCustom
	{
	public:
		struct DirectionalLight
		{
			alignas(16) glm::vec3 direction = { 0.75f, 0.75f, 0.75f };
			alignas(16) glm::vec3 ambient = { 0.5f, 0.5f, 0.5f };
			alignas(16) glm::vec3 diffuse = { 1.0f, 1.0f, 1.0f };
			alignas(16) glm::vec3 specular = { 1.0f, 1.0f, 1.0f };
		};

		struct PointLight
		{
			alignas(16) glm::vec3 position = { 0.0f, 0.0f, 0.0f };

			float constantAttenuation = 1.0f;
			float linearAttenuation = 0.001f;
			float quadraticAttenuation = 0.001f;

			alignas(16) glm::vec3 ambient = { 1.0f, 1.0f, 1.0f };
			alignas(16) glm::vec3 diffuse = { 1.0f, 1.0f, 1.0f };
			alignas(16) glm::vec3 specular = { 1.0f, 1.0f, 1.0f };
		};

		struct Material
		{
			alignas(16) glm::vec3 ambient = { 0.5f, 0.5f, 0.5f };
			alignas(16) glm::vec3 diffuse = { 0.5f, 0.5f, 0.5f };
			alignas(16) glm::vec3 specular = { 0.5f, 0.5f, 0.5f };
			float shininess = 32.0f;
		};

	private:
		struct CustomConstantBuffer
		{
			alignas(16) glm::vec3 eyePosition = { 0.0f, 0.0f, 0.0 };

			DirectionalLight directionalLight;
			PointLight pointLight;
			Material material;
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
		ShaderLight(const Vertex::Layout& layout, const std::set<Flag>& flags);
		ShaderLight(const Vertex::Layout& layout);
		~ShaderLight();

	public:
		void setEyePosition(const glm::vec3& value) { mCustomConstantBuffer.eyePosition = value; markDirty(); }

		auto getDirectionalLight() const { return mCustomConstantBuffer.directionalLight; }
		void setDirectionalLight(const DirectionalLight& value) { mCustomConstantBuffer.directionalLight = value; markDirty(); }

		auto getPointLight() const { return mCustomConstantBuffer.pointLight; }
		void setPointLight(const PointLight& value) { mCustomConstantBuffer.pointLight = value; markDirty(); }

		auto getMaterial() const { return mCustomConstantBuffer.material; }
		void setMaterial(const Material& value) { mCustomConstantBuffer.material = value; markDirty(); }

	private:
		CustomConstantBuffer mCustomConstantBuffer;
	};

	namespace Materials
	{
		// http://devernay.free.fr/cours/opengl/materials.html

		const ShaderLight::Material Emerald = { { 0.0215f, 0.1745f, 0.0215f }, { 0.07568f, 0.61424f, 0.07568f }, { 0.633f, 0.727811f, 0.633f }, 0.6f * 128.0f };
		const ShaderLight::Material Jade = { { 0.135f, 0.2225f, 0.1575f }, { 0.54f, 0.89f, 0.63f },  { 0.316228f, 0.316228f, 0.316228f }, 0.1f * 128.0f };
		const ShaderLight::Material Obsidian = { { 0.05375f, 0.05f, 0.06625f }, { 0.18275f, 0.17f, 0.22525f }, { 0.332741f, 0.328634f, 0.346435f }, 0.3f * 128.0f };
		const ShaderLight::Material Pearl = { { 0.25f, 0.20725f, 0.20725f }, { 1.0f, 0.829f, 0.829f }, { 0.296648f, 0.296648f, 0.296648f }, 0.088f * 128.0f };
		const ShaderLight::Material Ruby = { { 0.1745f, 0.01175f, 0.01175f }, { 0.61424f, 0.04136f, 0.04136f }, { 0.727811f, 0.626959f, 0.626959f }, 0.6f * 128.0f };
		const ShaderLight::Material Turquoise = { { 0.1f, 0.18725f, 0.1745f }, { 0.396f, 0.74151f, 0.69102f }, { 0.297254f, 0.30829f, 0.306678f }, 0.1f * 128.0f };
		const ShaderLight::Material Brass = { { 0.329412f, 0.223529f, 0.027451f }, { 0.780392f, 0.568627f, 0.113725f }, { 0.992157f, 0.941176f, 0.807843f }, 0.21794872f * 128.0f };
		const ShaderLight::Material Bronze = { { 0.2125f, 0.1275f, 0.054f }, { 0.714f, 0.4284f, 0.18144f }, { 0.393548f, 0.271906f, 0.166721f }, 0.2f * 128.0f };
		const ShaderLight::Material Chrome = { { 0.25f, 0.25f, 0.25f }, { 0.4f, 0.4f, 0.4f }, { 0.774597f, 0.774597f, 0.774597f }, 0.6f * 128.0f };
		const ShaderLight::Material Copper = { { 0.19125f, 0.0735f, 0.0225f }, { 0.7038f, 0.27048f, 0.0828f }, { 0.256777f, 0.137622f, 0.086014f }, 0.1f * 128.0f };
		const ShaderLight::Material Gold = { { 0.24725f, 0.1995f, 0.0745f }, { 0.75164f, 0.60648f, 0.22648f }, { 0.628281f, 0.555802f, 0.366065f }, 0.4f * 128.0f };
		const ShaderLight::Material Silver = { { 0.19225f, 0.19225f, 0.19225f }, { 0.50754f, 0.50754f, 0.50754f }, { 0.508273f, 0.508273f, 0.508273f }, 0.4f * 128.0f };
		const ShaderLight::Material BlackPlastic = { { 0.0f, 0.0f, 0.0f }, { 0.01f, 0.01f, 0.01f }, { 0.50f, 0.50f, 0.50f }, 0.25f * 128.0f };
		const ShaderLight::Material CyanPlastic = { { 0.0f, 0.1f, 0.06f }, { 0.0f, 0.50980392f, 0.50980392f }, { 0.50196078f, 0.50196078f, 0.50196078f }, 0.25f * 128.0f };
		const ShaderLight::Material GreenPlastic = { { 0.0f, 0.0f, 0.0f }, { 0.1f, 0.35f, 0.1f }, { 0.45f, 0.55f, 0.45f }, 0.25f * 128.0f };
		const ShaderLight::Material RedPlastic = { { 0.0f, 0.0f, 0.0f }, { 0.5f, 0.0f, 0.0f }, { 0.7f, 0.6f, 0.6f }, 0.25f * 128.0f };
		const ShaderLight::Material WhitePlastic = { { 0.0f, 0.0f, 0.0f }, { 0.55f, 0.55f, 0.55f }, { 0.70f, 0.70f, 0.70f }, 0.25f * 128.0f };
		const ShaderLight::Material YellowPlastic = { { 0.0f, 0.0f, 0.0f }, { 0.5f, 0.5f, 0.0f }, { 0.60f, 0.60f, 0.50f }, 0.25f * 128.0f };
		const ShaderLight::Material BlackRubber = { { 0.02f, 0.02f, 0.02f }, { 0.01f, 0.01f, 0.01f }, { 0.4f, 0.4f, 0.4f }, 0.078125f * 128.0f };
		const ShaderLight::Material CyanRubber = { { 0.0f, 0.05f, 0.05f }, { 0.4f, 0.5f, 0.5f }, { 0.04f, 0.7f, 0.7f }, 0.078125f * 128.0f };
		const ShaderLight::Material GreenRubber = { { 0.0f, 0.05f, 0.0f }, { 0.4f, 0.5f, 0.4f }, { 0.04f, 0.7f, 0.04f }, 0.078125f * 128.0f };
		const ShaderLight::Material RedRubber = { { 0.05f, 0.0f, 0.0f }, { 0.5f, 0.4f, 0.4f }, { 0.7f, 0.04f, 0.04f }, 0.078125f * 128.0f };
		const ShaderLight::Material WhiteRubber = { { 0.05f, 0.05f, 0.05f }, { 0.5f, 0.5f, 0.5f }, { 0.7f, 0.7f, 0.7f }, 0.078125f * 128.0f };
		const ShaderLight::Material YellowRubber = { { 0.05f, 0.05f, 0.0f }, { 0.5f, 0.5f, 0.4f }, { 0.7f, 0.7f, 0.04f }, 0.078125f * 128.0f };
	}
}