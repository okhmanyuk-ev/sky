#pragma once

#include <graphics/camera.h>
#include <common/event_system.h>
#include <common/frame_system.h>
#include <platform/system.h>

#include <platform/keyboard.h>
#include <platform/mouse.h>
#include <platform/touch.h>

#include <shared/console_device.h>

namespace Shared
{
	class FirstPersonCameraController : public Common::FrameSystem::Frameable,
		public Common::EventSystem::Listenable<Platform::Keyboard::Event>,
		public Common::EventSystem::Listenable<Platform::Mouse::Event>,
		public Common::EventSystem::Listenable<Platform::Touch::Event>
	{
	public:
		FirstPersonCameraController(Graphics::Camera3D& camera);
		~FirstPersonCameraController();

	private:
		void frame() override;

	private:
		void event(const Platform::Keyboard::Event& e) override;
		void event(const Platform::Mouse::Event& e) override;
		void event(const Platform::Touch::Event& e) override;

	private:
		bool mLookAround = false;
		glm::vec2 mStartAngles = { 0.0f, 0.0f };
		glm::vec2 mPrevAngles = { 0.0f, 0.0f };
		glm::vec2 mCurrentAngles = { 0.0f, 0.0f };

		bool mLeftArrow = false;
		bool mRightArrow = false;
		bool mUpArrow = false;
		bool mDownArrow = false;

		bool mKeyW = false;
		bool mKeyA = false;
		bool mKeyS = false;
		bool mKeyD = false;

		bool mKeyShift = false;
		bool mKeyCtrl = false;

	private:
		Graphics::Camera3D& mCamera;

	public:
		auto getSensivity() const { return mSensivity; }
		void setSensivity(float value) { mSensivity = value; }

		auto getSpeed() const { return mSpeed; }
		void setSpeed(float value) { mSpeed = value; }

	private:
		float mSensivity = 1.0f;
		float mSpeed = 1.0f;
		bool mMouseCaptured = false;
	};
}