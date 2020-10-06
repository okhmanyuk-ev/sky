#include "first_person_camera_controller.h"

#include <imgui.h>

using namespace Shared;

FirstPersonCameraController::FirstPersonCameraController(Graphics::Camera3D& camera) :
	mCamera(camera)
{
	//
}

void FirstPersonCameraController::frame()
{
	if (ImGui::IsAnyItemActive())
		return;

	if (mLookAround)
	{
		auto diff = mCurrentAngles - mPrevAngles;

		if (glm::length(diff) > 0.0f)
		{
			diff *= mSensivity / 10.0f;

			mCamera.setYaw(mCamera.getYaw() - glm::radians(diff.x));
			mCamera.setPitch(mCamera.getPitch() + glm::radians(diff.y));

			mPrevAngles = mCurrentAngles;
		}
	}

	mMouseCaptured = false;

	{
		auto offset = mSensivity * Clock::ToSeconds(FRAME->getTimeDelta()) * 1.5f;

		if (mLeftArrow)
			mCamera.setYaw(mCamera.getYaw() + offset);

		if (mRightArrow)
			mCamera.setYaw(mCamera.getYaw() - offset);

		if (mUpArrow)
			mCamera.setPitch(mCamera.getPitch() - offset);

		if (mDownArrow)
			mCamera.setPitch(mCamera.getPitch() + offset);

		float limit = glm::pi<float>() / 2.0f - 0.01f;

		mCamera.setPitch(fmaxf(-limit, mCamera.getPitch()));
		mCamera.setPitch(fminf(+limit, mCamera.getPitch()));

		auto pi = glm::pi<float>();

		while (mCamera.getYaw() > pi)
			mCamera.setYaw(mCamera.getYaw() - (pi * 2.0f));

		while (mCamera.getYaw() < -pi)
			mCamera.setYaw(mCamera.getYaw() + (pi * 2.0f));
	}
	{
		auto speed = mSpeed * Clock::ToSeconds(FRAME->getTimeDelta()) * 50.0f;

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

		if (mKeyW || mKeyS || mKeyA || mKeyD)
		{
			direction = glm::normalize(direction);

			mCamera.sideMove(direction.x * speed);
			mCamera.frontMove(direction.y * speed);
		}

		auto pos = mCamera.getPosition();

		mCamera.setPosition(pos);
	}
}

FirstPersonCameraController::~FirstPersonCameraController()
{
	//
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

	if (e.key == Key::Shift)
		mKeyShift = value;
	else if (e.key == Key::Ctrl)
		mKeyCtrl = value;
}

void FirstPersonCameraController::onEvent(const Platform::Input::Mouse::Event& e)
{
	using namespace Platform::Input::Mouse;

	if (e.button == Button::Left)
	{
		if (e.type == Event::Type::ButtonDown && !ImGui::IsWindowHovered(ImGuiHoveredFlags_AnyWindow))
		{
			mLookAround = true;
			PLATFORM->hideCursor();
			mStartAngles = { e.x, e.y };
			mCurrentAngles = mStartAngles;
			mPrevAngles = mStartAngles;
		}

		if (e.type == Event::Type::ButtonUp)
		{
			mLookAround = false;
			PLATFORM->showCursor();
		}
	}

	if (e.type == Event::Type::Move && mLookAround && !mMouseCaptured)
	{
		mMouseCaptured = true;
		mCurrentAngles = { e.x, e.y };
		PLATFORM->setCursorPos(static_cast<int>(mStartAngles.x), static_cast<int>(mStartAngles.y));
		mPrevAngles = mStartAngles;
	}

	if (e.type == Event::Type::Wheel)
	{
		mCamera.setFieldOfView(mCamera.getFieldOfView() - e.wheelY * 0.05f);
	}
}

void FirstPersonCameraController::onEvent(const Platform::Input::Touch::Event& e)
{
	if (e.type == Platform::Input::Touch::Event::Type::Begin && !ImGui::IsWindowHovered(ImGuiHoveredFlags_AnyWindow))
	{
		mLookAround = true;
		mCurrentAngles = { e.x, e.y };
		mPrevAngles = mCurrentAngles;

		if (e.y <= PLATFORM->getHeight() * 0.15f)
			mKeyW = true;
		else if (e.y >= PLATFORM->getHeight() * 0.85f)
			mKeyS = true;
		else if (e.x <= PLATFORM->getWidth() * 0.15f)
			mKeyA = true;
		else if (e.x >= PLATFORM->getWidth() * 0.85f)
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
		mCurrentAngles = { e.x, e.y };
	}
}