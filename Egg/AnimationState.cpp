#include "AnimationState.h"

namespace Egg::Animation {

	AnimationState::Transition::Transition() : movQueryFunction{ nullptr }, animQueryFunction{ nullptr }, targetState{ nullptr }, behaviour{ TransitionBehaviour::NONE } { }

	AnimationState::Transition::Transition(mov_query_func_t movQueryFunc, anim_query_func_t animQueryFunc, AnimationState * targetState, TransitionBehaviour howToTransition)
		: movQueryFunction{ movQueryFunc },
		animQueryFunction{ animQueryFunc },
		targetState{ targetState },
		behaviour{ howToTransition } { }

	bool AnimationState::Transition::operator()(MovementController * movCtrl, AnimationState * currentState) const {
		return  (movQueryFunction == nullptr || (movCtrl->*movQueryFunction)()) &&
			(animQueryFunction == nullptr || (currentState->*animQueryFunction)());
	}

	void AnimationState::SetAnimationRef(Asset::Animation * aRef) {
		animationRef = aRef;
		animDuration = (float)aRef->duration;
		animTicksPerSecond = (float)aRef->ticksPerSecond;
	}

	AnimationState::AnimationState(const AnimationState & a) {
		strcpy_s(name, a.name);
		id = a.id;
		behaviour = a.behaviour;
		animationRef = a.animationRef;
		transitions = a.transitions;
		transitionsLength = a.transitionsLength;
		weight = a.weight;
		animTime = a.animTime;
		animSpeed = a.animSpeed;
		animTicksPerSecond = a.animTicksPerSecond;
		animDuration = a.animDuration;
	}

	AnimationState::AnimationState(const std::string & nm, unsigned int animId, StateBehaviour behaviour) :
		name{  },
		behaviour{ behaviour },
		id{ animId },
		animationRef{ nullptr },
		transitions{ nullptr },
		transitionsLength{ 0 },
		weight{ 0.0f },
		animTime{ 0.0f },
		animSpeed{ 1.0f },
		animTicksPerSecond{ 0.0f },
		animDuration{ 0.0f } {
			strcpy_s(name, nm.c_str());
	}

	void AnimationState::Update(float dt) {
		animTime += animSpeed * (animTicksPerSecond * dt);

		switch(behaviour) {
		case StateBehaviour::LOOP:
			animTime = fmodf(animTime, animDuration);
			break;
		case StateBehaviour::ONCE:
			animTime = fminf(animTime, animDuration);
			break;
		default:
			break;
		}
	}

	bool AnimationState::IsFinished() const {
		return animTime == animDuration;
	}

	bool AnimationState::IsPlaying() const {
		return animSpeed > 0.0f;
	}

}
