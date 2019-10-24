#include <initializer_list>
/*

enum class StateBehaviour {
	NONE, LOOP, ONCE
};

class MovementController {
public:
	bool IsMovingForward() const {
		return false;
	}

	bool IsMovingLeft() const {
		return false;
	}

	bool IsMovingRight() const {
		return false;
	}

	bool IsMovingBackward() const {
		return IsOnGround(); // and only AxisY is active
	}

	bool IsJumping() const {

	}

	bool IsFiring() const {

	}

	bool IsOnGround() const {

	}

	bool IsIdle() const {

	}
};

enum class TransitionBehaviour {
	STOP_AND_LERP, LERP
};

class AnimationState {
public:
	AnimationState(const char * name, StateBehaviour behaviour) {

	}

	bool IsFinished() const {

	}

	bool IsPlaying() const {

	}
};

using mov_query_func_t = bool (MovementController:: *)() const;
using anim_query_func_t = bool (AnimationState:: *)() const;

class Transition {
	mov_query_func_t movQueryFunction;
	anim_query_func_t animQueryFunction;
public:
	Transition(const char * ownerState, const char * targetState, mov_query_func_t movQueryFunc, anim_query_func_t animQueryFunc, TransitionBehaviour howToTransition) {
		movQueryFunction = movQueryFunc;
		animQueryFunction = animQueryFunc;
	}

	bool operator()(MovementController* movCtrl, AnimationState * currentState) const {
		return  (movQueryFunction  == nullptr || (movCtrl->*movQueryFunction)()) &&
				(animQueryFunction == nullptr || (currentState->*animQueryFunction)());
	}
};

class Blackboard {
public:
	Blackboard(std::initializer_list<AnimationState> states, std::initializer_list<Transition> transitions) {
		
	}
};

void funct() {

	Blackboard bc({
					AnimationState{ "Idle",			StateBehaviour::LOOP },
					AnimationState{ "Forward",		StateBehaviour::LOOP },
					AnimationState{ "JumpStart",	StateBehaviour::ONCE },
					AnimationState{ "JumpLand",		StateBehaviour::ONCE }
			   },
			   {
					Transition{ "Idle",		 "Forward",		MovementController::IsMovingForward, nullptr,					 TransitionBehaviour::LERP },
					Transition{ "Forward",   "Idle",		MovementController::IsIdle,			 nullptr,					 TransitionBehaviour::LERP },
					Transition{ "Idle",		 "JumpStart",	MovementController::IsJumping,		 nullptr,					 TransitionBehaviour::STOP_AND_LERP },
					Transition{ "JumpStart", "JumpLoop",	MovementController::IsJumping,		 AnimationState::IsFinished, TransitionBehaviour::STOP_AND_LERP },
					Transition{ "JumpStart", "JumpLand",	MovementController::IsOnGround, 	 nullptr,					 TransitionBehaviour::STOP_AND_LERP },
					Transition{ "JumpLoop",  "JumpLand",	MovementController::IsOnGround,      nullptr,					 TransitionBehaviour::STOP_AND_LERP },
					Transition{ "JumpLand",  "Idle",        nullptr,							 AnimationState::IsFinished, TransitionBehaviour::LERP },
			   });
	

}


*/