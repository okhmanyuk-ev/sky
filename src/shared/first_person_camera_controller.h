#pragma once

#include <sky/dispatcher.h>
#include <common/frame_system.h>
#include <common/timestep_fixer.h>
#include <platform/system.h>
#include <platform/input.h>
#include <skygfx/utils.h>

namespace Shared
{
	class FirstPersonCameraController : public Common::FrameSystem::Frameable,
		public sky::Listenable<Platform::Input::Keyboard::Event>,
		public sky::Listenable<Platform::Input::Mouse::ButtonEvent>,
		public sky::Listenable<Platform::Input::Mouse::MoveEvent>,
		public sky::Listenable<Platform::Input::Mouse::ScrollEvent>,
		public sky::Listenable<Platform::Input::Touch::Event>
	{
	public:
		FirstPersonCameraController(std::shared_ptr<skygfx::utils::PerspectiveCamera> camera);
		~FirstPersonCameraController();

	private:
		void onFrame() override;
		void update(sky::Duration dTime);

	private:
		void onEvent(const Platform::Input::Keyboard::Event& e) override;
		void onEvent(const Platform::Input::Mouse::ButtonEvent& e) override;
		void onEvent(const Platform::Input::Mouse::MoveEvent& e) override;
		void onEvent(const Platform::Input::Mouse::ScrollEvent& e) override;
		void onEvent(const Platform::Input::Touch::Event& e) override;

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
		std::shared_ptr<skygfx::utils::PerspectiveCamera> mCamera;

	public:
		auto getSensivity() const { return mSensivity; }
		void setSensivity(float value) { mSensivity = value; }

		auto getSpeed() const { return mSpeed; }
		void setSpeed(float value) { mSpeed = value; }

	private:
		float mSensivity = 1.0f;
		float mSpeed = 1.0f;
		bool mMouseCaptured = false;
		glm::vec2 mSmoothDirection = { 0.0f, 0.0f };
		Common::TimestepFixer mTimestepFixer;
	};
}
