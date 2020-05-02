#pragma once

#include "State.h"

namespace Netcode::Animation {

	template<typename T>
	class StateGroup {
		std::vector<State<T>> states;
		Transition<T> * activeTransition;
		State<T> * activeState;
		State<T> * targetState;

		void EnableTransition(Transition<T> * transition) {
			activeTransition = transition;
			activeTransition->Clear();
			targetState = states.data() + activeTransition->GetTargetState();
			targetState->Clear();
		}

		void FinalizeTransition() {
			activeState = targetState;
			activeTransition = nullptr;
			targetState = nullptr;
		}

		void TryTransition(T * controller) {
			if(activeTransition == nullptr) {
				Transition<T> * transitionPtr = activeState->TryTransition(controller);
				if(transitionPtr != nullptr) {
					EnableTransition(transitionPtr);
				}
			}
		}

		int32_t GetStateIdByName(const std::string & name) {
			for(int32_t i = 0; i < states.size(); ++i) {
				if(states[i].name == name) {
					return i;
				}
			}
			return -1;
		}

	public:

		StateGroup(std::initializer_list<State<T>> initStates, std::initializer_list<TransitionInit<T>> transitions) :
			states{}, activeTransition{ nullptr }, activeState{ nullptr }, targetState{ nullptr } {
			states = initStates;

			for(State<T> & state : states) {
				std::vector<Transition<T>> stateTransitions;
				for(const TransitionInit<T> & tr : transitions) {
					if(state.name == tr.ownerState) {
						// Transition(Function::Scalar weightFunction, Function::Scalar timeScaleFunction, float duration,
						//			  ControllerFunction ctrlFunc, StateFunction stateFunc, State<T> * targetState)
						stateTransitions.emplace_back(tr.weightFunction, tr.timeScaleFunction, tr.duration,
							tr.ctrlFunc, tr.stateFunc, GetStateIdByName(tr.targetState));
					}
				}
				state.SetTransitions(std::move(stateTransitions));
			}
		}

		void SetDefaults() {
			activeState = states.data();
			activeTransition = nullptr;
			targetState = nullptr;
		}

		void GetActiveStates(std::vector<StateBase> & dst) const {
			if(activeState != nullptr) {
				dst.push_back(*activeState);
			}

			if(targetState != nullptr) {
				dst.push_back(*targetState);
			}
		}

		void Update(T * controller, float dt) {
			float currentStateDt = dt;
			float activeStateWeight = 1.0f;

			TryTransition(controller);

			if(activeTransition != nullptr) {
				activeTransition->Update(dt);
				if(activeTransition->IsComplete()) {
					FinalizeTransition();
				} else {
					currentStateDt = activeTransition->GetStateTimeIncrement(dt);
					activeStateWeight = 1.0f - activeTransition->GetStateWeight();
				}
			}

			if(activeState != nullptr) {
				activeState->weight = activeStateWeight;
				activeState->Update(currentStateDt);
			}

			if(targetState != nullptr) {
				targetState->weight = 1.0f - activeStateWeight;
				targetState->Update(dt);
			}
		}
	};

}
