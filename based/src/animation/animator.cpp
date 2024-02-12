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

		if (m_StateMachine)
		{
			Animation* anim = m_StateMachine->DetermineOutputAnimation();
			m_StateMachine->GetCurrentState()->OnStateUpdate(this);
			if (anim != m_CurrentAnimation)
			{
				m_CurrentAnimation->SetPlaying(false);
				m_CurrentTime = m_CurrentAnimation->GetDuration() - 0.1f; // Prevents wonky transitions between states
				PlayAnimation(anim);
			}
		}

		if (m_CurrentAnimation && m_CurrentAnimation->IsPlaying())
		{
			m_CurrentTime += m_CurrentAnimation->GetTicksPerSecond() * dt * m_CurrentAnimation->GetPlaybackSpeed();
			if (m_CurrentTime > m_CurrentAnimation->GetDuration() && !m_CurrentAnimation->IsLooping())
			{
				m_CurrentAnimation->SetPlaying(false);
				return;
			} else if (m_CurrentTime >= m_CurrentAnimation->GetDuration())
			{
				m_CurrentTime = fmod(m_CurrentTime, m_CurrentAnimation->GetDuration());
			}
			CalculateBoneTransform(&m_CurrentAnimation->GetRootNode(), glm::mat4(1.0f), 
				m_CurrentAnimation);
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

	void Animator::CalculateBoneTransform(const AssimpNodeData* node, const glm::mat4& parentTransform, 
		Animation* animation)
	{
		const std::string nodeName = node->name;
		glm::mat4 nodeTransform = node->transformation;

		Bone* bone = animation->FindBone(nodeName);
		Bone* previousBone = m_OldAnimation->FindBone(nodeName);

		if (bone && previousBone && animation != m_OldAnimation)
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
			CalculateBoneTransform(&node->children[i], globalTransformation, animation);
	}

	std::vector<glm::mat4> Animator::GetFinalBoneMatrices()
	{
		return m_FinalBoneMatrices;
	}

	void AnimationState::AddTransition(AnimationTransition* transition)
	{
		m_Transitions.emplace_back(transition);
	}

	Animation* AnimationStateMachine::DetermineOutputAnimation()
	{
		bool shouldTransition = false;
		AnimationState* dest = nullptr;
		for (const auto transition : m_CurrentState->GetTransitions())
		{
			if (transition->ShouldStateTransition())
			{
				shouldTransition = true;
				dest = transition->GetDestinationState();
				break;
			}
		}

		if (shouldTransition && dest)
		{
			m_CurrentState->OnStateExit(m_Animator);
			m_CurrentState = dest;
			m_CurrentState->OnStateEnter(m_Animator);
		}

		return m_CurrentState->GetStateAnimationClip();
	}

	void AnimationStateMachine::AddState(AnimationState* state, bool isDefault)
	{
		m_States.emplace_back(state);
		if (isDefault) 
		{
			m_CurrentState = state;
			m_Animator->PlayAnimation(m_CurrentState->GetStateAnimationClip());
		}
	}

	void AnimationStateMachine::SetFloat(const std::string& name, float value)
	{
		m_FloatParameters.insert_or_assign(name, value);
	}

	void AnimationStateMachine::SetInteger(const std::string& name, int value)
	{
		m_IntParameters.insert_or_assign(name, value);
	}

	void AnimationStateMachine::SetBool(const std::string& name, bool value)
	{
		m_BoolParameters.insert_or_assign(name, value);
	}

	void AnimationStateMachine::SetString(const std::string& name, std::string& value)
	{
		m_StringParameters.insert_or_assign(name, value);
	}

	float AnimationStateMachine::GetFloat(const std::string& name, float defaultValue)
	{
		if (const auto res = m_FloatParameters.find(name); res != m_FloatParameters.end())
		{
			return res->second;
		}
		else return defaultValue;
	}

	int AnimationStateMachine::GetInteger(const std::string& name, int defaultValue)
	{
		if (const auto res = m_IntParameters.find(name); res != m_IntParameters.end())
		{
			return res->second;
		}
		else return defaultValue;
	}

	bool AnimationStateMachine::GetBool(const std::string& name, bool defaultValue)
	{
		if (const auto res = m_BoolParameters.find(name); res != m_BoolParameters.end())
		{
			return res->second;
		}
		else return defaultValue;
	}

	std::string AnimationStateMachine::GetString(const std::string& name, std::string defaultValue)
	{
		if (const auto res = m_StringParameters.find(name); res != m_StringParameters.end())
		{
			return res->second;
		}
		else return defaultValue;
	}
}
