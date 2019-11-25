#pragma once

#include <Scene/node.h>

namespace Scene
{
	namespace Debug
	{
		inline std::optional<std::shared_ptr<Graphics::Font>> Font = std::nullopt;
		inline float FontSize = 12.0f;
	}

	template <typename T> class Debuggable : public T
	{
		static_assert(std::is_base_of<Node, T>::value, "T must be derived from Node");

	protected:
		void endRender() override
		{
			T::endRender();
			
			auto model = glm::scale(T::getTransform(), { T::getSize(), 1.0f });

			if (mHighlightEnabled)
				GRAPHICS->drawRectangle(model, mHighlightColor);

			if (mOutlineEnabled)
				GRAPHICS->drawLineRectangle(model, mOutlineColor);

			if (Debug::Font.has_value() && mNameShowingEnabled)
			{
				auto model = glm::mat4(1.0f);
                model = glm::translate(model, { T::project(T::getSize() / 2.0f) / PLATFORM->getScale(), 0.0f });
		
				auto text_width = Debug::Font.value()->getStringWidth(mName, Debug::FontSize);
				auto text_height = Debug::Font.value()->getStringHeight(mName, Debug::FontSize);

				auto bg_width = text_width + 4.0f;
				auto bg_height = text_height + 4.0f;

				auto label_model = glm::translate(model, { -text_width * 0.5f, -text_height * 0.5f, 0.0f });
				auto bg_model = glm::translate(model, { -bg_width * 0.5f, -bg_height * 0.5f, 0.0f });

				auto scaleFactor = Debug::Font.value()->getScaleFactorForSize(Debug::FontSize);
				label_model = glm::scale(label_model, { scaleFactor, scaleFactor, 1.0f });
				
				bg_model = glm::scale(bg_model, { bg_width, bg_height, 1.0f });

				const glm::vec4 NameColor = { 1.0f, 1.0f, 1.0f, 1.0 };
				const glm::vec4 BackgroundColor = { 0.0f, 0.0f, 0.0f, 0.5f };

				GRAPHICS->drawRectangle(bg_model, BackgroundColor);
				GRAPHICS->draw(*Debug::Font.value(), mName, label_model, Debug::FontSize, NameColor);
			}
		}

	public:
		auto getName() const { return mName; }
		void setName(const std::string& value) { mName = value; }

		auto isNameShowingEnabled() const { return mNameShowingEnabled; }
		void setNameShowingEnabled(bool value) { mNameShowingEnabled = value; }

		auto isHighlightEnabled() const { return mHighlightEnabled; }
		void setHighlightEnabled(bool value) { mHighlightEnabled = value; }

		void setHighlightColor(const glm::vec3& value) { mHighlightColor = { value, mHighlightColor.a }; }
		void setHighlightColor(const glm::vec4& value) { mHighlightColor = value; }
		void setHighlightAlpha(float value) { mHighlightColor.a = value; }

		auto isOutlineEnabled() const { return mOutlineEnabled; }
		void setOutlineEnabled(bool value) { mOutlineEnabled = value; }

		void setOutlineColor(const glm::vec3& value) { mOutlineColor = { value, mOutlineColor.a }; }
		void setOutlineColor(const glm::vec4& value) { mOutlineColor = value; }
		void setOutlineAlpha(float value) { mOutlineColor.a = value; }

	private:
		std::string mName = typeid(this).name();
		bool mNameShowingEnabled = true;
		bool mHighlightEnabled = false;
		bool mOutlineEnabled = true;
		glm::vec4 mHighlightColor = { 1.0f, 1.0f, 1.0f, 1.0 };
		glm::vec4 mOutlineColor = { 1.0f, 1.0f, 1.0f, 1.0 };
	};
}