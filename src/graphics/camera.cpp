#include "camera.h"

using namespace Graphics;

// camera 2d

glm::mat4 Camera2D::getViewMatrix() const
{
	auto eye = glm::vec3(0.0f, 0.0f, 0.0f);
	auto center = glm::vec3(0.0f, 0.0f, 1.0f);
	auto up = glm::vec3(0.0f, 1.0f, 0.0f);

	auto view = glm::lookAtLH(eye, center, up);

	view = glm::translate(view, glm::vec3({ PLATFORM->getLogicalWidth() / 2.0f, PLATFORM->getLogicalHeight() / 2.0f, 0.0f }));
	view = glm::translate(view, { -mPosition, 0.0f });

	return view;
}

glm::mat4 Camera2D::getProjectionMatrix() const
{
	return glm::orthoLH(0.0f, PLATFORM->getLogicalWidth(), PLATFORM->getLogicalHeight(), 0.0f, -1.0f, 1.0f);
}

// camera 3d

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