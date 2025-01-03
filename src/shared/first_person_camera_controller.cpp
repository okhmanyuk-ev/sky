#include "first_person_camera_controller.h"
#include <common/helpers.h>
#include <imgui.h>

using namespace Shared;

FirstPersonCameraController::FirstPersonCameraController(std::shared_ptr<skygfx::utils::PerspectiveCamera> camera) :
	mCamera(camera)
{
	mTimestepFixer.setSkipLongFrames(true);
}

void FirstPersonCameraController::onFrame()
{
	mTimestepFixer.execute([this](auto dtime) {
		update(dtime);
	});
}

void FirstPersonCameraController::update(Clock::Duration dTime)
{
	if (ImGui::IsAnyItemActive())
		return;

	if (mLookAround)
	{
		auto diff = mCurrentAngles - mPrevAngles;

		if (glm::length(diff) > 0.0f)
		{
			diff *= mSensivity / 10.0f;

			mCamera->yaw -= glm::radians(diff.x);
			mCamera->pitch += glm::radians(diff.y);

			mPrevAngles = mCurrentAngles;
		}
	}

	mMouseCaptured = false;

	{
		auto offset = mSensivity * Clock::ToSeconds(dTime) * 1.5f;

		if (mLeftArrow)
			mCamera->yaw += offset;

		if (mRightArrow)
			mCamera->yaw -= offset;

		if (mUpArrow)
			mCamera->pitch -= offset;

		if (mDownArrow)
			mCamera->pitch += offset;

		float limit = glm::pi<float>() / 2.0f - 0.01f;

		mCamera->pitch = glm::clamp(mCamera->pitch, -limit, +limit);
		mCamera->yaw = glm::wrapAngle(mCamera->yaw);
	}
	{
		auto speed = mSpeed * Clock::ToSeconds(dTime) * 50.0f;

		if (mKeyShift)
			speed *= 3.0f;

		if (mKeyCtrl)
			speed /= 3.0f;

		glm::vec2 direction = { 0.0f, 0.0f };

		if (mKeyW)
			direction.y = 1.0f;

		if (mKeyS)
			direction.y = -1.0f;

		if (mKeyA)
			direction.x = -1.0f;

		if (mKeyD)
			direction.x = 1.0f;

		if (glm::length(direction) > 0.0f)
		{
			direction = glm::normalize(direction);
			direction *= speed;
		}

		mSmoothDirection = sky::ease_towards(mSmoothDirection, direction, dTime, 0.075f);

		if (glm::length(mSmoothDirection) > 0.0f)
		{
			auto vectors = skygfx::utils::MakePerspectiveCameraVectors(*mCamera);
			mCamera->position += vectors.right * mSmoothDirection.x;
			mCamera->position += vectors.front * mSmoothDirection.y;
		}
	}
}

FirstPersonCameraController::~FirstPersonCameraController()
{
}

void FirstPersonCameraController::onEvent(const Platform::Input::Keyboard::Event& e)
{
	using namespace Platform::Input::Keyboard;

	bool value = e.type == Event::Type::Pressed ? true : false;

	if (e.key == Key::Left)
		mLeftArrow = value;
	else if (e.key == Key::Right)
		mRightArrow = value;
	else if (e.key == Key::Up)
		mUpArrow = value;
	else if (e.key == Key::Down)
		mDownArrow = value;

	if (e.key == Key::W)
		mKeyW = value;
	else if (e.key == Key::A)
		mKeyA = value;
	else if (e.key == Key::S)
		mKeyS = value;
	else if (e.key == Key::D)
		mKeyD = value;

	if (e.key == Key::LeftShift || e.key == Key::RightShift)
		mKeyShift = value;
	else if (e.key == Key::LeftCtrl || e.key == Key::RightCtrl)
		mKeyCtrl = value;
}

void FirstPersonCameraController::onEvent(const Platform::Input::Mouse::ButtonEvent& e)
{
	if (e.type == Platform::Input::Mouse::ButtonEvent::Type::Pressed && !ImGui::IsWindowHovered(ImGuiHoveredFlags_AnyWindow))
	{
		mLookAround = true;
		PLATFORM->hideCursor();
		mStartAngles = e.pos;
		mCurrentAngles = mStartAngles;
		mPrevAngles = mStartAngles;
	}

	if (e.type == Platform::Input::Mouse::ButtonEvent::Type::Released)
	{
		mLookAround = false;
		PLATFORM->showCursor();
	}
}

void FirstPersonCameraController::onEvent(const Platform::Input::Mouse::MoveEvent& e)
{
	if (!(mLookAround && !mMouseCaptured))
		return;

	mMouseCaptured = true;
	mCurrentAngles = e.pos;
	PLATFORM->setCursorPos(static_cast<int>(mStartAngles.x), static_cast<int>(mStartAngles.y));
	mPrevAngles = mStartAngles;
}

void FirstPersonCameraController::onEvent(const Platform::Input::Mouse::ScrollEvent& e)
{
	if (ImGui::IsWindowHovered(ImGuiHoveredFlags_AnyWindow))
		return;

	mCamera->fov -= e.scroll.y * 0.05f;
}

void FirstPersonCameraController::onEvent(const Platform::Input::Touch::Event& e)
{
	if (e.type == Platform::Input::Touch::Event::Type::Begin && !ImGui::IsWindowHovered(ImGuiHoveredFlags_AnyWindow))
	{
		mLookAround = true;
		mCurrentAngles = e.pos;
		mPrevAngles = mCurrentAngles;

		if (e.pos.y <= PLATFORM->getHeight() * 0.15f)
			mKeyW = true;
		else if (e.pos.y >= PLATFORM->getHeight() * 0.85f)
			mKeyS = true;
		else if (e.pos.x <= PLATFORM->getWidth() * 0.15f)
			mKeyA = true;
		else if (e.pos.x >= PLATFORM->getWidth() * 0.85f)
			mKeyD = true;
	}

	if (e.type == Platform::Input::Touch::Event::Type::End)
	{
		mLookAround = false;

		mKeyW = false;
		mKeyS = false;
		mKeyA = false;
		mKeyD = false;
	}

	if (e.type == Platform::Input::Touch::Event::Type::Continue)
	{
		mCurrentAngles = e.pos;
	}
}
