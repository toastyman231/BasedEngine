#pragma once

namespace based::graphics
{
	class AssimpGLMHelpers
	{
	public:

		// Evil void* abuse here so that I don't have to expose Assimp to the user (don't ask)
		static glm::mat4 ConvertMatrixToGLMFormat(const void* from);

		static glm::vec3 GetGLMVec(const void* vec);

		static glm::quat GetGLMQuat(const void* pOrientation);
	};
}