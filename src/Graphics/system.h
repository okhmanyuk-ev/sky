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

namespace Graphics
{
	class System
	{
	public:
		struct State;

	public:
		void begin(const glm::mat4& viewMatrix = glm::mat4(1.0f), const glm::mat4& projectionMatrix = glm::mat4(1.0f));
		void beginOrtho();
		void beginOrtho(const Renderer::RenderTarget& target);
		void begin(const Camera& camera);
		void end();

	private:
		void applyState();
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
		void drawCircle(const glm::mat4& model, int segments = 32, const glm::vec4& color = { Color::White, 1.0f });

		// colored line circle
		void drawLineCircle(const glm::mat4& model, int segments = 32, const glm::vec4 & color = { Color::White, 1.0f });

		// sprite
		void draw(std::shared_ptr<Renderer::Texture> texture, const glm::mat4& model, 
			const TexRegion& tex_region = { }, const glm::vec4& color = { Color::White, 1.0f });

		// sdf mesh
		void drawSdf(Renderer::Topology topology, std::shared_ptr<Renderer::Texture> texture,
			const std::vector<Renderer::Vertex::PositionColorTexture>& vertices,
			const std::vector<uint32_t>& indices, float minValue, float maxValue, 
			float smoothFactor, const glm::mat4 model = glm::mat4(1.0f));

		// low-level text
		void drawString(const Font& font, utf8_string::const_iterator begin, utf8_string::const_iterator end, 
			const glm::mat4& model, const glm::vec4& color, float minValue, float maxValue, 
			float smoothFactor);
		
		void drawString(const Font& font, const utf8_string& text, const glm::mat4& model,
			const glm::vec4& color, float minValue, float maxValue, float smoothFactor);

		// text
		void drawString(const Font& font, utf8_string::const_iterator begin, utf8_string::const_iterator end,
			const glm::mat4& model, float size, const glm::vec4& color = { Color::White, 1.0f }, 
			float outlineThickness = 0.0f, const glm::vec4& outlineColor = { Color::Black, 1.0f });

		void drawString(const Font& font, const utf8_string& text, const glm::mat4& model,
			float size, const glm::vec4& color = { Color::White, 1.0f }, float outlineThickness = 0.0f,
			const glm::vec4& outlineColor = { Color::Black, 1.0f });

		// multiline string, returning height of text
		float drawMultilineString(const Font& font, const utf8_string& text, const glm::mat4& model,
			float size, float maxWidth, const glm::vec4& color = { Color::White, 1.0f }, float outlineThickness = 0.0f,
			const glm::vec4& outlineColor = { Color::Black, 1.0f });

	public:
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

		std::stack<State> mStates;

	public:
		bool isBatching() const { return mBatching; }
		void setBatching(bool value) { mBatching = value; }

	private:
		bool mBatching = true;
		
	private:
		Renderer::ShaderSDF mSdfShader = Renderer::ShaderSDF(Renderer::Vertex::PositionColorTexture::Layout);
		Renderer::ShaderDefault mTexturedShader = Renderer::ShaderDefault(Renderer::Vertex::PositionColorTexture::Layout);
		Renderer::ShaderDefault mColoredShader = Renderer::ShaderDefault(Renderer::Vertex::PositionColor::Layout);

	private:
		std::vector<Renderer::Vertex::PositionColorTexture> mBatchTexturedVertices;
		std::vector<Renderer::Vertex::PositionColor> mBatchColoredVertices;
		std::vector<uint32_t> mBatchIndices;
		std::optional<std::shared_ptr<Renderer::Texture>> mBatchTexture;
		std::optional<Renderer::Topology> mBatchTopology;
		
		size_t mBatchVerticesCount = 0;
		size_t mBatchIndicesCount = 0;

		bool mBatchFlushed = true;
	};
}
