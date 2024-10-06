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
		virtual uint32_t getUniformBinding() const = 0;
		virtual void* getUniformData() const = 0;
		virtual size_t getUniformSize() const = 0;
	};

	template <typename T>
	concept HasEffectField = requires {
		{ T::Effect };
	};

	template<class T>
	class Effect : public IEffect
	{
	public:
		Effect()
		{
			auto type_index = std::type_index(typeid(T));
			auto& context = skygfx::utils::GetContext();

			auto concatDefines = [](std::vector<std::vector<std::string>> defines) -> std::vector<std::string> {
				auto result = std::vector<std::string>();
				for (const auto& strs : defines)
				{
					result.insert(result.end(), strs.begin(), strs.end());
				}
				return result;
			};

			if (!context.shaders.contains(type_index))
			{
				if constexpr (HasEffectField<T>)
				{
					auto vertex_shader = skygfx::utils::effects::BasicEffect::VertexShaderCode;
					auto fragment_shader = skygfx::utils::effects::BasicEffect::FragmentShaderCode + T::Effect;
					auto defines = concatDefines({ skygfx::utils::effects::BasicEffect::Defines, {
						"NORMAL_TEXTURE_BINDING 1",
						"EFFECT_UNIFORM_BINDING 3",
						"EFFECT_FUNC effect"
					} });
					auto shader = skygfx::Shader(vertex_shader, fragment_shader, defines);
					context.shaders.insert({ type_index, std::move(shader) });
				}
				else
				{
					auto shader = skygfx::Shader(T::VertexShaderCode, T::FragmentShaderCode, T::Defines);
					context.shaders.insert({ type_index, std::move(shader) });
				}
			}

			mShader = &context.shaders.at(type_index);
		}

		skygfx::Shader* getShader() const override { return mShader; }
		uint32_t getUniformBinding() const override { return 3; }
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

		static const std::string Effect;
	};

	struct alignas(16) Circle
	{
		glm::vec4 color = { 1.0f, 1.0f, 1.0f, 1.0f };
		glm::vec4 inner_color = { 1.0f, 1.0f, 1.0f, 1.0f };
		glm::vec4 outer_color = { 1.0f, 1.0f, 1.0f, 1.0f };
		float fill = 1.0f;
		float pie = 1.0f;

		static const std::string Effect;
	};

	struct alignas(16) Rounded
	{
		glm::vec4 color = { 1.0f, 1.0f, 1.0f, 1.0f };
		glm::vec2 size;
		float radius;

		static const std::string Effect;
	};

	struct alignas(16) Shockwave
	{
		float size = 1.0f;
		float thickness = 1.0f;
		float force = 1.0f;

		static const std::string Effect;
	};
}