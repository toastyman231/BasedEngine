#pragma once

#include <Jolt/Jolt.h>
#include <Jolt/Math/Vec3.h>
#include <Jolt/Math/Quat.h>

inline JPH::Vec3 convert(const glm::vec3& inVec)
{
	return JPH::Vec3(inVec.x, inVec.y, inVec.z);
}

inline glm::vec3 convert(const JPH::Vec3& inVec)
{
	return glm::vec3(inVec.GetX(), inVec.GetY(), inVec.GetZ());
}

inline JPH::Vec4 convert(const glm::vec4& inVec)
{
	return JPH::Vec4(inVec.x, inVec.y, inVec.z, inVec.w);
}

inline glm::vec4 convert(const JPH::Vec4& inVec)
{
	return glm::vec4(inVec.GetX(), inVec.GetY(), inVec.GetZ(), inVec.GetW());
}

inline JPH::Mat44 convert(const glm::mat4x4& inMat)
{
	return JPH::Mat44(
		convert(inMat[0]),
		convert(inMat[1]),
		convert(inMat[2]),
		convert(inMat[3])
	);
}

inline glm::mat4x4 convert(const JPH::Mat44& inMat)
{
	return glm::mat4x4(
		convert(inMat.GetColumn4(0)),
		convert(inMat.GetColumn4(1)),
		convert(inMat.GetColumn4(2)),
		convert(inMat.GetColumn4(3))
	);
}