#pragma once

#include <common/frame_system.h>
#include <platform/system.h>
#include <renderer/all.h>
#include <sky/singleton.h>
#include <graphics/color.h>
#include <graphics/font.h>
#include <graphics/tex_region.h>
#include <graphics/effects.h>

#include <glm/glm.hpp>
#include <glm/ext.hpp>
#include <optional>
#include <stack>
#include "text_mesh.h"

#define GRAPHICS sky::Singleton<Graphics::System>::GetInstance()

namespace Graphics
{
	class System : public Common::FrameSystem::Frameable
	{
	public:
		struct State;

	public:
		System();

	private:
		void onFrame() override;

	public:
		void begin();
		void end();

		void flushBatch();

		void clear(std::optional<glm::vec4> color = glm::vec4{ 0.0f, 0.0f, 0.0f, 0.0f },
			std::optional<float> depth = 1.0f, std::optional<uint8_t> stencil = 0);

		void draw(sky::effects::IEffect* effect, skygfx::Texture* texture, skygfx::Topology topology,
			const skygfx::utils::Mesh& mesh);

		void draw(sky::effects::IEffect* effect, std::shared_ptr<skygfx::Texture> texture,
			skygfx::Topology topology, skygfx::utils::Mesh::Vertex* vertices, uint32_t vertex_count,
			skygfx::utils::Mesh::Index* indices, uint32_t index_count);

		void draw(sky::effects::IEffect* effect, std::shared_ptr<skygfx::Texture> texture,
			skygfx::Topology topology, const skygfx::utils::Mesh::Vertices& vertices,
			const skygfx::utils::Mesh::Indices& indices);

		void draw(sky::effects::IEffect* effect, std::shared_ptr<skygfx::Texture> texture,
			skygfx::utils::MeshBuilder::Mode mode,
			std::function<void(std::function<void(const skygfx::utils::Mesh::Vertex&)>)> callback);

		// colored rectangle
		void drawTexturedRectangle(sky::effects::IEffect* effect, std::shared_ptr<skygfx::Texture> texture,
			const glm::vec2& top_left_uv, const glm::vec2& top_right_uv,
			const glm::vec2& bottom_left_uv, const glm::vec2& bottom_right_uv,
			const glm::vec4& top_left_color, const glm::vec4& top_right_color,
			const glm::vec4& bottom_left_color, const glm::vec4& bottom_right_color);

		void drawTexturedRectangle(sky::effects::IEffect* effect, std::shared_ptr<skygfx::Texture> texture,
			const TexRegion& region, const glm::vec4& top_left_color, const glm::vec4& top_right_color,
			const glm::vec4& bottom_left_color, const glm::vec4& bottom_right_color);

		void drawRectangle(sky::effects::IEffect* effect,
			const glm::vec4& top_left_color, const glm::vec4& top_right_color,
			const glm::vec4& bottom_left_color, const glm::vec4& bottom_right_color);

		void drawRectangle(sky::effects::IEffect* effect, const glm::vec4& color = { Color::White, 1.0f });

		// rounded rectangle
		void drawRoundedRectangle(const glm::vec4& top_left_color, const glm::vec4& top_right_color,
			const glm::vec4& bottom_left_color, const glm::vec4& bottom_right_color, const glm::vec2& size, float rounding, bool absolute_rounding);

		void drawRoundedRectangle(const glm::vec4& color, const glm::vec2& size, float rounding, bool absolute_rounding);

		void drawRoundedSlicedRectangle(const glm::vec4& color, const glm::vec2& size, float rounding, bool absolute_rounding);

		// colored line rectangle
		void drawLineRectangle(const glm::vec4& color = { Color::White, 1.0f });

		// circle
		void drawCircle(const glm::vec4& inner_color = { Color::White, 1.0f },
			const glm::vec4& outer_color = { Color::White, 1.0f }, float fill = 1.0f, float pie = 1.0f);

		void drawSegmentedCircle(int segments = 32, const glm::vec4& inner_color = { Color::White, 1.0f },
			const glm::vec4& outer_color = { Color::White, 1.0f }, float fill = 1.0f);

		void drawCircleTexture(const glm::vec4& color = { Color::White, 1.0f });

		// sliced sprite
		void drawSlicedSprite(sky::effects::IEffect* effect, std::shared_ptr<skygfx::Texture> texture, const TexRegion& center_region,
			const glm::vec2& size, std::optional<float> edge_size = std::nullopt,
			const glm::vec4& color = { Color::White, 1.0f });

		// sdf mesh
		void drawSdf(skygfx::Topology topology, std::shared_ptr<skygfx::Texture> texture,
			const skygfx::utils::Mesh::Vertices& vertices, const skygfx::utils::Mesh::Indices& indices,
			float minValue, float maxValue, float smoothFactor,
			const glm::vec4& color = { Graphics::Color::White, 1.0f });

		// text
		void drawString(const Font& font, const TextMesh& mesh, float minValue, float maxValue,
			float smoothFactor, const glm::vec4& color = { Graphics::Color::White, 1.0f });

		void drawString(const Font& font, const TextMesh& mesh, float size,
			const glm::vec4& color = { Graphics::Color::White, 1.0f }, float outlineThickness = 0.0f,
			const glm::vec4& outlineColor = { Graphics::Color::Black, 1.0f }, float smoothFactorScale = 1.0f);

		void drawString(const Font& font, const std::wstring& text, float size,
			const glm::vec4& color = { Graphics::Color::White, 1.0f }, float outlineThickness = 0.0f,
			const glm::vec4& outlineColor = { Graphics::Color::Black, 1.0f }, float smoothFactorScale = 1.0f);

	public:
		glm::vec3 project(const glm::vec3& pos);
		bool isSameBatch(const State& left, const State& right);
		void applyState();
		void push(const State& value);
		void pop(int count = 1);

		void pushCleanState();
		void pushSampler(skygfx::Sampler value);
		void pushBlendMode(skygfx::BlendMode value);
		void pushDepthMode(std::optional<skygfx::DepthMode> value);
		void pushCullMode(skygfx::CullMode value);
		void pushViewport(std::optional<skygfx::Viewport> value);
		void pushRenderTarget(std::shared_ptr<skygfx::RenderTarget> value);
		void pushScissor(std::optional<skygfx::Scissor> value, bool inherit_prev_scissor = false);
		void pushViewMatrix(const glm::mat4& value);
		void pushProjectionMatrix(const glm::mat4& value);
		void pushModelMatrix(const glm::mat4& value);
		void pushTextureAddress(skygfx::TextureAddress value);
		void pushOrthoMatrix(float width, float height);
		void pushOrthoMatrix(std::shared_ptr<skygfx::RenderTarget> target = nullptr);
		void pushStencilMode(std::optional<skygfx::StencilMode> value);
		void pushMipmapBias(float bias);

		const auto& getCurrentState() const { return mStates.top(); }

	public:
		struct State
		{
			glm::mat4 projection_matrix = glm::mat4(1.0f);
			glm::mat4 view_matrix = glm::mat4(1.0f);
			glm::mat4 model_matrix = glm::mat4(1.0f);
			std::shared_ptr<skygfx::RenderTarget> render_target = nullptr;
			std::optional<skygfx::Scissor> scissor = std::nullopt;
			std::optional<skygfx::Viewport> viewport = std::nullopt;
			std::optional<skygfx::DepthMode> depth_mode = std::nullopt;
			skygfx::CullMode cull_mode = skygfx::CullMode::None;
			skygfx::BlendMode blend_mode = skygfx::BlendStates::NonPremultiplied;
			skygfx::Sampler sampler = skygfx::Sampler::Nearest;
			skygfx::TextureAddress texture_address = skygfx::TextureAddress::Clamp;
			std::optional<skygfx::StencilMode> stencil_mode = std::nullopt;
			float mipmap_bias = 0.0f;
		};

	private:
		bool mWorking = false;

	private:
		std::stack<State> mStates;
		std::optional<State> mAppliedState;

	public:
		bool isBatching() const { return mBatching; }
		void setBatching(bool value);

		auto getSdfSmoothFactor() const { return mSdfSmoothFactor; }
		void setSdfSmoothFactor(float value) { mSdfSmoothFactor = value; }

		auto getBatchesCount() const { return mBatchesCountPublic; }
		auto getBatchFlushCount() const { return mFlushCountPublic; }

	private:
		bool mBatching = true;
		float mSdfSmoothFactor = 1.0f;
		int mBatchesCount = 0;
		int mBatchesCountPublic = 0;
		int mFlushCount = 0;
		int mFlushCountPublic = 0;

	private:
		struct
		{
			std::shared_ptr<skygfx::Texture> texture;
			std::optional<skygfx::Topology> topology;

			skygfx::utils::Mesh::Vertices vertices;
			skygfx::utils::Mesh::Indices indices;

			skygfx::utils::Mesh mesh;
		} mBatch;

	public:
		std::shared_ptr<skygfx::RenderTarget> getRenderTarget(const std::string& name, uint32_t width, uint32_t height);
		std::shared_ptr<skygfx::RenderTarget> getRenderTarget(const std::string& name);

		const auto& getRenderTargets() const { return mRenderTargets; }

	private:
		std::map<std::string, std::shared_ptr<skygfx::RenderTarget>> mRenderTargets;
		std::set<std::string> mUnusedRenderTargets;

	public:
		std::shared_ptr<skygfx::Texture> makeGenericTexture(const glm::ivec2& size, std::function<void()> callback);

	private:
		std::shared_ptr<skygfx::Texture> mWhiteCircleTexture = nullptr;
	};
}
