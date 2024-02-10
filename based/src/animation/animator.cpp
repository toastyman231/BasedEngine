#include <based/animation/animator.h>

#include "basedtime.h"
#include "external/glm/gtx/matrix_interpolation.hpp"
#include "math/basedmath.h"

namespace based::animation
{
	Animator::Animator(Animation* currentAnimation)
	{
		m_CurrentTime = 0.0f;
		m_BlendDelta = 0.0f;
		m_CurrentAnimation = currentAnimation;

		m_FinalBoneMatrices.reserve(100);

		for (int i = 0; i < 100; i++)
			m_FinalBoneMatrices.emplace_back(glm::mat4(1.f));
	}

	void Animator::UpdateAnimation(float dt)
	{
		m_DeltaTime = dt;
		m_BlendDelta = math::Clamp01(m_BlendDelta + m_DeltaTime * blendSpeed);
		if (m_CurrentAnimation && m_CurrentAnimation->IsPlaying())
		{
			m_CurrentTime += m_CurrentAnimation->GetTicksPerSecond() * dt;
			if (m_CurrentTime > m_CurrentAnimation->GetDuration() && !m_CurrentAnimation->IsLooping())
			{
				m_CurrentAnimation->SetPlaying(false);
				return;
			} else if (m_CurrentTime >= m_CurrentAnimation->GetDuration())
			{
				m_CurrentTime = fmod(m_CurrentTime, m_CurrentAnimation->GetDuration());
			}
			CalculateBoneTransform(&m_CurrentAnimation->GetRootNode(), glm::mat4(1.0f));
		}
	}

	void Animator::PlayAnimation(Animation* pAnimation)
	{
		m_OldAnimation = m_CurrentAnimation;
		m_PreviousTime = m_CurrentTime;
		m_CurrentAnimation = pAnimation;
		m_CurrentTime = 0.0f;
		m_BlendDelta = 0.0f;
		m_CurrentAnimation->SetPlaying(true);
	}

	void Animator::CalculateBoneTransform(const AssimpNodeData* node, const glm::mat4& parentTransform)
	{
		const std::string nodeName = node->name;
		glm::mat4 nodeTransform = node->transformation;

		Bone* bone = m_CurrentAnimation->FindBone(nodeName);
		Bone* previousBone = m_OldAnimation->FindBone(nodeName);

		if (bone && previousBone)
		{
			previousBone->Update(m_PreviousTime);
			bone->Update(m_CurrentTime);
			nodeTransform = (1.f - m_BlendDelta) * previousBone->GetLocalTransform() + 
				m_BlendDelta * bone->GetLocalTransform();
		} else if (bone)
		{
			bone->Update(m_CurrentTime);
			nodeTransform = bone->GetLocalTransform();
		}


		const glm::mat4 globalTransformation = parentTransform * nodeTransform;

		auto boneInfoMap = m_CurrentAnimation->GetBoneIDMap();
		if (boneInfoMap.find(nodeName) != boneInfoMap.end())
		{
			const int index = boneInfoMap[nodeName].id;
			const glm::mat4 offset = boneInfoMap[nodeName].offset;
			m_FinalBoneMatrices[index] = globalTransformation * offset;
		}

		for (int i = 0; i < node->childrenCount; i++)
			CalculateBoneTransform(&node->children[i], globalTransformation);
	}

	std::vector<glm::mat4> Animator::GetFinalBoneMatrices()
	{
		return m_FinalBoneMatrices;
	}
}
