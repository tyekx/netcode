#include "Blackboard.h"


namespace Egg::Animation {

	AnimationState * Blackboard::FindReferenceByName(const std::string & name) {

		for(UINT i = 0; i < statesLength; ++i) {
			if(strcmp((states + i)->name, name.c_str()) == 0) {
				return states + i;
			}
		}

		return nullptr;
	}

	void Blackboard::CheckCurrentTransitions(MovementController * movCtrl) {

		for(UINT i = 0; i < currentState->transitionsLength; ++i) {
			AnimationState::Transition * t = currentState->transitions + i;

			if((*t)(movCtrl, currentState)) {
				currentState->weightVelocity = -5.0f;
				switch(t->behaviour) {
				case TransitionBehaviour::STOP_AND_LERP:
					currentState->animSpeed = 0.0f;
					break;
				}
				currentState = t->targetState;
				currentState->animTime = 0.0f;
				currentState->animSpeed = 1.0f;
				currentState->weightVelocity = 5.0f;
				blender->ActivateState(currentState);
				break;
			}
		}
	}

	Blackboard::Blackboard() : classifier{}, prevState{ nullptr }, currentState{ nullptr }, states{ nullptr }, statesLength{ 0 }, blender{ nullptr } { }

	void Blackboard::CreateResources(Asset::Model * model, void * writeDest, unsigned int animationsLength,
						const std::initializer_list<AnimationState> & sts,
						const std::initializer_list<TransitionInit> &transitions) {

		UINT requiredSize = static_cast<UINT>(sts.size() * sizeof(AnimationState) + transitions.size() * sizeof(AnimationState::Transition) + sizeof(AnimationBlender));
		classifier.Initialize(requiredSize);

		blender = classifier.Allocate<AnimationBlender>();
		new (blender) AnimationBlender{ model->bones, model->bonesLength, writeDest };

		// step1: allocate and initialize states
		states = static_cast<AnimationState *>(classifier.Allocate(static_cast<UINT>(sizeof(AnimationState) * sts.size())));
		statesLength = static_cast<UINT>(sts.size());

		UINT idx = 0;
		for(auto & i : sts) {
			new (states + idx) AnimationState{ i };
			(states + idx)->SetAnimationRef(model->animations + i.id);
			++idx;
		}

		// step2: calculate the number of transitions for every state
		for(auto & t : transitions) {
			// omit nullptr check for crude error reporting
			FindReferenceByName(t.ownerState)->transitionsLength++;
		}

		// step3: allocate memory for transitions and initialize it
		for(UINT i = 0; i < statesLength; ++i) {
			AnimationState * state = states + i;
			state->transitions = static_cast<AnimationState::Transition *>(classifier.Allocate(sizeof(AnimationState::Transition) * state->transitionsLength));
			for(UINT j = 0; j < state->transitionsLength; ++j) {
				new (state->transitions + j) AnimationState::Transition{};
			}
		}

		// step4: fill transitions with meaningful data, bit wasteful but gets the job done
		for(UINT i = 0; i < statesLength; ++i) {
			AnimationState * state = states + i;
			UINT j = 0;
			for(const auto & t : transitions) {
				AnimationState* tOwner = FindReferenceByName(t.ownerState);

				if(tOwner == state) {
					AnimationState * targetState = FindReferenceByName(t.targetState);
					AnimationState::Transition * transition = state->transitions + j;
					new (state->transitions + j) AnimationState::Transition{ t.movQueryFunction, t.animQueryFunction, targetState, t.behaviour };
					++j;
				}
			}
		}

		// step5: set current transition
		currentState = states;
		currentState->animTime = 0.0f;
		currentState->animSpeed = 1.0f;
		currentState->weightVelocity = 5.0f;
		blender->ActivateState(currentState);
	}

	void Blackboard::Update(float dt, MovementController * movCtrl) {
		currentState->Update(dt);
		CheckCurrentTransitions(movCtrl);
		blender->ActivateState(currentState);

		blender->UpdateStates(dt);
		blender->Blend();
	}

}
