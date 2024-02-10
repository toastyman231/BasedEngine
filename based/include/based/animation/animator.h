#pragma once
#include "animation.h"

namespace based::animation
{
	class Animator
	{
	public:
		Animator(Animation* animation);
		void UpdateAnimation(float dt);
		void PlayAnimation(Animation* pAnimation);
		void CalculateBoneTransform(const AssimpNodeData* node, const glm::mat4& parentTransform);
		std::vector<glm::mat4> GetFinalBoneMatrices();
		float blendSpeed = 5.f;
	private:
		std::vector<glm::mat4> m_FinalBoneMatrices;
		Animation* m_CurrentAnimation;
		Animation* m_OldAnimation;
		float m_PreviousTime;
		float m_CurrentTime;
		float m_DeltaTime;
		float m_BlendDelta;
	};
}
