#pragma once

#include "external/glm/glm.hpp"

namespace based::graphics
{
	class Camera
	{
	public:
		Camera();
		Camera(const Camera& other);

		inline float GetAspectRatio() const { return mAspectRatio; }
		inline float GetHeight() const { return mHeight; }
		inline float GetNear() const { return mNear; }
		inline float GetFar() const { return mFar; }
		inline void SetViewMatrix(const glm::mat4& matrix) { mViewMatrix = matrix; }

		void SetAspectRatio(float aspectRatio);
		void SetHeight(float height);
		void SetNear(float near);
		void SetFar(float far);
		void SetOrtho(float height, float near = 0.f, float far = 100.f);
		void SetViewMatrix(const glm::vec3& pos, float rot);

		const glm::mat4& GetProjectionMatrix() const { return mProjectionMatrix; }
		const glm::mat4& GetViewMatrix() const { return mViewMatrix; }
	private:
		float mAspectRatio;
		float mHeight, mNear, mFar;

		glm::mat4 mProjectionMatrix;
		glm::mat4 mViewMatrix;

		void RecalculateProjectionMatrix();
	};
}