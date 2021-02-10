#pragma once

#include <common/frame_system.h>
#include <platform/system.h>
#include <glm/glm.hpp>
#include <glm/ext.hpp>

namespace Graphics
{
	class Camera
	{
	public:
		virtual glm::mat4 getViewMatrix() const = 0;
		virtual glm::mat4 getProjectionMatrix() const = 0;
	};

	class Camera2D : public Camera
	{
	public:
		glm::mat4 getViewMatrix() const override;
		glm::mat4 getProjectionMatrix() const override;

	public:
		auto getPosition() const { return mPosition; }
		void setPosition(glm::vec2 value) { mPosition = value; }
		
	private:
		glm::vec2 mPosition = { 0.0f, 0.0f };
	};

	class Camera3D : public Camera, public Common::FrameSystem::Frameable
	{
	public:
		Camera3D(float fieldOfView, glm::vec3 worldUp, float nearPlane = 1.0f, float farPlane = 8192.0f);

	public:
		void onFrame() override;

	public:
		void frontMove(float offset);
		void sideMove(float offset);

	public:
		glm::mat4 getViewMatrix() const override { return mViewMatrix; }
		glm::mat4 getProjectionMatrix() const override { return mProjectionMatrix; }

	private:
		glm::mat4 mViewMatrix = glm::mat4(1.0f);
		glm::mat4 mProjectionMatrix = glm::mat4(1.0f);
	
	public:
		auto getPosition() const { return mPosition; }
		void setPosition(glm::vec3 value) { mPosition = value; }

		auto getYaw() const { return mYaw; }
		void setYaw(float value) { mYaw = value; }

		auto getPitch() const { return mPitch; }
		void setPitch(float value) { mPitch = value; }

		auto getFieldOfView() const { return mFieldOfView; }
		void setFieldOfView(float value) { mFieldOfView = value; }

		auto getNearPlane() const { return mNearPlane; }
		void setNearPlane(float value) { mNearPlane = value; }

		auto getFarPlane() const { return mFarPlane; }
		void setFarPlane(float value) { mFarPlane = value; }

		auto getFront() const { return mFront; }
		auto getUp() const { return mUp; }
		auto getRight() const { return mRight; }

	private:
		glm::vec3 mPosition = { 0.0f, 0.0f, 0.0f };
		float mYaw = 0.0f;
		float mPitch = 0.0f;
		float mFieldOfView;
		glm::vec3 mWorldUp;
		float mNearPlane;
		float mFarPlane;

	private:
		glm::vec3 mFront;
		glm::vec3 mUp;
		glm::vec3 mRight;
	};



	// TODO: add Roll

	/*class Camera
	{
	public:
		Camera(float fieldOfView, glm::vec3 worldUp, float nearPlane, float farPlane);
		
		void updateMatrix(); // use Common:LoopSystem for updating

		glm::mat4 getViewMatrix();
		glm::mat4 getProjectionMatrix(float width, float height);

		void frontMove(float offset);
		void sideMove(float offset);
		// TODO: upMove ? (this is not jump in 3d games, you know) 

	public:
		auto getPosition() const { return mPosition; }
		void setPosition(glm::vec3 value) { mPosition = value; }

		auto getYaw() const { return mYaw; } 
		void setYaw(float value) { mYaw = value; }

		auto getPitch() const { return mPitch; }
		void setPitch(float value) { mPitch = value; }

		auto getFieldOfView() const { return mFieldOfView; }
		void setFieldOfView(float value) { mFieldOfView = value; }

		auto getNearPlane() const { return mNearPlane; }
		void setNearPlane(float value) { mNearPlane = value; }

		auto getFarPlane() const { return mFarPlane; }
		void setFarPlane(float value) { mFarPlane = value; }

		auto getFront() const { return mFront; }
		auto getUp() const { return mUp; }
		auto getRight() const { return mRight; }

	private:
		glm::vec3 mPosition;

		float mYaw;
		float mPitch;

		float mFieldOfView;
		
		float mNearPlane;
		float mFarPlane;

		glm::vec3 mFront;
		glm::vec3 mUp;
		glm::vec3 mRight;
		glm::vec3 mWorldUp;
	};*/
}