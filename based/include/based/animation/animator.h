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

	class AnimationTransition
	{
	public:
		AnimationTransition() = default;
		AnimationTransition(const std::shared_ptr<AnimationState>& source, const std::shared_ptr<AnimationState>& destination, 
			const std::shared_ptr<Animator>& animator, std::function<bool()> predicate = nullptr)
			: m_Source(source), m_Destination(destination), m_Animator(animator)
			, m_Predicate(std::move(predicate)) {}
		virtual ~AnimationTransition() = default;

		virtual bool ShouldStateTransition() {
			if (m_Predicate != nullptr) return m_Predicate();

			if (auto anim = m_Animator.lock())
			{
				if (auto source = m_Source.lock())
					return anim->GetPlaybackTime() >= source->GetStateAnimationClip()->GetDuration();
				BASED_WARN("Source state is invalid!");
			}
			else { BASED_WARN("Animator is invalid!"); }

			return false;
		}

		inline std::weak_ptr<AnimationState> GetSourceState() const { return m_Source; }
		inline std::weak_ptr<AnimationState> GetDestinationState() const { return m_Destination; }
	private:
		std::weak_ptr<AnimationState> m_Source;
		std::weak_ptr<AnimationState> m_Destination;
		std::weak_ptr<Animator> m_Animator;
		std::function<bool()> m_Predicate;
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

		inline std::vector<std::shared_ptr<AnimationState>> GetStates() const { return m_States; }
		inline std::shared_ptr<AnimationState> GetCurrentState() const { return m_CurrentState.lock(); }
	private:
		std::weak_ptr<Animator> m_Animator;
		std::weak_ptr<AnimationState> m_CurrentState;
		std::vector<std::shared_ptr<AnimationState>> m_States;
		std::unordered_map<std::string, float> m_FloatParameters;
		std::unordered_map<std::string, int> m_IntParameters;
		std::unordered_map<std::string, bool> m_BoolParameters;
		std::unordered_map<std::string, std::string> m_StringParameters;
	};
}
