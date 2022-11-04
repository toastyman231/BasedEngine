#include "graphics/camera.h"
#include "external/glm/gtc/matrix_transform.hpp"

namespace based::graphics
{
	Camera::Camera()
		: mProjectionMatrix(1.f)
		, mViewMatrix(1.f)
		, mAspectRatio(16.f / 9.f)
		, mHeight(1080.f)
		, mNear(0.f)
		, mFar(100.f)
	{
		RecalculateProjectionMatrix();
	}

	Camera::Camera(const Camera& other)
		: mAspectRatio(other.mAspectRatio)
		, mHeight(other.mHeight)
		, mNear(other.mNear)
		, mFar(other.mFar)
	{
		RecalculateProjectionMatrix();
	}

	void Camera::SetAspectRatio(float aspectRatio)
	{
		if (mAspectRatio != aspectRatio)
		{
			mAspectRatio = aspectRatio;
			RecalculateProjectionMatrix();
		}
	}

	void Camera::SetHeight(float height)
	{
		if (mHeight != height)
		{
			mHeight = height;
			RecalculateProjectionMatrix();
		}
	}

	void Camera::SetNear(float near)
	{
		if (mNear != near)
		{
			mNear = near;
			RecalculateProjectionMatrix();
		}
	}

	void Camera::SetFar(float far)
	{
		if (mFar != far)
		{
			mFar = far;
			RecalculateProjectionMatrix();
		}
	}

	void Camera::SetOrtho(float height, float near, float far)
	{
		bool shouldRecalculate = false;

		if (mHeight != height)
		{
			mHeight = height;
			shouldRecalculate = true;
		}
		if (mNear != near)
		{
			mNear = near;
			shouldRecalculate = true;
		}
		if (mFar != far)
		{
			mFar = far;
			shouldRecalculate = true;
		}
		if (shouldRecalculate)
		{
			RecalculateProjectionMatrix();
		}
	}

	void Camera::SetViewMatrix(const glm::vec3& pos, float rot)
	{
		glm::mat4 view = glm::mat4(1.f);
		view = glm::translate(view, pos);
		view = glm::rotate(view, glm::radians(rot), glm::vec3(0, 0, 1));
		view = glm::inverse(view);
		SetViewMatrix(view);

	}

	void Camera::RecalculateProjectionMatrix()
	{
		float halfwidth = mHeight * mAspectRatio * 0.5f;
		float halfheight = mHeight * 0.5f;
		mProjectionMatrix = glm::ortho(-halfwidth, halfwidth, -halfheight, halfheight, mNear, mFar);
	}
}