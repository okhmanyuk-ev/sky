#pragma once

// experimental stuff

#include <string>
#include <glm/glm.hpp>
#include <glm/ext.hpp>
#include <unordered_map>
#include <memory>
#include <stack>
#include <platform/system.h>
#include <renderer/all.h>
#include <graphics/tex_region.h>
#include <graphics/color.h>
#include <graphics/font.h>

#define IMSCENE ENGINE->getSystem<ImScene::ImScene>()

namespace ImScene
{
	class Node
	{
	public:
		glm::vec2 parent_size = { 0.0f, 0.0f };
		glm::mat4 parent_matrix = glm::mat4(1.0f);

		glm::vec2 size = { 0.0f, 0.0f };
		glm::vec2 scale = { 1.0f, 1.0f };
		glm::vec2 margin = { 0.0f, 0.0f };
		glm::vec2 stretch = { -1.0f, -1.0f };
		glm::vec2 anchor = { 0.0f, 0.0f };
		glm::vec2 pivot = { 0.0f, 0.0f };
		glm::vec2 position = { 0.0f, 0.0f };
		glm::vec2 origin = { 0.0f, 0.0f };
		float rotation = 0.0f;

		glm::mat4 matrix = glm::mat4(1.0f);

		bool dirty = true;

		void ensureMatrix();
	};

	class ImScene
	{
	public:
		class Drawable;

	public:
		void begin();
		void begin(const glm::mat4& matrix, const glm::vec2& size);
		void end();
		
		void push();
		void pop(int count = 1);
		
		glm::vec2 size() const;
		void size(const glm::vec2& value);
		void size(float value);

		glm::vec2 position() const;
		void position(const glm::vec2& value);
		void position(float value);

		glm::vec2 stretch() const;
		void stretch(const glm::vec2& value);
		void stretch(float value);

		glm::vec2 anchor() const;
		void anchor(const glm::vec2& value);
		void anchor(float value);

		glm::vec2 pivot() const;
		void pivot(const glm::vec2& value);
		void pivot(float value);

		float rotation() const;
		void rotation(float value);

		void draw(Drawable& drawable);

	private:
		std::stack<Node> mNodeStack;
		bool mWorking = false;
		glm::mat4 mDefaultMatrix = glm::mat4(1.0f);
		glm::vec2 mDefaultSize = { 0.0f, 0.0f };
	};

	class ImScene::Drawable
	{
		friend ImScene;

	protected:
		virtual void draw(Node& node) = 0;
	};

	class Label : public ImScene::Drawable
	{
	public:
		Label& font(std::shared_ptr<Graphics::Font> value);
		Label& fontSize(float value);
		Label& text(const utf8_string& value);

	protected:
		void draw(Node& node) override;
	
	private:
		std::shared_ptr<Graphics::Font> mFont = nullptr;
		float mFontSize = 24.0f;
		utf8_string mText;
	};

	class Rectangle : public ImScene::Drawable
	{
	public:
		Rectangle& color(const glm::vec4& value);

	protected:
		void draw(Node& node) override;

	private:
		glm::vec4 mColor = { Graphics::Color::White, 1.0f };
	};

	class Sprite : public ImScene::Drawable
	{
	public:
		Sprite& texture(std::shared_ptr<Renderer::Texture> value);
		Sprite& region(const Graphics::TexRegion& value);
		
	protected:
		void draw(Node& node) override;

	private:
		std::shared_ptr<Renderer::Texture> mTexture = nullptr;
		Graphics::TexRegion mRegion = {};
	};

	class Circle : public ImScene::Drawable
	{
	public:
		Circle& color(const glm::vec4& value);
		Circle& fill(float value);
		Circle& begin(float value);
		Circle& end(float value);

	protected:
		void draw(Node& node) override;

	private:
		glm::vec4 mColor = { Graphics::Color::White, 1.0f };
		float mFill = 1.0f;
		float mBegin = 0.0f;
		float mEnd = 1.0f;
	};
}