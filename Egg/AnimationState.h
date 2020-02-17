#pragma once

#include <string>
#include "MovementController.h"
#include <EggAssetLib/Animation.h>

namespace Egg::Animation {

	enum class TransitionBehaviour : unsigned {
		NONE = 0, STOP_AND_LERP = 1, LERP = 2
	};

	enum class StateBehaviour {
		NONE, LOOP, ONCE
	};

	class AnimationState {
	public:

		using mov_query_func_t = bool (MovementController:: *)() const;
		using anim_query_func_t = bool(AnimationState:: *)() const;


		class Transition {
		public:
			mov_query_func_t movQueryFunction;
			anim_query_func_t animQueryFunction;

			AnimationState * targetState;

			TransitionBehaviour behaviour;

			Transition();

			Transition(mov_query_func_t movQueryFunc, anim_query_func_t animQueryFunc, AnimationState * targetState, TransitionBehaviour howToTransition);

			bool operator()(MovementController * movCtrl, AnimationState * currentState) const;
		};

		char name[32];
		StateBehaviour behaviour;
		unsigned int id;
		Asset::Animation * animationRef;
		Transition * transitions;
		unsigned int transitionsLength;

		float weight;
		float weightVelocity;
		float animTime;
		float animSpeed;
		float animTicksPerSecond;
		float animDuration;

		void SetAnimationRef(Asset::Animation * aRef);

		AnimationState(const AnimationState & a);

		AnimationState(const std::string & name, unsigned int animId, StateBehaviour behaviour);

		void Update(float dt);

		bool IsFinished() const;

		bool IsPlaying() const;
	};
}
