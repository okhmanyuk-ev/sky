#pragma once

#include <Scene/node.h>

namespace Scene
{
	class Scrollbox : public Node
	{
	public:
		Scrollbox();

	protected:
		void update() override;
		void touch(Touch type, const glm::vec2& pos) override;

	private:
		glm::vec2 mPrevPosition = { 0.0f, 0.0f };
		
	public:
		void attachContent(std::shared_ptr<Node> value) { mContent->attach(value); }
		void detachContent(std::shared_ptr<Node> value) { mContent->detach(value); }
		void clearContent() { mContent->clear(); }

		void setContentSize(const glm::vec2& value) { mContent->setSize(value); }
		void setContentWidth(float value) { mContent->setWidth(value); }
		void setContentHeight(float value) { mContent->setHeight(value); }

		auto getContentPosition() const { return mContent->getPosition(); }
		void setContentPosition(const glm::vec2& value) { mContent->setPosition(value); }

		auto getContentX() const { return mContent->getX(); }
		void setContentX(float value) { mContent->setX(value); }

		auto getContentY() const { return mContent->getY(); }
		void setContentY(float value) { mContent->setY(value); }

		auto getBounding() { return std::static_pointer_cast<Transform>(mBounding); }

		auto getSensitivity() const { return mSensitivity; }
		void setSensitivity(const glm::vec2& value) { mSensitivity = value; }

	private:
		std::shared_ptr<Node> mBounding = std::make_shared<Node>();
		std::shared_ptr<Node> mContent = std::make_shared<Node>();
		glm::vec2 mSensitivity = { 1.0f, 1.0f };
	};
}
