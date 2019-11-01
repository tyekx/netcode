#pragma once


#include "AnimationState.h"

namespace Egg::Animation {

	enum class TransitionBehaviour {
		STOP_AND_LERP, LERP
	};

	using mov_query_func_t = bool (MovementController:: *)() const;
	using anim_query_func_t = bool (AnimationState:: *)() const;

	class Transition {
	public:
		mov_query_func_t movQueryFunction;
		anim_query_func_t animQueryFunction;

		std::string ownerState;
		std::string targetState;

		AnimationState * pOwnerState;
		AnimationState * pTargetState;

		Transition(const std::string & ownerState, const std::string & targetState, mov_query_func_t movQueryFunc, anim_query_func_t animQueryFunc, TransitionBehaviour howToTransition)
			: movQueryFunction{ movQueryFunc },
			animQueryFunction{ animQueryFunc },
			ownerState{ ownerState }, 
			targetState{ targetState }, 
			pOwnerState{ nullptr }, 
			pTargetState{ nullptr } { }

		bool operator()(MovementController * movCtrl, AnimationState * currentState) const {
			return  (movQueryFunction == nullptr || (movCtrl->*movQueryFunction)()) &&
				(animQueryFunction == nullptr || (currentState->*animQueryFunction)());
		}

		bool Owns(AnimationState * anim) {
			return anim == pOwnerState;
		}
	};

}
