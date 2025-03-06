#include "pch.h"
#include "graphics/camera.h"

#include "engine.h"

namespace based::graphics
{
	Camera::Camera()
		: mAspectRatio(16.f / 9.f)
		, mHeight(1080.f)
		, mNear(0.01f)
		, mFar(1000.f)
		, mFOV(60.f)
		, mForward(0.f, 0.f, -1.f)
		, mRight(0.f, 0.f, 1.f)
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
		, mFOV(other.mFOV)
	{
		RecalculateProjectionMatrix();
	}

	Camera& Camera::operator=(const Camera& other)
	{
		if (this != &other)
		{
			mAspectRatio = other.mAspectRatio;
			mHeight = other.mHeight;
			mNear = other.mNear;
			mFar = other.mFar;
			mFOV = other.mFOV;
			main = false;
			RecalculateProjectionMatrix();
		}
		return *this;
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
		PROFILE_FUNCTION();
		mTransform.SetGlobalTransform(pos, rot, scale);

		glm::mat4 view = glm::mat4(1.f);
		view = glm::rotate(view, glm::radians(rot.z), glm::vec3(0, 0, 1));
		view = glm::rotate(view, glm::radians(rot.x), glm::vec3(1, 0, 0));
		view = glm::rotate(view, glm::radians(rot.y), glm::vec3(0, 1, 0));
		view = glm::translate(view, -pos);

		mForward = glm::normalize(glm::vec3(view[0][2], view[1][2], view[2][2]));
		mRight = glm::normalize(glm::cross(glm::vec3(0.f, 1.f, 0.f), mForward));
		mUp = glm::cross(mForward, mRight);

		SetViewMatrix(view);
	}

	void Camera::SetPosition(glm::vec3 pos)
	{
		SetTransform(pos, mTransform.Rotation(), mTransform.Scale());
	}

	void Camera::SetRotation(glm::vec3 rot)
	{
		SetTransform(mTransform.Position(), rot, mTransform.Scale());
	}

	void Camera::SetScale(glm::vec3 scale)
	{
		SetTransform(mTransform.Position(), mTransform.Rotation(), scale);
	}

	const glm::vec3 Camera::ScreenToWorldPoint(float x, float y) const
	{
		PROFILE_FUNCTION();
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

	std::string Camera::GetProjectionString(Projection projection)
	{
		switch(projection)
		{
		case ORTHOGRAPHIC:
			return "Orthographic";
		case PERSPECTIVE:
			return "Perspective";
		}

		return "PROJECTION ERROR";
	}

	void Camera::RecalculateProjectionMatrix()
	{
		PROFILE_FUNCTION();
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