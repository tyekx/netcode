#include "Blackboard.h"


namespace Egg::Animation {

	AnimationState * Blackboard::FindReferenceByName(const std::string & name) {

		for(unsigned int i = 0; i < statesLength; ++i) {
			if(strcmp((states + i)->name, name.c_str()) == 0) {
				return states + i;
			}
		}

		return nullptr;
	}

	void Blackboard::CheckCurrentTransitions(MovementController * movCtrl) {

		for(unsigned int i = 0; i < currentState->transitionsLength; ++i) {
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

	void Blackboard::CreateResources(Asset::Model * model,
						const std::initializer_list<AnimationState> & sts,
						const std::initializer_list<TransitionInit> &transitions) {
		
		unsigned int requiredSize = static_cast<unsigned int>(sts.size() * sizeof(AnimationState) + transitions.size() * sizeof(AnimationState::Transition) + sizeof(AnimationBlender));
		
		classifier.Initialize(requiredSize);

		blender = classifier.Allocate<AnimationBlender>();
		new (blender) AnimationBlender{ model->bones, model->bonesLength };

		// step1: allocate and initialize states
		states = static_cast<AnimationState *>(classifier.Allocate(static_cast<unsigned int>(sizeof(AnimationState) * sts.size())));
		statesLength = static_cast<unsigned int>(sts.size());

		unsigned int idx = 0;
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
		for(unsigned int i = 0; i < statesLength; ++i) {
			AnimationState * state = states + i;
			state->transitions = reinterpret_cast<AnimationState::Transition *>(classifier.Allocate(sizeof(AnimationState::Transition) * state->transitionsLength));
			for(unsigned int j = 0; j < state->transitionsLength; ++j) {
				new (state->transitions + j) AnimationState::Transition{};
			}
		}

		// step4: connect object references as pointers instead of strings, for faster access
		for(unsigned int i = 0; i < statesLength; ++i) {
			AnimationState * state = states + i;
			unsigned int j = 0;
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

	void Blackboard::CopyBoneDataInto(void * dest)
	{
		blender->CopyBoneDataInto(dest);
	}

}
