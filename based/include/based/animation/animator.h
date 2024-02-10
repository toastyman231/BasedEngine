#pragma once
#include "animation.h"

namespace based::animation
{
	class AnimationStateMachine;
	class AnimationTransition;

	class Animator
	{
	public:
		Animator(Animation* animation);
		void UpdateAnimation(float dt);
		void PlayAnimation(Animation* pAnimation);
		void CalculateBoneTransform(const AssimpNodeData* node, const glm::mat4& parentTransform, 
			Animation* animation);
		std::vector<glm::mat4> GetFinalBoneMatrices();
		float blendSpeed = 5.f;

		inline float GetPlaybackTime() const { return m_CurrentTime; }
		inline void SetStateMachine(AnimationStateMachine* machine) { m_StateMachine = machine; }
		inline AnimationStateMachine* GetStateMachine() const { return m_StateMachine; }
	private:
		std::vector<glm::mat4> m_FinalBoneMatrices;
		Animation* m_CurrentAnimation;
		Animation* m_OldAnimation;
		AnimationStateMachine* m_StateMachine;
		float m_PreviousTime;
		float m_CurrentTime;
		float m_DeltaTime;
		float m_BlendDelta;
	};

	class AnimationState
	{
	public:
		AnimationState() = default;
		AnimationState(Animation* anim) : m_Animation(anim), m_Speed(1.f) {}
		virtual ~AnimationState() = default;

		virtual void OnStateEnter(Animator* animator) {}
		virtual void OnStateExit(Animator* animator) {}
		virtual void OnStateUpdate(Animator* animator) {}

		std::vector<AnimationTransition*> GetTransitions() const { return m_Transitions; }
		void AddTransition(AnimationTransition* transition);

		inline void SetPlaybackSpeed(float speed) { m_Speed = speed; }
		inline float GetPlaybackSpeed() const { return m_Speed; }
		inline Animation* GetStateAnimationClip() const { return m_Animation; }
	private:
		Animation* m_Animation;
		std::vector<AnimationTransition*> m_Transitions;
		float m_Speed;
	};

	class AnimationTransition
	{
	public:
		AnimationTransition() = default;
		AnimationTransition(AnimationState* source, AnimationState* destination, 
			Animator* animator, std::function<bool()> predicate = nullptr)
			: m_Source(source), m_Destination(destination), m_Animator(animator)
			, m_Predicate(std::move(predicate)) {}
		virtual ~AnimationTransition() = default;

		virtual bool ShouldStateTransition() {
			if (m_Predicate != nullptr) return m_Predicate();
			else return m_Animator->GetPlaybackTime() >=
				m_Source->GetStateAnimationClip()->GetDuration();
		}

		inline AnimationState* GetSourceState() const { return m_Source; }
		inline AnimationState* GetDestinationState() const { return m_Destination; }
	private:
		AnimationState* m_Source;
		AnimationState* m_Destination;
		Animator* m_Animator;
		std::function<bool()> m_Predicate;
	};

	class AnimationStateMachine
	{
	public:
		AnimationStateMachine() = default;
		AnimationStateMachine(Animator* animator) : m_Animator(animator) {}
		virtual ~AnimationStateMachine() = default;

		virtual Animation* DetermineOutputAnimation();

		void AddState(AnimationState* state, bool isDefault = false);

		void SetFloat(const std::string& name, float value);
		void SetInteger(const std::string& name, int value);
		void SetBool(const std::string& name, bool value);
		void SetString(const std::string& name, std::string& value);

		float GetFloat(const std::string& name, float defaultValue = 0.f);
		int GetInteger(const std::string& name, int defaultValue = 0);
		bool GetBool(const std::string& name, bool defaultValue = false);
		std::string GetString(const std::string& name, std::string defaultValue = "none");

		inline std::vector<AnimationState*> GetStates() const { return m_States; }
		inline AnimationState* GetCurrentState() const { return m_CurrentState; }
	private:
		Animator* m_Animator;
		AnimationState* m_CurrentState;
		std::vector<AnimationState*> m_States;
		std::unordered_map<std::string, float> m_FloatParameters;
		std::unordered_map<std::string, int> m_IntParameters;
		std::unordered_map<std::string, bool> m_BoolParameters;
		std::unordered_map<std::string, std::string> m_StringParameters;
	};
}
