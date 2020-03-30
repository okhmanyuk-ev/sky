#pragma once

#include <common/frame_system.h>
#include <platform/system.h>
#include <renderer/system.h>
#include <renderer/topology.h>
#include <renderer/vertex.h>
#include <renderer/shaders/default.h>
#include <renderer/shaders/sdf.h>
#include <renderer/shaders/circle.h>

#include <graphics/color.h>
#include <graphics/font.h>
#include <graphics/camera.h>
#include <graphics/tex_region.h>

#include <glm/glm.hpp>
#include <glm/ext.hpp>
#include <optional>
#include <stack>
#include <tinyutf8.hpp>
#include "text_mesh.h"

#define GRAPHICS ENGINE->getSystem<Graphics::System>()

namespace Graphics
{
	class System
	{
	public:
		struct State;

	public:
		void begin();
		void end();

	private:
		void flush();

	public:
		void clear(const glm::vec4& color = { 0.0f, 0.0f, 0.0f, 0.0f });

		// draw colored vertices
		void draw(Renderer::Topology topology, const std::vector<Renderer::Vertex::PositionColor>& vertices,
			const glm::mat4& model = glm::mat4(1.0f));

		// draw indexed colored vertices
		void draw(Renderer::Topology topology, const std::vector<Renderer::Vertex::PositionColor>& vertices,
			const std::vector<uint32_t>& indices, const glm::mat4& model = glm::mat4(1.0f),
			std::shared_ptr<Renderer::ShaderMatrices> shader = nullptr);

		// draw colored and textured vertices
		void draw(Renderer::Topology topology, std::shared_ptr<Renderer::Texture> texture,
			const std::vector<Renderer::Vertex::PositionColorTexture>& vertices,
			const glm::mat4& model = glm::mat4(1.0f),
			std::shared_ptr<Renderer::ShaderMatrices> shader = nullptr);

		// draw indexed colored and textured vertices
		void draw(Renderer::Topology topology, std::shared_ptr<Renderer::Texture> texture, 
			const std::vector<Renderer::Vertex::PositionColorTexture>& vertices,
			const std::vector<uint32_t>& indices, const glm::mat4& model = glm::mat4(1.0f),
			std::shared_ptr<Renderer::ShaderMatrices> shader = nullptr);
		
		// colored rectangle
		void drawRectangle(const glm::mat4& model, const glm::vec4& color = { Color::White, 1.0f },
			std::shared_ptr<Renderer::ShaderMatrices> shader = nullptr);

		// colored line rectangle
		void drawLineRectangle(const glm::mat4& model, const glm::vec4& color = { Color::White, 1.0f });

		void drawCircle(const glm::mat4& model, const glm::vec4& inner_color = { Color::White, 1.0f },
			const glm::vec4& outer_color = { Color::White, 1.0f }, float fill = 1.0f, float begin = 0.0f, float end = 1.0f);

		// colored circle
		void drawSegmentedCircle(const glm::mat4& model, int segments = 32, const glm::vec4& inner_color = { Color::White, 1.0f },
			const glm::vec4& outer_color = { Color::White, 1.0f }, float fill = 1.0f);

		// sprite
		void draw(std::shared_ptr<Renderer::Texture> texture, const glm::mat4& model, 
			const TexRegion& tex_region = { }, const glm::vec4& color = { Color::White, 1.0f }, 
			std::shared_ptr<Renderer::ShaderMatrices> shader = nullptr);

		void draw(std::shared_ptr<Renderer::Texture> texture, const glm::mat4& model,
			std::shared_ptr<Renderer::ShaderMatrices> shader);

		// sdf mesh
		void drawSdf(Renderer::Topology topology, std::shared_ptr<Renderer::Texture> texture,
			const std::vector<Renderer::Vertex::PositionColorTexture>& vertices,
			const std::vector<uint32_t>& indices, float minValue, float maxValue, 
			float smoothFactor, const glm::mat4& model,
			const glm::vec4& color = { Graphics::Color::White, 1.0f });

		// text
		void drawString(const Font& font, const TextMesh& mesh, const glm::mat4& model, 
			float minValue, float maxValue, float smoothFactor, 
			const glm::vec4& color = { Graphics::Color::White, 1.0f });

		void drawString(const Font& font, const TextMesh& mesh, const glm::mat4& model, float size, 
			const glm::vec4& color = { Graphics::Color::White, 1.0f }, float outlineThickness = 0.0f,
			const glm::vec4& outlineColor = { Graphics::Color::Black, 1.0f });
		
		void drawString(const Font& font, const utf8_string& text, const glm::mat4& model, float size,
			const glm::vec4& color = { Graphics::Color::White, 1.0f }, float outlineThickness = 0.0f,
			const glm::vec4& outlineColor = { Graphics::Color::Black, 1.0f });

	private:
		glm::vec3 project(const glm::vec3& pos, const glm::mat4& model);
		void pushBatchIndices(const std::vector<uint32_t>& indices, size_t vertices_size);
		
	public:
		void applyState();
		void push(const State& value);
		void pop(int count = 1);

		void pushSampler(Renderer::Sampler value);
		void pushBlendMode(Renderer::BlendMode value);
		void pushDepthMode(Renderer::DepthMode value);
		void pushViewport(const Renderer::Viewport& value);
		void pushViewport(std::shared_ptr<Renderer::RenderTarget> target = nullptr);
		void pushRenderTarget(std::shared_ptr<Renderer::RenderTarget> value);
		void pushScissor(std::optional<Renderer::Scissor> value);
		void pushViewMatrix(const glm::mat4& value);
		void pushProjectionMatrix(const glm::mat4& value);
		void pushTextureAddress(Renderer::TextureAddress value);
		void pushOrthoMatrix(float width, float height);
		void pushOrthoMatrix(std::shared_ptr<Renderer::RenderTarget> target = nullptr);
		void pushStencilMode(const Renderer::StencilMode& value);

		auto getCurrentState() const { return mStates.top(); }
		
	public:
		struct State
		{
			glm::mat4 projectionMatrix = glm::mat4(1.0f);
			glm::mat4 viewMatrix = glm::mat4(1.0f);
			std::shared_ptr<Renderer::RenderTarget> renderTarget = nullptr;
			std::optional<Renderer::Scissor> scissor = std::nullopt;
			Renderer::Viewport viewport = Renderer::Viewport();
			Renderer::DepthMode depthMode = Renderer::DepthMode();
			Renderer::BlendMode blendMode = Renderer::BlendStates::NonPremultiplied;
			Renderer::Sampler sampler = Renderer::Sampler::Nearest;
			Renderer::TextureAddress textureAddress = Renderer::TextureAddress::Clamp;
			Renderer::StencilMode stencilMode = Renderer::StencilMode();
		};

	private:
		bool mWorking = false;

	private:
		std::stack<State> mStates;
		std::optional<State> mAppliedState;

	public:
		bool isBatching() const { return mBatching; }
		void setBatching(bool value);

	private:
		bool mBatching = true;
		
	private:
		std::shared_ptr<Renderer::Shaders::Sdf> mSdfShader = std::make_shared<Renderer::Shaders::Sdf>(Renderer::Vertex::PositionColorTexture::Layout);
		std::shared_ptr<Renderer::Shaders::Default> mTexturedShader = std::make_shared<Renderer::Shaders::Default>(Renderer::Vertex::PositionColorTexture::Layout);
		std::shared_ptr<Renderer::Shaders::Default> mColoredShader = std::make_shared<Renderer::Shaders::Default>(Renderer::Vertex::PositionColor::Layout);

		std::shared_ptr<Renderer::Shaders::Circle> mCircleShader = std::make_shared<Renderer::Shaders::Circle>(Renderer::Vertex::PositionColor::Layout);

		std::shared_ptr<Renderer::Shaders::Default> mBatchColorShader = std::make_shared<Renderer::Shaders::Default>(Renderer::Vertex::PositionColorTexture::Layout,
			std::set<Renderer::Shaders::Default::Flag>({ Renderer::Shaders::Default::Flag::Colored }));

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

			std::optional<std::shared_ptr<Renderer::Texture>> texture;
			std::optional<Renderer::Topology> topology;
			
			size_t verticesCount = 0;
			size_t indicesCount = 0;

			std::vector<uint32_t> indices;
			std::vector<Renderer::Vertex::PositionColorTexture> vertices;
		} mBatch;
	};

	inline bool operator==(const System::State& left, const System::State& right)
	{
		return
			left.projectionMatrix == right.projectionMatrix &&
			left.viewMatrix == right.viewMatrix &&
			left.renderTarget == right.renderTarget &&
			left.scissor == right.scissor &&
			left.depthMode == right.depthMode &&
			left.blendMode == right.blendMode &&
			left.sampler == right.sampler &&
			left.textureAddress == right.textureAddress &&
			left.stencilMode == right.stencilMode;
	}

	inline bool operator!=(const System::State& left, const System::State& right)
	{
		return !(left == right);
	}
}
