#pragma once

#include <common/frame_system.h>
#include <platform/system.h>
#include <renderer/all.h>

#include <graphics/color.h>
#include <graphics/font.h>
#include <graphics/camera.h>
#include <graphics/tex_region.h>

#include <glm/glm.hpp>
#include <glm/ext.hpp>
#include <optional>
#include <stack>
#include <tinyutf8/tinyutf8.h>
#include "text_mesh.h"

#define GRAPHICS ENGINE->getSystem<Graphics::System>()

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

		void flush();

		void clear(std::optional<glm::vec4> color = glm::vec4{ 0.0f, 0.0f, 0.0f, 0.0f },
			std::optional<float> depth = 1.0f, std::optional<uint8_t> stencil = 0);

		void draw(skygfx::Topology topology, const Renderer::Buffer& vertices,
			const Renderer::Buffer& indices, std::shared_ptr<Renderer::ShaderMatrices> shader,
			std::function<void()> draw_func);

		// generic
		// TODO: hidden overload conflict when name "draw"
		// TODO: maybe remove after adding draw with draw_func
		void drawGeneric(skygfx::Topology topology, const Renderer::Buffer& vertices,
			const Renderer::Buffer& indices, std::shared_ptr<Renderer::ShaderMatrices> shader,
			std::optional<std::shared_ptr<skygfx::Texture>> texture = std::nullopt,
			std::optional<uint32_t> count = std::nullopt, uint32_t start = 0);

		// draw colored vertices
		void draw(skygfx::Topology topology, const std::vector<skygfx::Vertex::PositionColor>& vertices,
			std::optional<uint32_t> count = std::nullopt, uint32_t start = 0);

		// draw indexed colored vertices
		void draw(skygfx::Topology topology, const std::vector<skygfx::Vertex::PositionColor>& vertices,
			const std::vector<uint32_t>& indices, std::shared_ptr<Renderer::ShaderMatrices> shader = nullptr);

		// draw colored and textured vertices
		void draw(skygfx::Topology topology, std::shared_ptr<skygfx::Texture> texture,
			const std::vector<skygfx::Vertex::PositionColorTexture>& vertices,
			std::shared_ptr<Renderer::ShaderMatrices> shader = nullptr);

		// draw indexed colored and textured vertices
		void draw(skygfx::Topology topology, std::shared_ptr<skygfx::Texture> texture,
			const std::vector<skygfx::Vertex::PositionColorTexture>& vertices,
			const std::vector<uint32_t>& indices, std::shared_ptr<Renderer::ShaderMatrices> shader = nullptr);
		
		// colored rectangle
		void drawRectangle(const glm::vec4& top_left_color, const glm::vec4& top_right_color,
			const glm::vec4& bottom_left_color, const glm::vec4& bottom_right_color,
			std::shared_ptr<Renderer::ShaderMatrices> shader = nullptr);

		void drawRectangle(const glm::vec4& color = { Color::White, 1.0f },
			std::shared_ptr<Renderer::ShaderMatrices> shader = nullptr);

		void drawRectangle(std::shared_ptr<Renderer::ShaderMatrices> shader = nullptr);

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

		// sprite
		void drawSprite(std::shared_ptr<skygfx::Texture> texture, const glm::vec2& top_left_uv,
			const glm::vec2& top_right_uv, const glm::vec2& bottom_left_uv, const glm::vec2& bottom_right_uv,
			const glm::vec4& color = { Color::White, 1.0f }, std::shared_ptr<Renderer::ShaderMatrices> shader = nullptr);

		void drawSprite(std::shared_ptr<skygfx::Texture> texture,
			const TexRegion& tex_region = { }, const glm::vec4& color = { Color::White, 1.0f }, 
			std::shared_ptr<Renderer::ShaderMatrices> shader = nullptr);

		void drawSprite(std::shared_ptr<skygfx::Texture> texture, std::shared_ptr<Renderer::ShaderMatrices> shader);

		// sliced sprite
		void drawSlicedSprite(std::shared_ptr<skygfx::Texture> texture, const TexRegion& center_region,
			const glm::vec2& size, std::optional<float> edge_size = std::nullopt,
			const glm::vec4& color = { Color::White, 1.0f }, std::shared_ptr<Renderer::ShaderMatrices> shader = nullptr);

		// sdf mesh
		void drawSdf(skygfx::Topology topology, std::shared_ptr<skygfx::Texture> texture,
			const std::vector<skygfx::Vertex::PositionColorTexture>& vertices,
			const std::vector<uint32_t>& indices, float minValue, float maxValue, 
			float smoothFactor, const glm::vec4& color = { Graphics::Color::White, 1.0f });

		// text
		void drawString(const Font& font, const TextMesh& mesh, float minValue, float maxValue, 
			float smoothFactor, const glm::vec4& color = { Graphics::Color::White, 1.0f });

		void drawString(const Font& font, const TextMesh& mesh, float size, 
			const glm::vec4& color = { Graphics::Color::White, 1.0f }, float outlineThickness = 0.0f,
			const glm::vec4& outlineColor = { Graphics::Color::Black, 1.0f });
		
		void drawString(const Font& font, const utf8_string& text, float size,
			const glm::vec4& color = { Graphics::Color::White, 1.0f }, float outlineThickness = 0.0f,
			const glm::vec4& outlineColor = { Graphics::Color::Black, 1.0f });

	private:
		glm::vec3 project(const glm::vec3& pos);
		void pushBatchIndices(const std::vector<uint32_t>& indices, size_t vertices_size);
		
	public:
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
		void pushScissor(std::optional<skygfx::Scissor> value);
		void pushViewMatrix(const glm::mat4& value);
		void pushProjectionMatrix(const glm::mat4& value);
		void pushModelMatrix(const glm::mat4& value);
		void pushTextureAddress(skygfx::TextureAddress value);
		void pushOrthoMatrix(float width, float height);
		void pushOrthoMatrix(std::shared_ptr<skygfx::RenderTarget> target = nullptr);
		void pushStencilMode(std::optional<skygfx::StencilMode> value);

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

	private:
		bool mBatching = true;
		float mSdfSmoothFactor = 1.0f;
		int mBatchesCount = 0;
		int mBatchesCountPublic = 0;

	private:
		std::shared_ptr<Renderer::Shaders::Generic> mTexturedShader = std::make_shared<Renderer::Shaders::Generic>(skygfx::Vertex::PositionColorTexture::Layout);
		std::shared_ptr<Renderer::Shaders::Generic> mColoredShader = std::make_shared<Renderer::Shaders::Generic>(skygfx::Vertex::PositionColor::Layout);
		std::shared_ptr<Renderer::Shaders::Generic> mBatchColorShader = std::make_shared<Renderer::Shaders::Generic>(skygfx::Vertex::PositionColorTexture::Layout,
			std::set<Renderer::Shaders::Generic::Flag>({ Renderer::Shaders::Generic::Flag::Colored }));

	private:
		enum class BatchMode
		{
			None,
			Colored,
			Textured
		};

		struct
		{
			BatchMode mode = BatchMode::None;
			std::shared_ptr<Renderer::Shader> shader = nullptr;

			std::optional<std::shared_ptr<skygfx::Texture>> texture;
			std::optional<skygfx::Topology> topology;
			
			uint32_t verticesCount = 0;
			uint32_t indicesCount = 0;

			std::vector<uint32_t> indices;
			std::vector<skygfx::Vertex::PositionColorTexture> vertices;
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

	inline bool operator==(const System::State& left, const System::State& right)
	{
		return
			left.projection_matrix == right.projection_matrix &&
			left.view_matrix == right.view_matrix &&
		//	left.model_matrix == right.model_matrix && // we should not compare model matrix for fine batching
			left.render_target == right.render_target &&
			left.scissor == right.scissor &&
			left.viewport == right.viewport &&
			left.depth_mode == right.depth_mode &&
			left.cull_mode == right.cull_mode &&
			left.blend_mode == right.blend_mode &&
			left.sampler == right.sampler &&
			left.texture_address == right.texture_address &&
			left.stencil_mode == right.stencil_mode;
	}

	inline bool operator!=(const System::State& left, const System::State& right)
	{
		return !(left == right);
	}
}
