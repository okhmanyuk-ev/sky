#pragma once

#include <Common/frame_system.h>
#include <Platform/system.h>
#include <Renderer/system.h>
#include <Renderer/topology.h>
#include <Renderer/vertex.h>
#include <Renderer/shader_default.h>
#include <Renderer/shader_sdf.h>

#include <Graphics/color.h>
#include <Graphics/font.h>
#include <Graphics/camera.h>
#include <Graphics/tex_region.h>

#include <glm/glm.hpp>
#include <glm/ext.hpp>
#include <optional>
#include <stack>
#include <tinyutf8.hpp>
#include "text_mesh.h"

namespace Graphics
{
	class System
	{
	public:
		struct State;

	public:
		void begin(const glm::mat4& viewMatrix = glm::mat4(1.0f), const glm::mat4& projectionMatrix = glm::mat4(1.0f));
		void beginOrtho();
		void begin(const Camera& camera);
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
			const std::vector<uint32_t>& indices, const glm::mat4& model = glm::mat4(1.0f));

		// draw colored and textured vertices
		void draw(Renderer::Topology topology, std::shared_ptr<Renderer::Texture> texture,
			const std::vector<Renderer::Vertex::PositionColorTexture>& vertices,
			const glm::mat4& model = glm::mat4(1.0f));

		// draw indexed colored and textured vertices
		void draw(Renderer::Topology topology, std::shared_ptr<Renderer::Texture> texture, 
			const std::vector<Renderer::Vertex::PositionColorTexture>& vertices,
			const std::vector<uint32_t>& indices, const glm::mat4& model = glm::mat4(1.0f));
		
		// colored rectangle
		void drawRectangle(const glm::mat4& model, const glm::vec4& color = { Color::White, 1.0f });

		// colored line rectangle
		void drawLineRectangle(const glm::mat4& model, const glm::vec4& color = { Color::White, 1.0f });

		// colored circle
		void drawCircle(const glm::mat4& model, int segments = 32, const glm::vec4& color = { Color::White, 1.0f },
			float fill = 1.0f, float begin = 0.0f, float end = 1.0f);

		// sprite
		void draw(std::shared_ptr<Renderer::Texture> texture, const glm::mat4& model, 
			const TexRegion& tex_region = { }, const glm::vec4& color = { Color::White, 1.0f });

		// sdf mesh
		void drawSdf(Renderer::Topology topology, std::shared_ptr<Renderer::Texture> texture,
			const std::vector<Renderer::Vertex::PositionTexture>& vertices,
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
		std::vector<uint32_t> triangulate(Renderer::Topology topology, const std::vector<uint32_t>& indices);

	public:
		void applyState();
		void push(const State& value);
		void pop(int count = 1);

		void push(Renderer::Sampler value);
		void push(Renderer::BlendMode value);
		void push(Renderer::DepthMode value);
		void push(const Renderer::Viewport& value);
		void push(std::shared_ptr<Renderer::RenderTarget> value);
		void push(std::optional<Renderer::Scissor> value);
		void pushViewMatrix(const glm::mat4& value);
		void pushProjectionMatrix(const glm::mat4& value);

		auto getCurrentState() const { return mStates.top(); }
		
	private:
		bool mWorking = false;

	public:
		struct State
		{
			glm::mat4 projectionMatrix = glm::mat4(1.0f);
			glm::mat4 viewMatrix = glm::mat4(1.0f);
			std::shared_ptr<Renderer::RenderTarget> renderTarget = nullptr;
			std::optional<Renderer::Scissor> scissor = std::nullopt;
			Renderer::Viewport viewport;
			Renderer::DepthMode depthMode = Renderer::DepthMode::None;
			Renderer::BlendMode blendMode = Renderer::BlendStates::NonPremultiplied;
			Renderer::Sampler sampler = Renderer::Sampler::Nearest;

			inline bool operator==(const State& value) const
			{
				return
					projectionMatrix == value.projectionMatrix &&
					viewMatrix == value.viewMatrix &&
					renderTarget == value.renderTarget &&
					scissor == value.scissor &&
					depthMode == value.depthMode &&
					blendMode == value.blendMode &&
					sampler == value.sampler;
			}

			inline bool operator!=(const State& value) const
			{
				return !(*this == value);
			}
		};

	private:
		std::stack<State> mStates;
		std::optional<State> mAppliedState;

	public:
		bool isBatching() const { return mBatching; }
		void setBatching(bool value);

	private:
		bool mBatching = true;
		
	private:
		Renderer::ShaderSDF mSdfShader = Renderer::ShaderSDF(Renderer::Vertex::PositionTexture::Layout);
		Renderer::ShaderDefault mTexturedShader = Renderer::ShaderDefault(Renderer::Vertex::PositionColorTexture::Layout);
		Renderer::ShaderDefault mColoredShader = Renderer::ShaderDefault(Renderer::Vertex::PositionColor::Layout);

	private:
		enum class BatchMode
		{
			None,
			Colored,
			Textured,
			Sdf
		};

		struct
		{
			BatchMode mode = BatchMode::None;

			std::optional<std::shared_ptr<Renderer::Texture>> texture;
			std::optional<Renderer::Topology> topology;
			
			size_t verticesCount = 0;
			size_t indicesCount = 0;

			std::vector<uint32_t> indices;

			std::vector<Renderer::Vertex::PositionTexture> positionTextureVertices;
			std::vector<Renderer::Vertex::PositionColorTexture> positionColorTextureVertices;
			std::vector<Renderer::Vertex::PositionColor> positionColorVertices;
		} mBatch;
	};
}
