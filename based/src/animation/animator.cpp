#include "pch.h"
#include <based/animation/animator.h>

#include "basedtime.h"
#include "math/basedmath.h"

namespace based::animation
{
	Animator::Animator(const std::shared_ptr<Animation>& currentAnimation)
	{
		m_CurrentTime = 0.0f;
		m_BlendDelta = 0.0f;
		m_CurrentAnimation = currentAnimation;
		m_TimeMode = TimeMode::Scaled;

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
			if (auto anim = m_StateMachine->DetermineOutputAnimation().lock())
			{
				// TODO: Figure out how to make this work
				//m_StateMachine->GetCurrentState()->OnStateUpdate(std::shared_ptr<Animator>(this));
				if (auto cur = m_CurrentAnimation.lock())
				{
					if (anim != cur)
					{
						cur->SetPlaying(false);
						m_CurrentTime = cur->GetDuration() - 0.1f; // Prevents wonky transitions between states
						PlayAnimation(anim);
					}
				}
				else { BASED_WARN("Current anim is invalid!"); }
			}
			else { BASED_WARN("State machine output is invalid!"); }
		}

		if (auto cur = m_CurrentAnimation.lock())
		{
			if (!cur->IsPlaying() && m_StateMachine) return;

			m_CurrentTime += cur->GetTicksPerSecond() * dt * cur->GetPlaybackSpeed();
			if (m_CurrentTime > cur->GetDuration() && !cur->IsLooping())
			{
				cur->SetPlaying(false);
				return;
			} else if (m_CurrentTime >= cur->GetDuration())
			{
				m_CurrentTime = fmod(m_CurrentTime, cur->GetDuration());
			}
			CalculateBoneTransform(&cur->GetRootNode(), glm::mat4(1.0f),cur);
		}
	}

	void Animator::PlayAnimation(const std::shared_ptr<Animation>& pAnimation)
	{
		m_OldAnimation = m_CurrentAnimation;
		m_PreviousTime = m_CurrentTime;
		m_CurrentAnimation = pAnimation;
		m_CurrentTime = 0.0f;
		m_BlendDelta = 0.0f;
		pAnimation->SetPlaying(true);
	}

	void Animator::CalculateBoneTransform(const AssimpNodeData* node, const glm::mat4& parentTransform, 
		std::weak_ptr<Animation> animation)
	{
		const std::string nodeName = node->name;
		glm::mat4 nodeTransform = node->transformation;

		auto anim = animation.lock();
		auto cur = m_CurrentAnimation.lock();
		auto old = m_OldAnimation.lock();

		if (anim && cur && old)
		{
			Bone* bone = anim->FindBone(nodeName);
			Bone* previousBone = old->FindBone(nodeName);

			if (bone && previousBone && anim != old)
			{
				previousBone->Update(m_PreviousTime);
				bone->Update(m_CurrentTime);
				nodeTransform = (1.f - m_BlendDelta) * previousBone->GetLocalTransform() +
					m_BlendDelta * bone->GetLocalTransform();
			}
			else if (bone)
			{
				bone->Update(m_CurrentTime);
				nodeTransform = bone->GetLocalTransform();
			}

			const glm::mat4 globalTransformation = parentTransform * nodeTransform;

			auto boneInfoMap = cur->GetBoneIDMap();
			if (boneInfoMap.find(nodeName) != boneInfoMap.end())
			{
				const int index = boneInfoMap[nodeName].id;
				const glm::mat4 offset = boneInfoMap[nodeName].offset;
				m_FinalBoneMatrices[index] = globalTransformation * offset;
			}

			for (int i = 0; i < node->childrenCount; i++)
				CalculateBoneTransform(&node->children[i], globalTransformation, animation);
		} else if (anim && cur)
		{
			Bone* bone = anim->FindBone(nodeName);

			if (bone)
			{
				bone->Update(m_CurrentTime);
				nodeTransform = bone->GetLocalTransform();
			}

			const glm::mat4 globalTransformation = parentTransform * nodeTransform;

			auto boneInfoMap = cur->GetBoneIDMap();
			if (boneInfoMap.find(nodeName) != boneInfoMap.end())
			{
				const int index = boneInfoMap[nodeName].id;
				const glm::mat4 offset = boneInfoMap[nodeName].offset;
				m_FinalBoneMatrices[index] = globalTransformation * offset;
			}

			for (int i = 0; i < node->childrenCount; i++)
				CalculateBoneTransform(&node->children[i], globalTransformation, animation);
		}
	}

	std::vector<glm::mat4> Animator::GetFinalBoneMatrices()
	{
		return m_FinalBoneMatrices;
	}

	void AnimationState::AddTransition(const std::shared_ptr<AnimationTransition>& transition)
	{
		m_Transitions.emplace_back(transition);
	}

	std::weak_ptr<Animation> AnimationStateMachine::DetermineOutputAnimation()
	{
		std::shared_ptr<AnimationState> dest = nullptr;

		auto state = m_CurrentState.lock();
		auto animator = m_Animator.lock();

		if (state && animator)
		{
			bool shouldTransition = false;
			for (const auto& transition : state->GetTransitions())
			{
				if (transition->ShouldStateTransition())
				{
					shouldTransition = true;
					dest = transition->GetDestinationState().lock();
					break;
				}
			}

			if (shouldTransition && dest)
			{
				state->OnStateExit(animator);
				m_CurrentState = dest;
				dest->OnStateEnter(m_Animator.lock());
			}

			return state->GetStateAnimationClip();
		} else
		{
			BASED_WARN("Current state or animator is invalid!");
		}

		return m_CurrentState.lock()->GetStateAnimationClip();
	}

	void AnimationStateMachine::AddState(const std::shared_ptr<AnimationState>& state, bool isDefault)
	{
		m_States.emplace_back(state);
		if (isDefault) 
		{
			m_CurrentState = state;
			if (auto anim = m_Animator.lock())
			{
				if (auto clip = m_CurrentState.lock())
					anim->PlayAnimation(clip->GetStateAnimationClip());
			}
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
