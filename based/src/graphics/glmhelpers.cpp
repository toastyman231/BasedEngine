#include "pch.h"
#include "graphics/glmhelpers.h"

#include <assimp/matrix4x4.h>
#include <assimp/vector3.h>
#include <assimp/quaternion.h>

glm::mat4 based::graphics::AssimpGLMHelpers::ConvertMatrixToGLMFormat(const void* from)
{
	glm::mat4 to;
	const aiMatrix4x4* temp = static_cast<const aiMatrix4x4*>(from);
	//the a,b,c,d in assimp is the row ; the 1,2,3,4 is the column
	to[0][0] = temp->a1; to[1][0] = temp->a2; to[2][0] = temp->a3; to[3][0] = temp->a4;
	to[0][1] = temp->b1; to[1][1] = temp->b2; to[2][1] = temp->b3; to[3][1] = temp->b4;
	to[0][2] = temp->c1; to[1][2] = temp->c2; to[2][2] = temp->c3; to[3][2] = temp->c4;
	to[0][3] = temp->d1; to[1][3] = temp->d2; to[2][3] = temp->d3; to[3][3] = temp->d4;
	return to;
}

glm::vec3 based::graphics::AssimpGLMHelpers::GetGLMVec(const void* vec)
{
	const aiVector3D* temp = static_cast<const aiVector3D*>(vec);
	return glm::vec3(temp->x, temp->y, temp->z);
}

glm::quat based::graphics::AssimpGLMHelpers::GetGLMQuat(const void* pOrientation)
{
	const aiQuaternion* temp = static_cast<const aiQuaternion*>(pOrientation);
	return glm::quat(temp->w, temp->x, temp->y, temp->z);
}
