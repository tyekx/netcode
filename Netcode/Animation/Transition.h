#pragma once

#include <string>
#include <functional>

#include "../Functions.h"

namespace Netcode::Animation {
	template<typename T>
	class State;



	class TransitionBase {
	protected:
		Function::Scalar weightFunction;
		Function::Scalar timeScaleFunction;

		float time;
		float duration;
	public:

		TransitionBase(Function::Scalar weightFunction, Function::Scalar timeScaleFunction, float duration);

		float GetNormalizedTime() const;

		float GetStateTimeIncrement(float deltaTime) const;

		float GetStateWeight() const;

		bool IsComplete() const;

		void Update(float dt);

		void Clear();
	};

	template<typename T>
	class Transition : public TransitionBase {
	public:
		using ControllerFunction = bool(T:: *)() const;
		using StateFunction = bool(State<T>:: *) () const;

	private:
		ControllerFunction ctrlFunc;
		StateFunction stateFunc;
		int32_t targetState;

	public:
		Transition(Function::Scalar weightFunction, Function::Scalar timeScaleFunction, float duration,
			ControllerFunction ctrlFunc, StateFunction stateFunc, int32_t targetState) :
			TransitionBase{ weightFunction, timeScaleFunction, duration },
			ctrlFunc{ ctrlFunc },
			stateFunc{ stateFunc },
			targetState{ targetState } {}

		int32_t GetTargetState() const {
			return targetState;
		}

		bool operator()(T * transitionController, State<T> * currentState) {
			return ((ctrlFunc == nullptr) || std::invoke(ctrlFunc, transitionController)) &&
				((stateFunc == nullptr) ||  std::invoke(stateFunc, currentState));
		}
	};

	template<typename T>
	struct TransitionInit {
		using ControllerFunction = bool(T:: *)() const;
		using StateFunction = bool(State<T>:: *) () const;

		std::string ownerState;
		std::string targetState;
		ControllerFunction ctrlFunc;
		StateFunction stateFunc;
		Function::Scalar weightFunction;
		Function::Scalar timeScaleFunction;
		float duration;

		TransitionInit(const std::string & ownerState, const std::string & targetState,
			ControllerFunction ctrlFunc,
			StateFunction stateFunc,
			Function::Scalar weightFunction,
			Function::Scalar timeScaleFunction,
			float duration) : ownerState{ ownerState }, targetState{ targetState },
			ctrlFunc{ ctrlFunc },
			stateFunc{ stateFunc },
			weightFunction{ weightFunction },
			timeScaleFunction{ timeScaleFunction },
			duration{ duration } {

		}
	};

}
