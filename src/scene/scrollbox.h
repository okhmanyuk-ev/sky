#pragma once

#include <scene/node.h>

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
		auto getContent() { return mContent; }
		auto getBounding() { return std::static_pointer_cast<Transform>(mBounding); }

		auto getSensitivity() const { return mSensitivity; }
		void setSensitivity(const glm::vec2& value) { mSensitivity = value; }

	private:
		std::shared_ptr<Node> mBounding = std::make_shared<Node>();
		std::shared_ptr<Node> mContent = std::make_shared<Node>();
		glm::vec2 mSensitivity = { 1.0f, 1.0f };
	};
}
