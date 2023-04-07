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
		, mPosition(0.f, 0.f, 1.5f)
		, mTarget(0.f, 0.f, -1.f)
		, mUp(0.f, 1.f, 0.f)
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

	void Camera::SetNear(float nearPlane)
	{
		if (mNear != nearPlane)
		{
			mNear = nearPlane;
			RecalculateProjectionMatrix();
		}
	}

	void Camera::SetFar(float farPlane)
	{
		if (mFar != farPlane)
		{
			mFar = farPlane;
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

	void Camera::SetOrtho(float height, float nearPlane, float farPlane)
	{
		bool shouldRecalculate = false;

		if (mHeight != height)
		{
			mHeight = height;
			shouldRecalculate = true;
		}
		if (mNear != nearPlane)
		{
			mNear = nearPlane;
			shouldRecalculate = true;
		}
		if (mFar != farPlane)
		{
			mFar = farPlane;
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

	void Camera::SetTransform(glm::vec3 pos, glm::vec3 rot, glm::vec3 scale)
	{
		//mPosition = pos;
		//mTarget = rot;
		mTransform.Position = pos;
		mTransform.Rotation = rot;
		mTransform.Scale = scale;

		glm::mat4 view = glm::mat4(1.f);
		view = glm::translate(view, pos);
		view = glm::rotate(view, glm::radians(rot.x), glm::vec3(1, 0, 0));
		view = glm::rotate(view, glm::radians(rot.y), glm::vec3(0, 1, 0));
		view = glm::rotate(view, glm::radians(rot.z), glm::vec3(0, 0, 1));
		view = glm::scale(view, glm::vec3(1.f));
		view = glm::inverse(view);

		SetViewMatrix(view);
		//const glm::vec3 forward = glm::normalize(glm::vec3(view[2]));
		//const glm::vec3 up = glm::vec3(view[0], view[4], view[8]);

		//SetViewMatrix(mPosition, mTarget, mUp);
	}

	void Camera::SetPosition(glm::vec3 pos)
	{
		SetTransform(pos, mTransform.Rotation, mTransform.Scale);
		//mPosition = pos * mTarget;

		//SetViewMatrix(mPosition, mTarget, mUp);
	}

	void Camera::SetRotation(glm::vec3 rot)
	{
		SetTransform(mTransform.Position, rot, mTransform.Scale);
	}

	void Camera::SetScale(glm::vec3 scale)
	{
		SetTransform(mTransform.Position, mTransform.Rotation, scale);
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