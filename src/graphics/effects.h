#pragma once

#include <string>
#include <glm/glm.hpp>
#include <skygfx/skygfx.h>
#include <skygfx/utils.h>

namespace sky::effects
{
	class IEffect
	{
	public:
		virtual skygfx::Shader* getShader() const = 0;
		virtual void* getUniformData() const = 0;
		virtual size_t getUniformSize() const = 0;
	};

	template<class T>
	class Effect : public IEffect
	{
	public:
		Effect()
		{
			auto type_index = std::type_index(typeid(T));
			auto& context = skygfx::utils::GetContext();

			if (!context.shaders.contains(type_index))
				context.shaders.insert({ type_index, skygfx::utils::MakeEffectShader(T::Shader) });

			mShader = &context.shaders.at(type_index);
		}

		skygfx::Shader* getShader() const override { return mShader; }
		void* getUniformData() const override { return (void*)&uniform; }
		size_t getUniformSize() const override { return sizeof(T); }

		T uniform;

	private:
		skygfx::Shader* mShader;
	};

	struct alignas(16) Sdf
	{
		glm::vec4 color = { 1.0f, 1.0f, 1.0f, 1.0f };
		float min_value = 0.0f;
		float max_value = 0.0f;
		float smooth_factor = 0.0f;

		static const std::string Shader;
	};

	struct alignas(16) Circle
	{
		glm::vec4 color = { 1.0f, 1.0f, 1.0f, 1.0f };
		glm::vec4 inner_color = { 1.0f, 1.0f, 1.0f, 1.0f };
		glm::vec4 outer_color = { 1.0f, 1.0f, 1.0f, 1.0f };
		float fill = 1.0f;
		float pie = 1.0f;

		static const std::string Shader;
	};

	struct alignas(16) Rounded
	{
		glm::vec4 color = { 1.0f, 1.0f, 1.0f, 1.0f };
		glm::vec2 size;
		float radius;

		static const std::string Shader;
	};

	struct alignas(16) MipmapBias // TODO: remove this effect and use skygfx::utils::commands::SetMipmapBias()
	{
		float bias;

		static const std::string Shader;
	};

	struct alignas(16) Shockwave
	{
		float size = 1.0f;
		float thickness = 1.0f;
		float force = 1.0f;

		static const std::string Shader;
	};
}