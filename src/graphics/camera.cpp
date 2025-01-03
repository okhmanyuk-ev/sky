#include "camera.h"

using namespace Graphics;

Camera3D::Camera3D()
{
}

void Camera3D::update()
{
	auto yaw = mYaw;// *(mWorldUp.y * -1.0f);
	auto pitch = mPitch;// *(mWorldUp.y * -1.0f);

	float sinYaw = glm::sin(yaw);
	float sinPitch = glm::sin(pitch);

	float cosYaw = glm::cos(yaw);
	float cosPitch = glm::cos(pitch);

	mFront = glm::normalize(glm::vec3(cosYaw * cosPitch, sinPitch, sinYaw * cosPitch));
	mRight = glm::normalize(glm::cross(mFront, mWorldUp));
	mUp = glm::normalize(glm::cross(mRight, mFront));

	auto width = static_cast<float>(PLATFORM->getWidth());
	auto height = static_cast<float>(PLATFORM->getHeight());

	mViewMatrix = glm::lookAtRH(mPosition, mPosition + mFront, mUp);
	mProjectionMatrix = glm::perspectiveFov(mFieldOfView, width, height, mNearPlane, mFarPlane);
}

void Camera3D::frontMove(float offset)
{
	mPosition += mFront * offset;
}

void Camera3D::sideMove(float offset)
{
	mPosition += mRight * offset;
}