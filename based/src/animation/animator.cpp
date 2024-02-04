#include <based/animation/animator.h>

namespace based::animation
{
	Animator::Animator(Animation* currentAnimation)
	{
		m_CurrentTime = 0.0;
		m_CurrentAnimation = currentAnimation;

		m_FinalBoneMatrices.reserve(100);

		for (int i = 0; i < 100; i++)
			m_FinalBoneMatrices.push_back(glm::mat4(1.0f));
	}

	void Animator::UpdateAnimation(float dt)
	{
		m_DeltaTime = dt;
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
		m_CurrentAnimation = pAnimation;
		m_CurrentTime = 0.0f;
		m_CurrentAnimation->SetPlaying(true);
	}

	void Animator::CalculateBoneTransform(const AssimpNodeData* node, const glm::mat4& parentTransform)
	{
		std::string nodeName = node->name;
		glm::mat4 nodeTransform = node->transformation;

		Bone* Bone = m_CurrentAnimation->FindBone(nodeName);

		if (Bone)
		{
			Bone->Update(m_CurrentTime);
			nodeTransform = Bone->GetLocalTransform();
		}

		glm::mat4 globalTransformation = parentTransform * nodeTransform;

		auto boneInfoMap = m_CurrentAnimation->GetBoneIDMap();
		if (boneInfoMap.find(nodeName) != boneInfoMap.end())
		{
			int index = boneInfoMap[nodeName].id;
			glm::mat4 offset = boneInfoMap[nodeName].offset;
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
