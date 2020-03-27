#include "Transition.h"

namespace Egg::Animation {


	TransitionInit::TransitionInit(const std::string & ownerState, const std::string & targetState, AnimationState::mov_query_func_t movQueryFunc, AnimationState::anim_query_func_t animQueryFunc, TransitionBehaviour howToTransition) :
		ownerState{ ownerState }, targetState{ targetState }, movQueryFunction{ movQueryFunc }, animQueryFunction{ animQueryFunc }, behaviour{ howToTransition } {

	}

}
