#pragma once

// experimental stuff

#include <string>
#include <glm/glm.hpp>
#include <glm/ext.hpp>
#include <unordered_map>
#include <memory>
#include <stack>
#include <platform/system.h>

#define IMSCENE ENGINE->getSystem<ImScene::ImScene>()

namespace ImScene
{
	class Transform
	{
	public:
		auto getPosition() const { return mPosition; }
		auto& setPosition(const glm::vec2& value) { mPosition = value; return *this; }
		auto& setPosition(float value) { return setPosition({ value, value }); }

		auto getSize() const { return mSize; }
		auto& setSize(const glm::vec2& value) { mSize = value; return *this; }
		auto& setSize(float value) { return setSize({ value, value }); }

		auto getScale() const { return mScale; }
		auto& setScale(const glm::vec2& value) { mScale = value; return *this; }
		auto& setScale(float value) { return setScale({ value, value }); }

		auto getStretch() const { return mStretch; }
		auto& setStretch(const glm::vec2& value) { mStretch = value; return *this; }
		auto& setStretch(float value) { return setStretch({ value, value }); }

		auto getAnchor() const { return mAnchor; }
		auto& setAnchor(const glm::vec2& value) { mAnchor = value; return *this; }
		auto& setAnchor(float value) { return setAnchor({ value, value }); }

		auto getPivot() const { return mPivot; }
		auto& setPivot(const glm::vec2& value) { mPivot = value; return *this; }
		auto& setPivot(float value) { return setPivot({ value, value }); }

	private:
		glm::vec2 mPosition = { 0.0f, 0.0f };
		glm::vec2 mSize = { 0.0f, 0.0f };
		glm::vec2 mScale = { 1.0f, 1.0f };
		glm::vec2 mStretch = { -1.0f, -1.0f };
		glm::vec2 mAnchor = { 0.0f, 0.0f };
		glm::vec2 mPivot = { 0.0f, 0.0f };
	};

	struct Node
	{
		glm::vec2 size;
		glm::mat4 matrix;
	};

	class ImScene
	{
	public:
		void begin();
		glm::mat4 push(const std::string& name, const Transform& transform = Transform());
		void pop(int count = 1);
		void end();

	private:
		std::stack<Node> mNodeStack;
		bool mWorking = false;
	};
}