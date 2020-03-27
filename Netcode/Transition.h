#pragma once


#include "AnimationState.h"

namespace Netcode::Animation {

	/*
	Class that stores enough information about a transition to properly initialize
	The reason for this type is that referencing to states by strings is wasteful, however needed for the user
	*/
	class TransitionInit {
	public:

		std::string ownerState;
		std::string targetState;
		AnimationState::mov_query_func_t movQueryFunction;
		AnimationState::anim_query_func_t animQueryFunction;
		TransitionBehaviour behaviour;

		TransitionInit(const TransitionInit & ti) :
			ownerState{ ti.ownerState },
			targetState{ ti.targetState },
			movQueryFunction{ ti.movQueryFunction },
			animQueryFunction{ ti.animQueryFunction },
			behaviour{ ti.behaviour } { }

		TransitionInit(const std::string & ownerState, const std::string & targetState, AnimationState::mov_query_func_t movQueryFunc, AnimationState::anim_query_func_t animQueryFunc, TransitionBehaviour howToTransition);
	};

}
