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