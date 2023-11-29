#pragma once

#include "external/glm/glm.hpp"
#include "scene/components.h"

namespace based::graphics
{
	enum Projection
	{
		ORTHOGRAPHIC,
		PERSPECTIVE
	};

	class Camera
	{
	public:
		Camera();
		Camera(const Camera& other);

		Projection projection = ORTHOGRAPHIC;

		inline float GetAspectRatio() const { return mAspectRatio; }
		inline float GetHeight() const { return mHeight; }
		inline float GetNear() const { return mNear; }
		inline float GetFar() const { return mFar; }
		inline float GetFOV() const { return mFOV; }
		inline void SetViewMatrix(const glm::mat4& matrix) { mViewMatrix = matrix; }
		inline scene::Transform GetTransform() const { return mTransform; }

		void SetAspectRatio(float aspectRatio);
		void SetHeight(float height);
		void SetNear(float nearPlane);
		void SetFar(float farPlane);
		void SetFOV(float fov);
		void SetProjection(Projection newProjection);
		void SetOrtho(float height, float nearPlane = 0.f, float farPlane = 100.f);
		void SetViewMatrix(const glm::vec3& pos, float rot);
		void SetViewMatrix(const glm::vec3& pos, const glm::vec3& front, const glm::vec3& up);
		void SetTransform(glm::vec3 pos, glm::vec3 rot, glm::vec3 scale);
		void SetPosition(glm::vec3 pos);
		void SetRotation(glm::vec3 rot);
		void SetScale(glm::vec3 scale);

		const glm::mat4& GetProjectionMatrix() const { return mProjectionMatrix; }
		const glm::mat4& GetViewMatrix() const { return mViewMatrix; }
		glm::vec3 GetForward() const { return mForward; }
		const glm::vec3 GetRight() { return mRight; }
		const glm::vec3 GetUp() { return mUp; }
		const glm::vec3 ScreenToWorldPoint(float x, float y) const;
		const glm::vec3 ScreenToWorldPoint(glm::vec2 point) const;
	private:
		float mAspectRatio;
		float mHeight, mNear, mFar;
		float mFOV;

		glm::vec3 mForward;
		glm::vec3 mRight;
		glm::vec3 mUp;

		scene::Transform mTransform;

		glm::mat4 mProjectionMatrix{};
		glm::mat4 mViewMatrix{};

		void RecalculateProjectionMatrix();
	};
}
