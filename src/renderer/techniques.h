#pragma once

#include <glm/glm.hpp>
#include <renderer/render_target.h>
#include <unordered_map>

#include <renderer/shaders/light.h> // TODO: del

namespace Renderer
{
	class Technique
	{
	public:
		struct IndexRange
		{
			uint32_t offset = 0;
			uint32_t count = 0;
		};

		using TexturesMap = std::unordered_map<std::shared_ptr<Renderer::Texture>, IndexRange>;

	public:
		virtual void draw(const skygfx::Buffer& vertex_buffer, const skygfx::Buffer& index_buffer, const skygfx::Vertex::Layout& layout,
			const TexturesMap& textures_map) = 0;

	public:
		const auto& getProjectionMatrix() const { return mProjectionMatrix; }
		void setProjectionMatrix(const glm::mat4& value) { mProjectionMatrix = value; }

		const auto& getViewMatrix() const { return mViewMatrix; }
		void setViewMatrix(const glm::mat4& value) { mViewMatrix = value; }

		const auto& getModelMatrix() const { return mModelMatrix; }
		void setModelMatrix(const glm::mat4& value) { mModelMatrix = value; }

		auto getMaterial() const { return mMaterial; }
		void setMaterial(const Renderer::Shaders::Light::Material& value) { mMaterial = value; }

	private:
		glm::mat4 mProjectionMatrix = glm::mat4(1.0f);
		glm::mat4 mViewMatrix = glm::mat4(1.0f);
		glm::mat4 mModelMatrix = glm::mat4(1.0f);
		Renderer::Shaders::Light::Material mMaterial; // TODO: this should not be here
	};

	class ForwardLightTechnique : public Technique
	{
	public:
		void draw(const skygfx::Buffer& vertex_buffer, const skygfx::Buffer& index_buffer, const skygfx::Vertex::Layout& layout,
			const TexturesMap& textures_map) override;

	public:
		void setEyePosition(const glm::vec3& value) { mEyePosition = value; }

		auto getDirectionalLight() const { return mDirectionalLight; }
		void setDirectionalLight(const Renderer::Shaders::Light::DirectionalLight& value) { mDirectionalLight = value; }

		auto getPointLight() const { return mPointLight; }
		void setPointLight(const Renderer::Shaders::Light::PointLight& value) { mPointLight = value; }

	private:
		glm::vec3 mEyePosition = { 0.0f, 0.0f, 0.0f };
		Renderer::Shaders::Light::DirectionalLight mDirectionalLight;
		Renderer::Shaders::Light::PointLight mPointLight;
	};
}