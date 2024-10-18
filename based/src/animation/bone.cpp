#include "pch.h"
#include <based/animation/bone.h>

namespace based::animation
{
	Bone::Bone(std::string name, int ID, const aiNodeAnim* channel)
		: m_LocalTransform(1.0f)
		, m_Name(std::move(name))
		, m_ID(ID)
	{
		m_NumPositions = channel->mNumPositionKeys;
		m_Positions.reserve(m_NumPositions);

		for (int positionIndex = 0; positionIndex < m_NumPositions; ++positionIndex)
		{
			aiVector3D aiPosition = channel->mPositionKeys[positionIndex].mValue;
			auto timeStamp = static_cast<float>(channel->mPositionKeys[positionIndex].mTime);
			KeyPosition data;
			data.position = graphics::AssimpGLMHelpers::GetGLMVec(aiPosition);
			data.timeStamp = timeStamp;
			m_Positions.push_back(data);
		}

		m_NumRotations = channel->mNumRotationKeys;
		m_Rotations.reserve(m_NumRotations);
		for (int rotationIndex = 0; rotationIndex < m_NumRotations; ++rotationIndex)
		{
			aiQuaternion aiOrientation = channel->mRotationKeys[rotationIndex].mValue;
			auto timeStamp = static_cast<float>(channel->mRotationKeys[rotationIndex].mTime);
			KeyRotation data;
			data.orientation = graphics::AssimpGLMHelpers::GetGLMQuat(aiOrientation);
			data.timeStamp = timeStamp;
			m_Rotations.push_back(data);
		}

		m_NumScalings = channel->mNumScalingKeys;
		m_Scales.reserve(m_NumScalings);
		for (int keyIndex = 0; keyIndex < m_NumScalings; ++keyIndex)
		{
			aiVector3D scale = channel->mScalingKeys[keyIndex].mValue;
			auto timeStamp = static_cast<float>(channel->mScalingKeys[keyIndex].mTime);
			KeyScale data;
			data.scale = graphics::AssimpGLMHelpers::GetGLMVec(scale);
			data.timeStamp = timeStamp;
			m_Scales.push_back(data);
		}
	}

	/*interpolates  b/w positions,rotations & scaling keys based on the current time of
	the animation and prepares the local transformation matrix by combining all keys
	transformations*/
	void Bone::Update(float animationTime)
	{
		const glm::mat4 translation = InterpolatePosition(animationTime);
		const glm::mat4 rotation = InterpolateRotation(animationTime);
		const glm::mat4 scale = InterpolateScaling(animationTime);
		m_LocalTransform = translation * rotation * scale;
	}

	/* Gets the current index on mKeyPositions to interpolate to based on
	the current animation time*/
	int Bone::GetPositionIndex(float animationTime) const
	{
		for (int index = 0; index < m_NumPositions - 1; ++index)
		{
			if (animationTime < m_Positions[index + 1].timeStamp)
				return index;
		}
		//assert(0);
		return 0;
	}

	/* Gets the current index on mKeyRotations to interpolate to based on the
	current animation time*/
	int Bone::GetRotationIndex(float animationTime) const
	{
		for (int index = 0; index < m_NumRotations - 1; ++index)
		{
			if (animationTime < m_Rotations[index + 1].timeStamp)
				return index;
		}
		//assert(0);
		return 0;
	}

	/* Gets the current index on mKeyScalings to interpolate to based on the
	current animation time */
	int Bone::GetScaleIndex(float animationTime) const
	{
		for (int index = 0; index < m_NumScalings - 1; ++index)
		{
			if (animationTime < m_Scales[index + 1].timeStamp)
				return index;
		}
		//assert(0);
		return 0;
	}

	/* Gets normalized value for Lerp & Slerp*/
	float Bone::GetScaleFactor(float lastTimeStamp, float nextTimeStamp, float animationTime)
	{
		float scaleFactor = 0.0f;
		const float midWayLength = animationTime - lastTimeStamp;
		const float framesDiff = nextTimeStamp - lastTimeStamp;
		scaleFactor = midWayLength / framesDiff;
		return scaleFactor;
	}

	/*figures out which position keys to interpolate b/w and performs the interpolation
	and returns the translation matrix*/
	glm::mat4 Bone::InterpolatePosition(float animationTime)
	{
		if (1 == m_NumPositions)
			return glm::translate(glm::mat4(1.0f), m_Positions[0].position);

		const int p0Index = GetPositionIndex(animationTime);
		const int p1Index = p0Index + 1;
		const float scaleFactor = GetScaleFactor(m_Positions[p0Index].timeStamp,
		                                         m_Positions[p1Index].timeStamp, animationTime);
		const glm::vec3 finalPosition = glm::mix(m_Positions[p0Index].position,
		                                         m_Positions[p1Index].position, scaleFactor);
		return glm::translate(glm::mat4(1.0f), finalPosition);
	}

	/*figures out which rotations keys to interpolate b/w and performs the interpolation
	and returns the rotation matrix*/
	glm::mat4 Bone::InterpolateRotation(float animationTime)
	{
		if (1 == m_NumRotations)
		{
			const auto rotation = glm::normalize(m_Rotations[0].orientation);
			return glm::toMat4(rotation);
		}

		const int p0Index = GetRotationIndex(animationTime);
		const int p1Index = p0Index + 1;
		const float scaleFactor = GetScaleFactor(m_Rotations[p0Index].timeStamp,
		                                         m_Rotations[p1Index].timeStamp, animationTime);
		glm::quat finalRotation = glm::slerp(m_Rotations[p0Index].orientation,
			m_Rotations[p1Index].orientation, scaleFactor);
		finalRotation = glm::normalize(finalRotation);
		return glm::toMat4(finalRotation);
	}

	/*figures out which scaling keys to interpolate b/w and performs the interpolation
	and returns the scale matrix*/
	glm::mat4 Bone::InterpolateScaling(float animationTime)
	{
		if (1 == m_NumScalings)
			return glm::scale(glm::mat4(1.0f), m_Scales[0].scale);

		const int p0Index = GetScaleIndex(animationTime);
		const int p1Index = p0Index + 1;
		const float scaleFactor = GetScaleFactor(m_Scales[p0Index].timeStamp,
		                                         m_Scales[p1Index].timeStamp, animationTime);
		const glm::vec3 finalScale = glm::mix(m_Scales[p0Index].scale, m_Scales[p1Index].scale
		                                      , scaleFactor);
		return glm::scale(glm::mat4(1.0f), finalScale);
	}
}
