#pragma once

#include "animation.h"

namespace based::animation
{
	class AnimationStateMachine;
	class AnimationTransition;

	enum class TimeMode : uint8_t
	{
		Scaled,
		Unscaled
	};

	class Animator
	{
	public:
		Animator(const std::shared_ptr<Animation>& animation);
		~Animator()
		{
			BASED_TRACE("Destroying animator");
		}

		void UpdateAnimation(float dt);
		void PlayAnimation(const std::shared_ptr<Animation>& pAnimation);
		void CalculateBoneTransform(const AssimpNodeData* node, const glm::mat4& parentTransform, 
			std::weak_ptr<Animation> animation);
		std::vector<glm::mat4> GetFinalBoneMatrices();
		float blendSpeed = 5.f;

		std::shared_ptr<Animation> GetCurrentAnimation() const { return m_CurrentAnimation.lock(); }

		TimeMode GetTimeMode() const { return m_TimeMode; }
		void SetTimeMode(TimeMode mode) { m_TimeMode = mode; }

		inline float GetPlaybackTime() const { return m_CurrentTime; }
		inline void SetStateMachine(const std::shared_ptr<AnimationStateMachine>& machine) { m_StateMachine = machine; }
		inline std::shared_ptr<AnimationStateMachine> GetStateMachine() const { return m_StateMachine; }
	private:
		std::vector<glm::mat4> m_FinalBoneMatrices;
		std::weak_ptr<Animation> m_CurrentAnimation;
		std::weak_ptr<Animation> m_OldAnimation;
		std::shared_ptr<AnimationStateMachine> m_StateMachine;
		float m_PreviousTime;
		float m_CurrentTime;
		float m_DeltaTime;
		float m_BlendDelta;
		bool m_FirstFrame = true;

		TimeMode m_TimeMode;
	};

	class AnimationState
	{
	public:
		AnimationState() = default;
		AnimationState(const std::shared_ptr<Animation>& anim, const std::string& name = "New State")
			: m_Animation(anim), m_Speed(1.f), m_Name(name) {}
		virtual ~AnimationState() = default;

		virtual void OnStateEnter(const std::shared_ptr<Animator>& animator) {}
		virtual void OnStateExit(const std::shared_ptr<Animator>& animator) {}
		virtual void OnStateUpdate(const std::shared_ptr<Animator>& animator) {}

		std::vector<std::shared_ptr<AnimationTransition>> GetTransitions() const { return m_Transitions; }
		void AddTransition(const std::shared_ptr<AnimationTransition>& transition);

		inline void SetPlaybackSpeed(float speed) { m_Speed = speed; }
		inline float GetPlaybackSpeed() const { return m_Speed; }
		inline std::shared_ptr<Animation> GetStateAnimationClip() const { return m_Animation.lock(); }

		inline const std::string& GetStateName() const { return m_Name; }
	private:
		std::weak_ptr<Animation> m_Animation;
		std::vector<std::shared_ptr<AnimationTransition>> m_Transitions;
		float m_Speed;
		std::string m_Name;
	};

	template <typename T>
	struct TransitionRule
	{
		std::string name;
		T val;
		T defaultVal;
	};

	enum class TransitionType : uint8_t
	{
		All,
		Any
	};

	class AnimationStateMachine
	{
	public:
		AnimationStateMachine() = default;
		AnimationStateMachine(const std::shared_ptr<Animator>& animator) : m_Animator(animator) {}
		virtual ~AnimationStateMachine() = default;

		virtual std::weak_ptr<Animation> DetermineOutputAnimation();

		void AddState(const std::shared_ptr<AnimationState>& state, bool isDefault = false);

		void SetFloat(const std::string& name, float value);
		void SetInteger(const std::string& name, int value);
		void SetBool(const std::string& name, bool value);
		void SetString(const std::string& name, std::string& value);

		float GetFloat(const std::string& name, float defaultValue = 0.f);
		int GetInteger(const std::string& name, int defaultValue = 0);
		bool GetBool(const std::string& name, bool defaultValue = false);
		std::string GetString(const std::string& name, std::string defaultValue = "none");

		void SetHasBeenReset(bool reset) { m_HasBeenReset = reset; }

		inline std::vector<std::shared_ptr<AnimationState>> GetStates() const { return m_States; }
		inline std::shared_ptr<AnimationState> GetCurrentState() const { return m_CurrentState.lock(); }
		inline std::shared_ptr<AnimationTransition> GetLastTransition() const { return m_LastTransition; }
		inline bool HasBeenReset() const { return m_HasBeenReset; }
	private:
		std::weak_ptr<Animator> m_Animator;
		std::weak_ptr<AnimationState> m_CurrentState;
		std::shared_ptr<AnimationTransition> m_LastTransition;
		std::vector<std::shared_ptr<AnimationState>> m_States;
		std::unordered_map<std::string, float> m_FloatParameters;
		std::unordered_map<std::string, int> m_IntParameters;
		std::unordered_map<std::string, bool> m_BoolParameters;
		std::unordered_map<std::string, std::string> m_StringParameters;
		bool m_HasBeenReset = true;
	};

	struct TransitionRules
	{
		std::vector<TransitionRule<float>> floatRules;
		std::vector<TransitionRule<int>> intRules;
		std::vector<TransitionRule<bool>> boolRules;
		std::vector<TransitionRule<std::string>> stringRules;
		TransitionType transitionType = TransitionType::All;

		TransitionRules() = default;

		inline bool CheckRules(const std::shared_ptr<AnimationStateMachine>& stateMachine) const
		{
			bool result = false;

			for (auto& rule : floatRules)
			{
				if (stateMachine->GetFloat(rule.name, rule.defaultVal) == rule.val)
				{
					if (transitionType == TransitionType::Any) return true;
					else result = true;
				}
				else result = false;
			}

			for (auto& rule : intRules)
			{
				if (stateMachine->GetInteger(rule.name, rule.defaultVal) == rule.val)
				{
					if (transitionType == TransitionType::Any) return true;
					else result = true;
				}
				else result = false;
			}

			for (auto& rule : boolRules)
			{
				if (stateMachine->GetBool(rule.name, rule.defaultVal) == rule.val)
				{
					if (transitionType == TransitionType::Any) return true;
					else result = true;
				}
				else result = false;
			}

			for (auto& rule : stringRules)
			{
				if (stateMachine->GetString(rule.name, rule.defaultVal) == rule.val)
				{
					if (transitionType == TransitionType::Any) return true;
					else result = true;
				}
				else result = false;
			}

			return result;
		}

		inline void Reset(const std::shared_ptr<AnimationStateMachine>& stateMachine)
		{
			for (auto& rule : floatRules)
			{
				stateMachine->SetFloat(rule.name, rule.defaultVal);
			}

			for (auto& rule : intRules)
			{
				stateMachine->SetInteger(rule.name, rule.defaultVal);
			}

			for (auto& rule : boolRules)
			{
				stateMachine->SetBool(rule.name, rule.defaultVal);
			}

			for (auto& rule : stringRules)
			{
				stateMachine->SetString(rule.name, rule.defaultVal);
			}
		}
	};

	class AnimationTransition
	{
	public:
		AnimationTransition() = default;
		AnimationTransition(const std::shared_ptr<AnimationState>& source, const std::shared_ptr<AnimationState>& destination,
			const std::shared_ptr<Animator>& animator, std::function<bool()> predicate = nullptr)
			: m_Source(source), m_Destination(destination), m_Animator(animator)
			, m_Predicate(std::move(predicate)) {}
		AnimationTransition(
			const std::shared_ptr<AnimationState>& source,
			const std::shared_ptr<AnimationState>& destination,
			const std::shared_ptr<Animator>& animator,
			const std::shared_ptr<AnimationStateMachine>& stateMachine,
			const TransitionRules& rules = TransitionRules(),
			bool autoReset = false
			)
			: m_Source(source), m_Destination(destination), m_Animator(animator), m_StateMachine(stateMachine),
				m_TransitionRules(rules), m_AutoReset(autoReset) {}
			virtual ~AnimationTransition() = default;

		virtual bool ShouldStateTransition() {
			if (m_Predicate != nullptr) return m_Predicate();

			auto stateMachine = m_StateMachine.lock();
			auto anim = m_Animator.lock();
			auto source = m_Source.lock();

			if (stateMachine && anim && source)
			{
				if (m_TransitionRules.CheckRules(stateMachine)) return true;

				bool animDone = anim->GetPlaybackTime() >= source->GetStateAnimationClip()->GetDuration();
				if (animDone && (stateMachine->GetLastTransition() && stateMachine->GetLastTransition()->m_AutoReset) 
					&& !stateMachine->HasBeenReset())
				{
					if (stateMachine->GetLastTransition())
					{
						stateMachine->GetLastTransition()->m_TransitionRules.Reset(stateMachine);
						stateMachine->SetHasBeenReset(true);
					}
				}
				return animDone;
			} else if (anim && source)
			{
				bool animDone = anim->GetPlaybackTime() >= source->GetStateAnimationClip()->GetDuration();
				return animDone;
			}

			return false;
		}

		inline std::weak_ptr<AnimationState> GetSourceState() const { return m_Source; }
		inline std::weak_ptr<AnimationState> GetDestinationState() const { return m_Destination; }

		inline std::function<bool()> GetPredicate() const { return m_Predicate; }
	private:
		std::weak_ptr<AnimationState> m_Source;
		std::weak_ptr<AnimationState> m_Destination;
		std::weak_ptr<Animator> m_Animator;
		std::weak_ptr<AnimationStateMachine> m_StateMachine;
		std::function<bool()> m_Predicate;

		TransitionRules m_TransitionRules;
		bool m_AutoReset;
	};
}
