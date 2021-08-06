#pragma once

#include <glm/glm.hpp>

namespace Scene3D
{
	class Transform
	{
	public:
		auto getScale() const { return mScale; }
		void setScale(const glm::vec3& value) { mScale = value; }

		auto getRotation() const { return mRotation; }
		void setRotation(const glm::vec3& value) { mRotation = value; }

	private:
		glm::vec3 mScale = { 1.0f, 1.0f, 1.0f };
		glm::vec3 mRotation = { 0.0f, 0.0f, 0.0f };

	public:
		void setScale(float value) { setScale({ value, value, value }); }
	};
}