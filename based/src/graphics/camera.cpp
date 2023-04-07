#include "graphics/camera.h"

#include "engine.h"
#include "external/glm/gtc/matrix_transform.hpp"

namespace based::graphics
{
	Camera::Camera()
		: mAspectRatio(16.f / 9.f)
		, mHeight(1080.f)
		, mNear(0.01f)
		, mFar(1000.f)
		, mFOV(60.f)
		, mProjectionMatrix(1.f)
		, mViewMatrix(1.f)
	{
		RecalculateProjectionMatrix();
	}

	Camera::Camera(const Camera& other)
		: mAspectRatio(other.mAspectRatio)
		, mHeight(other.mHeight)
		, mNear(other.mNear)
		, mFar(other.mFar)
		, mFOV(60.f)
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
		if (projection != ORTHOGRAPHIC) return;

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

	void Camera::SetFOV(float fov)
	{
		mFOV = fov;
		RecalculateProjectionMatrix();
	}

	void Camera::SetProjection(Projection newProjection)
	{
		projection = newProjection;
		RecalculateProjectionMatrix();
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

	void Camera::SetPerspective(float fov, float near, float far)
	{
		bool shouldRecalculate = false;

		if (mFOV != fov)
		{
			mFOV = fov;
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

	void Camera::SetViewMatrix(const glm::vec3& pos, const glm::vec3& front, const glm::vec3& up)
	{
		glm::mat4 view = glm::mat4(1.f);
		view = glm::lookAt(pos, pos + front, up);
		SetViewMatrix(view);
	}

	const glm::vec3 Camera::ScreenToWorldPoint(float x, float y) const
	{
		// NORMALIZED DEVICE SPACE
		const double xNorm = 2.0 * x / Engine::Instance().GetWindow().GetSize().x - 1;
		const double yNorm = 2.0 * y / Engine::Instance().GetWindow().GetSize().y - 1;
		// HOMOGENEOUS SPACE
		const glm::vec4 screenPos = glm::vec4(xNorm, -yNorm, 0.f, 1.0f);
		// Projection/Eye Space
		const glm::mat4 ProjectView = mProjectionMatrix * mViewMatrix;
		const float det = glm::determinant(ProjectView);
		if (det == 0.f) return glm::vec3(0.f);

		const glm::mat4 viewProjectionInverse = inverse(ProjectView);
		const glm::vec4 worldPos = viewProjectionInverse * screenPos;
		return glm::vec3(worldPos);
	}

	const glm::vec3 Camera::ScreenToWorldPoint(glm::vec2 point) const
	{
		return ScreenToWorldPoint(point.x, point.y);
	}

	void Camera::RecalculateProjectionMatrix()
	{
		float halfwidth = mHeight * mAspectRatio * 0.5f;
		float halfheight = mHeight * 0.5f;

		switch (projection)
		{
		case ORTHOGRAPHIC:
			mProjectionMatrix = glm::ortho(-halfwidth, halfwidth, -halfheight, halfheight, mNear, mFar);
			break;
		case PERSPECTIVE:
			mProjectionMatrix = glm::perspective(glm::radians(mFOV), mAspectRatio, mNear, mFar);
			break;
		}
	}
}